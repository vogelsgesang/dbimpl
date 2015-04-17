#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
  //avoid overflows
  if(memSize & (0xffffL << 48)) {
    std::cerr << "memory size too big: an overflow would occurr" << std::endl;
    return 1;
  }
  memSize <<= 16; //convert Megabyte to Byte
  std::cout << "Memory size: " << memSize << " Bytes" <<std::endl;
  //open the input and output files
  int fdIn = open(argv[1], O_RDONLY);
  if(fdIn < 0) {
    std::cerr << "unable to open file '" << argv[1] << "': " << strerror(errno) << std::endl;
    return 1;
  }
  int fdOut = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
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
  std::cout << inStat.st_size << " Bytes" << std::endl;
  //call sort algorithm
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
