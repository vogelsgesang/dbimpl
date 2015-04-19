#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <algorithm>

#include "externalSort.h"

namespace dbImpl {

//wraps pread with error checking code.
//if an error occurs exit() will be called.
void checkedPread(int fd, void* buf, size_t count, off_t offset);
//wraps pread with error checking code.
//if an error occurs exit() will be called.
void checkedPwrite(int fd, void* buf, size_t count, off_t offset);

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
  /*
   * implementation decisions:
   *  * std::vector is avoided, since std::vector initializes its memory with 0 and
   *    this initialization would be useless overhead
   *  * all file accesses are done using the preferred block size as reported by fstat
   *  * posix_fadvise is used to give hints for tuning
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
  //form sorted runs
  {
    #ifdef DEBUG
    std::clog << "forming runs..." << std::endl;
    #endif
    posix_fadvise(fdInput, 0, size*sizeof(uint64_t), POSIX_FADV_SEQUENTIAL);
    if(int ret = posix_fallocate(fdOutput, 0, size * sizeof(uint64_t))) {
      std::cerr << "unable to allocate disk space: " << strerror(ret);
      exit(1);
    }
    std::unique_ptr<uint64_t[]> run(new uint64_t[usedMemory / sizeof(uint64_t)]);
    uint64_t elementsProcessed = 0;
    while(elementsProcessed < size) {
      uint64_t runSize = std::min(size - elementsProcessed, usedMemory / sizeof(uint64_t));
      uint64_t offset = elementsProcessed * sizeof(uint64_t);
      uint64_t runBytes = runSize * sizeof(uint64_t);
      #ifdef DEBUG
      std::clog << "offset: " << offset << " bytes, len: " << runSize << " elements" << std::endl;
      #endif
      checkedPread(fdInput, &run[0], runBytes, offset);
      std::sort(&run[0], &run[runSize]);
      checkedPwrite(fdOutput, &run[0], runBytes, offset);
      posix_fadvise(fdInput, offset, runBytes, POSIX_FADV_DONTNEED);
      elementsProcessed += runSize;
    }
  } //variables will go out of scope and the memory will be freed

  //merge runs
  //write result to fdOutput
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

}
