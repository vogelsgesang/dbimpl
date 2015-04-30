
#ifndef __BUFFER_FRAME_H
#define __BUFFER_FRAME_H

#include <cstdint>
#include "pthread.h"

namespace dbImpl {
  class BufferFrame {
  public:
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
