#ifndef _RECORD_HPP_
#define _RECORD_HPP_

#include <cstring>
#include <cstdlib>

namespace dbImpl {

  // A simple Record implementation
  class Record {
    private:
      unsigned len;
      char* data;

    public:
      // Assignment Operator: deleted
      Record& operator=(Record& rhs) = delete;
      // Copy Constructor: deleted
      Record(Record& t) = delete;
      // Move Constructor
      Record(Record&& t) : len(t.len), data(t.data) {
        t.data = nullptr;
        t.len = 0;
      }
      // Constructor
      Record(unsigned len, const char* const ptr) : len(len) {
        data = new char[len];
        memcpy(data, ptr, len);
      }
      // Destructor
      ~Record() {
        if(data != nullptr) {
          delete data;
        }
      }
      // Get pointer to data
      const char* getData() const {
        return data;
      }
      // Get data size in bytes
      unsigned getLen() const {
        return len;
      }
  };

}

#endif
