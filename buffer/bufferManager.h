#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_

#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "buffer/bufferFrame.h"
#include "buffer/twoQ.h"


namespace dbImpl {
  class BufferManager {
  public:
    // constructor
    BufferManager(uint64_t size);
    // destructor
    virtual ~BufferManager();
    
    // returns BufferFrame for given pageId
    // if exclusive==true, the page is write-locked otherwise read-locked
    // if there's no free space in buffer, an old page will be evicted following the twoQ strategy
    BufferFrame& fixPage(uint64_t pageId, bool exclusive);

    // takes a BufferFrame and writes it on disk if it is dirty
    void unfixPage(BufferFrame& frame, bool isDirty);

    //  the number of bits contained stored in one page
    static const int pageSize;

  private:
    static const int segmentBits;
    static const uint64_t offsetBitMask;

    // maximum number of pages in buffer
    uint64_t size;
    // hash map storing the BufferFrames
    std::unordered_map<uint64_t, BufferFrame> frames;
    // file descriptor for the directory storing all segment files
    int folderFd;
    // hash map storing file descriptors for all created segment files
    std::unordered_map<uint64_t, int> segmentFds;
    // implementation of twoQ strategy
    TwoQ<uint64_t> twoQ;
    std::condition_variable twoQAccessed;
    // the global mutex for the maps and the twoQ
    std::mutex globalMutex;
  };
}

#endif //_BUFFER_MANAGER_H_
