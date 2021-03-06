#ifndef _CHECKED_IO_HPP_
#define _CHECKED_IO_HPP_

#include <sys/types.h>
#include <system_error>

namespace dbImpl {

  //thrown when a read reaches the end of a file
  class UnexpectedEofError : public std::runtime_error {
    public:
      UnexpectedEofError(int fd);
    private:
      static std::string buildErrorMsg(int fd);
  };

  //wraps pread with error checking code.
  //if an error occurs abort() will be called.
  void checkedPread(int fd, void* buf, size_t count, off_t offset);
  //wraps pread with error checking code.
  //if an error occurs abort() will be called.
  void checkedPwrite(int fd, void* buf, size_t count, off_t offset);

}

#endif
