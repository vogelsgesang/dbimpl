#ifndef _RECORD_HPP_
#define _RECORD_HPP_

#include <cstdint>
#include <cstring>
#include <cstdlib>

namespace dbImpl {

  // A simple Record implementation
  class Record {
    private:
      unsigned len;
      uint8_t* data;

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
      Record(unsigned len, const uint8_t* const ptr = nullptr) : len(len) {
        data = new uint8_t[len];
        if(ptr != nullptr) {
          memcpy(data, ptr, len);
        }
      }
      // Destructor
      ~Record() {
        if(data != nullptr) {
          delete[] data;
        }
      }
      // Get pointer to data
      uint8_t* getData() {
        return data;
      }
      const uint8_t* getData() const {
        return data;
      }
      // Get data size in bytes
      unsigned getLen() const {
        return len;
      }
  };

}

#endif
