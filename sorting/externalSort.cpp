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

namespace dbImpl {

//holds informations about a run of sorted numbers
struct runDescriptor {
  off_t offset; //offset from start of file in bytes
  size_t size; //lenght of the run in number of elements
};
typedef std::vector<runDescriptor> runList;

//forms runs of sorted numbers
//the input numbers are read from the file descriptor fdInput.
//size gives the number of uint64_t integers to be read from fdInput.
//the runs are stored in fdOutput.
//memSize gives the amount of memory to be used.
//a list of runDescriptors will be returned.
runList formRuns(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);
//merges runs of sorted numbers
//returns the descriptor for the resulting run
runDescriptor mergeRuns(const runList& runs, int fd, uint64_t runsMergedPerSweep);

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
  /*
   * implementation decisions:
   *  * std::vector is avoided for holding the uint64_t elements since std::vector
   *    initializes its memory with 0 and this initialization would be useless overhead
   *  * all file accesses are done using the preferred block size as reported by fstat
   *  * posix_fadvise is used to give hints for tuning disk accesses
   */
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

  //form sorted runs
  runList runs = formRuns(fdInput, size, fdOutput, memSize);

  //merge runs
  uint64_t runsMergedPerSweep = blocksInMemory-1;
  runDescriptor mergedRun = mergeRuns(runs, fdOutput, runsMergedPerSweep);

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
  if(ftruncate(fdOutput, mergedRun.size) != 0) {
    std::cerr << "unable to truncate output file: " << strerror(errno) << std::endl;
    exit(1);
  }
}

runList formRuns(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
  runList runs;
  #ifdef DEBUG
  std::clog << "forming runs..." << std::endl;
  #endif
  if(int ret = posix_fallocate(fdOutput, 0, size * sizeof(uint64_t))) {
    std::cerr << "unable to allocate disk space: " << strerror(ret);
    exit(1);
  }
  posix_fadvise(fdInput, 0, size*sizeof(uint64_t), POSIX_FADV_SEQUENTIAL);
  std::unique_ptr<uint64_t[]> run(new (std::nothrow) uint64_t[memSize / sizeof(uint64_t)]);
  if(!run) {
    std::cerr << "unable to allocate buffer" << std::endl;
    exit(1);
  }
  uint64_t elementsProcessed = 0;
  while(elementsProcessed < size) {
    size_t runSize = std::min(size - elementsProcessed, memSize / sizeof(uint64_t));
    off_t offset = elementsProcessed * sizeof(uint64_t);
    size_t runBytes = runSize * sizeof(uint64_t);
    checkedPread(fdInput, &run[0], runBytes, offset);
    std::sort(&run[0], &run[runSize]);
    checkedPwrite(fdOutput, &run[0], runBytes, offset);
    posix_fadvise(fdInput, offset, runBytes, POSIX_FADV_DONTNEED);
    runs.push_back({.offset = offset, .size = runSize});
    elementsProcessed += runSize;
  }
  return runs;
}

runDescriptor mergeRuns(const runList& runs, int fd, uint64_t runsMergedPerSweep) {
  #ifdef DEBUG
  std::clog << "merging runs..." << std::endl;
  #endif
  //necessary for avoiding "unused parameters" compiler warning...
  int unused = fd + runsMergedPerSweep;
  runs[unused];
  //for now, simply return the last run
  return runs.back();
  //TODO: implement the merge phase
}

} //namespace dbImpl
