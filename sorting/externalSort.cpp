#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <algorithm>
#include <vector>
#include <queue>

#include "sorting/externalSort.h"
#include "utils/checkedIO.h"
#include "utils/sequenceReader.h"
#include "utils/sequenceWriter.h"

namespace dbImpl {

//holds informations about a run of sorted numbers
class RunDescriptor {
  public:
    uint64_t offset; //offset from start of file in bytes
    size_t size; //lenght of the run in number of elements
};
typedef std::vector<RunDescriptor> RunList;

//forms runs of sorted numbers
//the input numbers are read from the file descriptor fdInput.
//size gives the number of uint64_t integers to be read from fdInput.
//the runs are stored in fdOutput.
//memSize gives the amount of memory to be used.
//a list of runDescriptors will be returned.
RunList formRuns(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);
//merges runs of sorted numbers
//returns the descriptor for the resulting run
RunDescriptor mergeRuns(RunList runs, int fd, uint64_t size, uint64_t memSize, uint64_t blockSize);

//returns the quotient of the parameters.
//In contrast to normal integer arithmetics, the result will be rounded towards infinity
uint64_t divideRoundingUp(uint64_t dividend, uint64_t divisor);
//returns the smallest number greater than num which can be divided by
//alignment without remainder
uint64_t padTo(uint64_t num, uint64_t alignment);

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
  /*
   * implementation decisions:
   *  * the output file will be used as scratch space. It is twice as large as neccessary.
   *    In the merging phase, the data will be copied repeatedly between both halfs.
   *  * std::vector is avoided for holding the uint64_t elements since std::vector
   *    initializes its memory with 0 and this initialization would be useless overhead
   *  * all file accesses are done using the preferred block size as reported by fstat
   *  * posix_fadvise is used to give hints for tuning disk accesses
   */
  //return immediately if size is 0 since there is no work to be done
  //posix_fallocate could not handle a size of 0
  if(size <= 0) return;
  //calculate block sizes, nr. of blocks, ...
  struct stat inStat;
  if(fstat(fdOutput, &inStat) != 0) {
    std::cerr << "unable to stat output file" << fdOutput << ": " << strerror(errno) << std::endl;
    exit(1);
  }
  blksize_t blockSize = inStat.st_blksize;
  #ifdef DEBUG
  std::clog << "block size of temporary file: " << blockSize << " bytes" << std::endl;
  #endif
  if(blockSize % sizeof(uint64_t) != 0) {
    std::cerr << "memory size is not a multiple of the I/O block size" << std::endl;
    std::cerr << "aligned disk access is not possible" << std::endl;
    exit(1);
  }
  uint64_t blocksInMemory = memSize / blockSize;
  memSize = blocksInMemory * blockSize;
  #ifdef DEBUG
  std::clog << "number of blocks kept in memory: " << blocksInMemory << std::endl;
  std::clog << "used memory: " << memSize << " bytes" << std::endl;
  #endif
  if(blocksInMemory < 3) {
    std::cerr << "unable to merge runs: not enough memory" << std::endl;
    exit(1);
  }

  //preallocate disk space
  if(int ret = posix_fallocate(fdOutput, 0, 2 * padTo(size*sizeof(uint64_t), blockSize))) {
    std::cerr << "unable to allocate disk space: " << strerror(ret) << std::endl;
    exit(1);
  }

  //form sorted runs
  RunList runs = formRuns(fdInput, size, fdOutput, memSize);

  //merge runs
  RunDescriptor mergedRun = mergeRuns(runs, fdOutput, size, memSize, blockSize);
  #ifdef DEBUG
  std::clog << "merged run" << std::endl;
  std::clog << "offset: " << mergedRun.offset << ", size: " << mergedRun.size << std::endl;
  #endif

  //truncate the output file so it only contains the mergedRun
  #ifdef DEBUG
  std::clog << "carving sorted data out of temporary files..." << std::endl;
  #endif
  if(mergedRun.offset != 0) {
    //move the data to the beginning of the file.
    //fallocate with FALLOC_FL_COLLAPSE_RANGE would be a better solution
    //but it is only supported on ext4 and xfs file systems.
    size_t runBytes = mergedRun.size*sizeof(uint64_t);
    off_t offset = 0;
    std::unique_ptr<uint8_t[]> buffer(new (std::nothrow) uint8_t[memSize]);
    if(!buffer) {
      std::cerr << "unable to allocate buffer" << std::endl;
      exit(1);
    }
    while(static_cast<size_t>(offset) < runBytes) {
      size_t chunkSize = std::min(runBytes - offset, memSize);
      checkedPread(fdOutput, &buffer[0], chunkSize, mergedRun.offset + offset);
      checkedPwrite(fdOutput, &buffer[0], chunkSize, offset);
      offset += chunkSize;
    }
  }
  if(ftruncate(fdOutput, mergedRun.size*sizeof(uint64_t)) != 0) {
    std::cerr << "unable to truncate output file: " << strerror(errno) << std::endl;
    exit(1);
  }
}

RunList formRuns(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
  #ifdef DEBUG
  std::clog << "forming runs..." << std::endl;
  #endif
  //preallocate the list of runs
  RunList runs;
  runs.reserve(divideRoundingUp(size, memSize/sizeof(uint64_t)));
  posix_fadvise(fdInput, 0, size*sizeof(uint64_t), POSIX_FADV_SEQUENTIAL);
  std::unique_ptr<uint64_t[]> run(new (std::nothrow) uint64_t[memSize / sizeof(uint64_t)]);
  if(!run) {
    std::cerr << "unable to allocate buffer" << std::endl;
    exit(1);
  }
  uint64_t elementsProcessed = 0;
  while(elementsProcessed < size) {
    size_t runSize = std::min(size - elementsProcessed, memSize / sizeof(uint64_t));
    uint64_t offset = elementsProcessed * sizeof(uint64_t);
    size_t runBytes = runSize * sizeof(uint64_t);
    checkedPread(fdInput, &run[0], runBytes, offset);
    std::sort(&run[0], &run[runSize]);
    checkedPwrite(fdOutput, &run[0], runBytes, offset);
    posix_fadvise(fdInput, offset, runBytes, POSIX_FADV_DONTNEED);
    runs.push_back({offset, runSize});
    elementsProcessed += runSize;
  }
  return runs;
}

//an element of the priority queue used for merging the runs
struct MergeQueueElement {
  uint64_t value;
  uint64_t bufferNr;

  bool operator> (const MergeQueueElement& rhs) const {
    return this->value > rhs.value;
  }
};

RunDescriptor mergeRuns(RunList runs, int fd, uint64_t size, uint64_t memSize, uint64_t blockSize) {
  uint64_t blocksInMemory = memSize/blockSize;
  uint64_t bufferSize = blockSize/sizeof(uint64_t);
  //one block will be occupied by the output buffer. All other blocks are input buffers
  uint64_t runsMergedPerSweep = blocksInMemory-1;
  //the queue used for ordering
  std::priority_queue<MergeQueueElement, std::vector<MergeQueueElement>, std::greater<MergeQueueElement>> queue;
  RunList mergedRuns;
  mergedRuns.reserve(divideRoundingUp(runs.size(), runsMergedPerSweep));
  //repeatedly merge runs until only one run remains
  while(runs.size() > 1) {
    //the offset within the file at which the scratch space starts
    uint64_t outputStartOffset;;
    if(runs.front().offset == 0) {
      outputStartOffset = padTo(size * sizeof(uint64_t), blockSize);
    } else {
      outputStartOffset = 0;
    }
    uint64_t runOffset = 0;
    while(runOffset < runs.size()) {
      //prepare output buffer
      SequenceWriter<uint64_t> outBuffer(fd, outputStartOffset, bufferSize);
      //load buffers, fill queue
      std::vector<SequenceReader<uint64_t>> readers;
      readers.reserve(runsMergedPerSweep);
      for(uint64_t bufferNr = 0; bufferNr < runsMergedPerSweep && runOffset + bufferNr < runs.size(); ++bufferNr) {
        uint64_t runNr = runOffset + bufferNr;
        readers.emplace_back(fd, runs[runNr].offset, runs[runNr].size, bufferSize);
        queue.push({readers.back().consumeElement(), bufferNr});
      }
      #ifdef DEBUG
      std::clog << "merging runs " << runOffset << ".." << runOffset + readers.size() << std::endl;
      #endif
      //sort until all input runs are exhausted
      while(!queue.empty()) {
        const auto elem = queue.top();
        queue.pop();
        const auto value = elem.value;
        const auto bufferNr = elem.bufferNr;
        //add element to output buffer
        outBuffer.append(value);
        //load new elements from run
        if(!readers[bufferNr].empty()) {
          uint64_t newValue = readers[bufferNr].consumeElement();
          queue.push({newValue, bufferNr});
        }
      }
      //write the remaining output buffer
      outBuffer.flush();
      //update offsets and add descriptors
      mergedRuns.push_back({outputStartOffset, outBuffer.getElementCount()});
      outputStartOffset += padTo(outBuffer.getElementCount() * sizeof(uint64_t), blockSize);
      runOffset += readers.size();
    }
    //swap runs and mergedRuns
    runs.swap(mergedRuns);
    mergedRuns.clear();
  }
  //return the one remaining run
  return runs[0];
}

uint64_t divideRoundingUp(uint64_t dividend, uint64_t divisor) {
  //by using 1+ ... -1 the division rounds towards infinity
  return 1 + (dividend-1)/divisor;
}

uint64_t padTo(uint64_t num, uint64_t alignment) {
  return divideRoundingUp(num, alignment) * alignment;
}

} //namespace dbImpl
