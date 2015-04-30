#include "buffer/bufferFrame.h"
#include "buffer/definitions.h"

#include <iostream>
#include <unistd.h>
#include <stdlib.h>

namespace dbImpl {

  BufferFrame::BufferFrame(uint64_t pageId, int segmentFd) : pageId(pageId), segmentFd(segmentFd)   {
    pthread_rwlock_init(&latch, nullptr);
    
    // initially read data from disk
    data = malloc(PAGE_SIZE);
    int offset = pageId & offsetBitMask;
    if (pread(segmentFd, data, PAGE_SIZE, offset * PAGE_SIZE) == -1) {
      std::cerr << "error while reading page " << pageId << ": errno " << errno << std::endl;
    }
  }

  BufferFrame::~BufferFrame() {
    free(data);
    pthread_rwlock_destroy(&latch);
  }

  void* BufferFrame::getData() {
    return data;
  }

  void BufferFrame::lock(bool exclusive) {
    if (exclusive) {
      pthread_rwlock_wrlock(&latch);
    } else {
      pthread_rwlock_rdlock(&latch);
    }
  }

  void BufferFrame::unlock() {
      pthread_rwlock_unlock(&latch);
  }


}