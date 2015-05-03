#include "buffer/bufferFrame.h"
#include "buffer/definitions.h"

#include <iostream>
#include <unistd.h>
#include <stdlib.h>

namespace dbImpl {

  BufferFrame::BufferFrame(uint64_t pageId) : pageId(pageId) {
    pthread_rwlock_init(&latch, nullptr);
    data = malloc(PAGE_SIZE);
    dirty = false;
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

  bool BufferFrame::tryLock(bool exclusive) {
    int ret;
    if (exclusive) {
      ret = pthread_rwlock_trywrlock(&latch);
    } else {
      ret = pthread_rwlock_tryrdlock(&latch);
    }
    return ret == 0;
  }

  void BufferFrame::unlock() {
      pthread_rwlock_unlock(&latch);
  }


}
