#include "buffer/bufferFrame.h"

#include <unistd.h>
#include <stdlib.h>
#include <system_error>

namespace dbImpl {

  BufferFrame::BufferFrame(uint64_t pageId, uint64_t pageSize) : pageId(pageId) {
    pthread_rwlock_init(&latch, nullptr);
    data = reinterpret_cast<uint8_t*> (malloc(pageSize));
    dirty = false;
    users = 0;
  }

  BufferFrame::~BufferFrame() {
    free(data);
    pthread_rwlock_destroy(&latch);
  }

  uint8_t* BufferFrame::getData() {
    return data;
  }

  void BufferFrame::lock(bool exclusive) {
    users++;
    int ret;
    if (exclusive) {
      ret = pthread_rwlock_wrlock(&latch);
    } else {
      ret = pthread_rwlock_rdlock(&latch);
    }
    if(ret != 0) {
      throw std::system_error(std::error_code(ret, std::system_category()), "unable to lock frame");
    }
  }

  bool BufferFrame::tryLock(bool exclusive) {
    int ret;
    if (exclusive) {
      ret = pthread_rwlock_trywrlock(&latch);
    } else {
      ret = pthread_rwlock_tryrdlock(&latch);
    }
    if(ret == 0) {
      return true;
    } else if(ret == EBUSY) {
      return false;
    } else {
      throw std::system_error(std::error_code(ret, std::system_category()), "unable to trylock frame");
    }
  }

  void BufferFrame::unlock() {
    int ret = pthread_rwlock_unlock(&latch);
    if(ret != 0) {
      throw std::system_error(std::error_code(ret, std::system_category()), "unable to unlock frame");
    }
    users--;
  }

  bool BufferFrame::isUsed() {
    return users > 0;
  }

}
