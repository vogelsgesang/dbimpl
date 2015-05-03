#include "buffer/bufferFrame.h"
#include "buffer/definitions.h"

#include <iostream>
#include <unistd.h>
#include <stdlib.h>

namespace dbImpl {

  BufferFrame::BufferFrame(uint64_t pageId) : pageId(pageId) {
    pthread_rwlock_init(&latch, nullptr);
    data = malloc(PAGE_SIZE);
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
