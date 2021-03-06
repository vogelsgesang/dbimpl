#ifndef _BUFFER_FRAME_H_
#define _BUFFER_FRAME_H_

#include <cstdint>
#include "pthread.h"

namespace dbImpl {
  //forward declaration of class BufferManager.
  //Necessary for "friend class BufferManager".
  //I could also include BufferManager.h but doing so would hurt build time.
  class BufferManager;

  class BufferFrame {
    friend class BufferManager;
    public:
      //deleted copy constructor => non copyable
      BufferFrame(const BufferFrame&) = delete;
      //deleted operator= => non copyable
      const BufferFrame& operator=(const BufferFrame&) = delete;

      const uint64_t pageId;
      
      // constructor
      BufferFrame(uint64_t pageId, uint64_t bufferSize);
      // destructor
      virtual ~BufferFrame();
      // returns data from page
      uint8_t* getData();


      bool isUsed();

    private:
      bool dirty;
      unsigned int users;
      // pointer to the actual data
      uint8_t* data;
      // frame's lock
      pthread_rwlock_t latch;
      // locks this frame with a write or read lock
      void lock(bool exclusive);
      // tries to acquire the lock. does not block.
      // returns true if the lock was acquired and false otherwise
      bool tryLock(bool exclusive);
      // unlocks frame
      void unlock();
  };
}

#endif //_BUFFER_FRAME_H_
