
#ifndef __BUFFER_FRAME_H
#define __BUFFER_FRAME_H

#include <cstdint>
#include "pthread.h"

namespace dbImpl {
  class BufferFrame {
    public:
      //deleted copy constructor => non copyable
      BufferFrame(const BufferFrame&) = delete;
      //deleted operator= => non copyable
      const BufferFrame& operator=(const BufferFrame&) = delete;

      uint64_t pageId;
      // file descriptor for segment file, wherein page is stored
      int segmentFd;
      
      // constructor
      BufferFrame(uint64_t pageId, int segmentFd);
      // destructor
      virtual ~BufferFrame();
      // returns data from page
      void* getData();
      // locks frame with an write or read lock whether lock is exclusive or not
      void lock(bool exclusive);
      // unlocks frame
      void unlock();
      
    private:
      // actual data
      void* data;
      // frame's lock
      pthread_rwlock_t latch;
  };
}

#endif //__BUFFER_FRAME_H
