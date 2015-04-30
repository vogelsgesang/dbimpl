#include "buffer/bufferManager.h"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "buffer/definitions.h"

namespace dbImpl {
  
  BufferManager::BufferManager(uint64_t size) : size(size) {
    frames.reserve(size);
  }
  
  BufferManager::~BufferManager() {
    frames.clear();
    for (auto segment: segmentFds) {
      close(segment.second);
    }
  }
  
  BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
    lock.lock();
    
    if (frames.count(pageId) == 0) { // page is not in buffer
      
      if (frames.size() >= size) {  // buffer is already full
        // get next page to evict from twoQ
        uint64_t evictedPage = twoQ.evict();
        // delete reference from hash table
        frames.erase(evictedPage);
      }
      uint64_t segmentId = pageId >> segmentBits;
      int segmentFd;
      
      if (segmentFds.count(segmentId) == 1) { // segment file exists
        segmentFd = segmentFds.at(segmentId);
      } else { // segment file does not exist
        // create segment directory
        if (mkdir("segments", S_IRWXU) == -1 && errno != 17) {
          std::cerr << "error while creating directory " << pageId << ": errno " << errno << std::endl;
        }
        segmentFd = open(("segments/" + std::to_string(segmentId)).c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
        if (segmentFd == -1) {
          std::cerr << "error while opening segment " << segmentId << ": errno " << errno << std::endl;
        }
        segmentFds[segmentId] = segmentFd;
      }
      // insert page into unordered_map (completely copied from michael, my solution didn't work.)
      frames.emplace(std::piecewise_construct, std::forward_as_tuple(pageId), std::make_tuple(pageId, segmentFd));
    }
    
    // move in fifo or when already in fifo move to lru
    twoQ.access(pageId);
    BufferFrame& frame = frames.at(pageId);
    
    lock.unlock();
    frame.lock(exclusive);
    
    return frame;
  }
  
  void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
    lock.lock();
    
    if (isDirty) { // if page is dirty, write it to disk
      int segmentId = frame.pageId << segmentBits;
      int segmentFd = segmentFds.at(segmentId);
      int offset = frame.pageId & offsetBitMask;
      
#if __unix
      posix_fallocate(segmentFd, offset * PAGE_SIZE, PAGE_SIZE);
#endif
      if (pwrite(segmentFd, frame.getData(), PAGE_SIZE, offset * PAGE_SIZE) == -1) {
        std::cerr << "error while writing segment " << segmentId << " to disk: " << strerror(errno) << std::endl;
      }
    }
    
    frame.unlock();
    lock.unlock();
  }
  
}