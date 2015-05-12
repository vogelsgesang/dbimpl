#ifndef _SEQUENCE_WRITER_HPP_
#define _SEQUENCE_WRITER_HPP_

#include <iostream>
#include <memory>
#include "utils/checkedIO.h"

namespace dbImpl {

  //writes a sequence of values to disk
  template<typename T>
  class SequenceWriter {
    public:
      //offset is measured in bytes; bufferSize is measured in sizeof(T)
      SequenceWriter(int fd, uint64_t fileOffset, uint64_t bufferSize)
        : fd(fd),
          fileOffset(fileOffset),
          buffer(new T[bufferSize]),
          bufferSize(bufferSize),
          bufferOffset(0),
          elementCnt(0)
        {}

      //appends an element to the buffer and flushes the buffer if it is full
      void append(T value) {
        buffer[bufferOffset] = value;
        bufferOffset++;
        elementCnt++;
        if(bufferOffset >= bufferSize) {
          flush();
        }
      }

      //flushes the buffer
      void flush() {
        checkedPwrite(fd, &buffer[0], bufferSize*sizeof(T), fileOffset);
        fileOffset += bufferSize*sizeof(T);
        bufferOffset = 0;
      }

      uint64_t getElementCount() {
        return elementCnt;
      }

    private:
      int fd;
      uint64_t fileOffset; //the file offset in bytes
      std::unique_ptr<T[]> buffer;
      uint64_t bufferSize; //the size of the buffer in sizeof(T)
      uint64_t bufferOffset; //the buffer offset in sizeof(T)
      uint64_t elementCnt; //the number of elements written
  };

} //namespace dbImpl

#endif
