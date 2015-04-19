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

#include "externalSort.h"

namespace dbImpl {

//wraps pread with error checking code.
//if an error occurs exit() will be called.
void checkedPread(int fd, void* buf, size_t count, off_t offset);
//wraps pread with error checking code.
//if an error occurs exit() will be called.
void checkedPwrite(int fd, void* buf, size_t count, off_t offset);

//holds informations about a run of sorted numbers
struct runDescriptor {
  off_t offset; //offset from start of file in bytes
  size_t size; //lenght of the run in number of elements
};
typedef std::vector<runDescriptor> runList;

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
    //aligned block I/O will not be possible
    std::cerr << "memory size is not a multiple of the I/O block size" << std::endl;
    std::cerr << "aligned disk access is not possible" << std::endl;
    exit(1);
  }
  uint64_t blocksInMemory = memSize / blockSize;
  uint64_t usedMemory = blocksInMemory * blockSize;
  #ifdef DEBUG
  std::clog << "number of blocks kept in memory: " << blocksInMemory << std::endl;
  std::clog << "used memory: " << usedMemory << " bytes" << std::endl;
  #endif
  if(blocksInMemory < 3) {
    std::cerr << "unable to merge runs: not enough memory" << std::endl;
    exit(1);
  }
  //form sorted runs
  runList runs;
  {
    #ifdef DEBUG
    std::clog << "forming runs..." << std::endl;
    #endif
    if(int ret = posix_fallocate(fdOutput, 0, size * sizeof(uint64_t))) {
      std::cerr << "unable to allocate disk space: " << strerror(ret);
      exit(1);
    }
    posix_fadvise(fdInput, 0, size*sizeof(uint64_t), POSIX_FADV_SEQUENTIAL);
    std::unique_ptr<uint64_t[]> run(new (std::nothrow) uint64_t[usedMemory / sizeof(uint64_t)]);
    if(!run) {
      std::cerr << "unable to allocate buffer" << std::endl;
      exit(1);
    }
    uint64_t elementsProcessed = 0;
    while(elementsProcessed < size) {
      size_t runSize = std::min(size - elementsProcessed, usedMemory / sizeof(uint64_t));
      off_t offset = elementsProcessed * sizeof(uint64_t);
      size_t runBytes = runSize * sizeof(uint64_t);
      checkedPread(fdInput, &run[0], runBytes, offset);
      std::sort(&run[0], &run[runSize]);
      checkedPwrite(fdOutput, &run[0], runBytes, offset);
      posix_fadvise(fdInput, offset, runBytes, POSIX_FADV_DONTNEED);
      runs.push_back({.offset = offset, .size = runSize});
      elementsProcessed += runSize;
    }
  } //variables will go out of scope and the memory will be freed

  //merge runs
  #ifdef DEBUG
  std::clog << "merging runs..." << std::endl;
  #endif
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
    std::unique_ptr<uint8_t[]> buffer(new (std::nothrow) uint8_t[usedMemory]);
    if(!buffer) {
      std::cerr << "unable to allocate buffer" << std::endl;
      exit(1);
    }
    while(static_cast<size_t>(offset) < runBytes) {
      size_t chunkSize = std::min(runBytes - offset, usedMemory);
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

void checkedPread(int fd, void* buf, size_t count, off_t offset) {
  size_t bytesRead = 0;
  while(bytesRead != count) { 
    ssize_t ret = pread(fd, (uint8_t*)buf + bytesRead, count - bytesRead, offset + bytesRead);
    if(ret < 0) {
      std::cerr << "I/O error while reading from file descriptor " << fd << ": "
                << strerror(errno) << std::endl;
      exit(1);
    }
    if(ret == 0) {
      std::cerr << "unexpected EOF while reading from file descriptor " << fd << std::endl;
      exit(1);
    }
    bytesRead += ret;
  }
}


void checkedPwrite(int fd, void* buf, size_t count, off_t offset) {
  size_t bytesWritten = 0;
  while(bytesWritten != count) { 
    ssize_t ret = pwrite(fd, (uint8_t*)buf + bytesWritten, count - bytesWritten, offset + bytesWritten);
    if(ret < 0) {
      std::cerr << "I/O error while writing to file descriptor " << fd << ": "
                << strerror(errno) << std::endl;
      exit(1);
    }
    bytesWritten += ret;
  }
}

runDescriptor mergeRuns(const runList& runs, int fd, uint64_t runsMergedPerSweep) {
  //necessary for avoiding "unused parameters" compiler warning...
  int unused = fd + runsMergedPerSweep;
  runs[unused];
  //for now, simply return the last run
  return runs.back();
  //TODO: implement the merge phase
}

} //namespace dbImpl
