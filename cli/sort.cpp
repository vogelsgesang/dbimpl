#include <iostream>
#include <cstdlib>
#include <cerrno>

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
    std::cerr << "unable to parse memory size: overflow" << std::endl;
    return 1;
  }
  memSize <<= 16; //convert Megabyte to Byte
  std::cout << "Memory size: " << memSize << " Bytes" <<std::endl;
  //open the input and output files
  //TODO: ---
}

void printHelp() {
  std::cerr << "Usage:" << std::endl
            << "  sort <infile> <outfile> <memoryBufferInMB>" << std::endl;
}
