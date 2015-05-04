#include "checkedIO.h"

#include <sstream>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace dbImpl {

UnexpectedEofError::UnexpectedEofError(int fd)
: std::runtime_error(buildErrorMsg(fd)) {}

std::string UnexpectedEofError::buildErrorMsg(int fd) {
  std::ostringstream msg;
  msg << "unexpected EOF while reading from file descriptor " << fd;
  return msg.str();
}


void checkedPread(int fd, void* buf, size_t count, off_t offset) {
  size_t bytesRead = 0;
  while(bytesRead != count) { 
    ssize_t ret = pread(fd, (uint8_t*)buf + bytesRead, count - bytesRead, offset + bytesRead);
    if(ret < 0) {
      int occurredErrno = errno;
      errno = 0; //reset, so that later calls can succeed
      std::ostringstream msg;
      msg << "I/O error while reading from file descriptor " << fd;
      throw std::system_error(std::error_code(occurredErrno, std::system_category()), msg.str());
    }
    if(ret == 0) {
      throw dbImpl::UnexpectedEofError(fd);
    }
    bytesRead += ret;
  }
}


void checkedPwrite(int fd, void* buf, size_t count, off_t offset) {
  size_t bytesWritten = 0;
  while(bytesWritten != count) { 
    ssize_t ret = pwrite(fd, (uint8_t*)buf + bytesWritten, count - bytesWritten, offset + bytesWritten);
    if(ret < 0) {
      int occurredErrno = errno;
      errno = 0; //reset, so that later calls can succeed
      std::ostringstream msg;
      msg << "I/O error while writing to file descriptor " << fd;
      throw std::system_error(std::error_code(occurredErrno, std::system_category()), msg.str());
    }
    bytesWritten += ret;
  }
}

} //namespace dbImpl
