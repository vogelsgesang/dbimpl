#include <iostream>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace dbImpl {

void checkedPread(int fd, void* buf, size_t count, off_t offset) {
  size_t bytesRead = 0;
  while(bytesRead != count) { 
    ssize_t ret = pread(fd, (uint8_t*)buf + bytesRead, count - bytesRead, offset + bytesRead);
    if(ret < 0) {
      std::cerr << "I/O error while reading from file descriptor " << fd << ": "
                << strerror(errno) << std::endl;
      abort();
    }
    if(ret == 0) {
      std::cerr << "unexpected EOF while reading from file descriptor " << fd << std::endl;
      abort();
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
      abort();
    }
    bytesWritten += ret;
  }
}

} //namespace dbImpl
