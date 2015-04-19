#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sorting/externalSort.h"

void printHelp();

int main(int argc, const char* argv[]) {
  if(argc < 4) {
    std::cerr << "Not enough arguments" << std::endl;
    printHelp();
    return 1;
  } else if (argc > 4) {
    std::cerr << "Too many arguments" << std::endl;
    printHelp();
    return 1;
  }
  //parse the memory size
  char* lastChar;
  uint64_t memSize = std::strtoull(argv[3], &lastChar, 10);
  if(*lastChar) {
    std::cerr << "unable to parse memory size: unexpected character \"" << *lastChar << "\"" << std::endl;
    return 1;
  }
  if(errno == ERANGE) {
    std::cerr << "unable to parse memory size: " << std::strerror(errno) << std::endl;
    return 1;
  }
  if(memSize <= 0) {
    std::cerr << "memory size <= 0" << std::endl;
    return 1;
  }
  //avoid overflows
  if(memSize & (0xfffffL << 44)) {
    std::cerr << "memory size too big: an overflow would occur" << std::endl;
    return 1;
  }
  memSize <<= 20; //convert Megabyte to Byte
  #ifdef DEBUG
  std::clog << "memory size: " << memSize << " Bytes" << std::endl;
  #endif
  //open the input and output files
  int fdIn = open(argv[1], O_RDONLY);
  if(fdIn < 0) {
    std::cerr << "unable to open file '" << argv[1] << "': " << strerror(errno) << std::endl;
    return 1;
  }
  int fdOut = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if(fdOut < 0) {
    std::cerr << "unable to open file '" << argv[2] << "': " << strerror(errno) << std::endl;
    return 1;
  }
  //get file size of input file
  int ret;
  struct stat inStat;
  if((ret = fstat(fdIn, &inStat)) != 0) {
    std::cerr << "unable to stat file '" << argv[1] << "': " << strerror(errno) << std::endl;
    return 1;
  }
  uint64_t nrIntegers = inStat.st_size/sizeof(uint64_t);
  #ifdef DEBUG
  std::clog << "size of input file: " << inStat.st_size << " Bytes" << std::endl;
  std::clog << "number of uint64_t integers: " << nrIntegers << std::endl;
  #endif
  //call sort algorithm
  dbImpl::externalSort(fdIn, nrIntegers, fdOut, memSize);
  //close file descriptors
  if((ret = close(fdIn)) != 0) {
    std::cerr << "unable to close file '" << argv[1] << "': " << strerror(errno) << std::endl;
    return 1;
  }
  if((ret = close(fdOut)) != 0) {
    std::cerr << "unable to close file '" << argv[1] << "': " << strerror(errno) << std::endl;
    return 1;
  }
}

void printHelp() {
  std::cerr << "Usage:" << std::endl
            << "  sort <infile> <outfile> <memoryBufferInMB>" << std::endl;
}
