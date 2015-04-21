#ifndef __SEQUENCE_READER_HPP__
#define __SEQUENCE_READER_HPP__

#include <iostream>
#include <memory>
#include "utils/checkedIO.h"

namespace dbImpl {

  //reads a sequence of elements from disk
  template<typename T>
  class SequenceReader {
    public:
      //offset is measured in bytes, bufferSize and size are measured in sizeof(T)
      SequenceReader(int fd, uint64_t offset, size_t size, uint64_t bufferSize)
        : fd(fd),
          fileOffset(offset),
          runSize(size),
          elementsProcessed(0),
          buffer(new T[bufferSize]),
          bufferSize(bufferSize),
          bufferOffset(0)
      {
        readBlockFromDisk();
      }

      bool empty() {
        return elementsProcessed >= runSize;
      }

      T consumeElement() {
        if(empty()) {
          std::cerr << "reading past the end of a run" << std::endl;
          abort();
        }
        //load new elements from run
        if(bufferOffset >= bufferSize) {
          readBlockFromDisk();
        }
        //return next element
        T value = buffer[bufferOffset];
        bufferOffset++;
        elementsProcessed++;
        return value;
      }

    private:
      int fd;
      uint64_t fileOffset; //the current offset within the file in bytes
      uint64_t runSize; //the size of this run in sizeof(T)
      uint64_t elementsProcessed; //how many elements where already processed
      std::unique_ptr<T[]> buffer;
      uint64_t bufferSize; //the size of the buffer in sizeof(T)
      uint64_t bufferOffset; //the buffer offset in sizeof(T)

      void readBlockFromDisk() {
        uint64_t remainingElements = runSize - elementsProcessed;
        uint64_t bytesToLoad = std::min(remainingElements, bufferSize) * sizeof(T); 
        checkedPread(fd, &buffer[0], bytesToLoad, fileOffset);
        fileOffset += bytesToLoad;
        bufferOffset = 0;
      }
  };

} //namespace dbImpl

#endif
