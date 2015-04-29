#ifndef __BUFFER_MANAGER_H
#define __BUFFER_MANAGER_H

#include <cstdint>
#include <unordered_map>
#include "buffer/bufferFrame.h"
#include "buffer/twoQ.h"

namespace dbImpl {
  class BufferManager {
  public:
    BufferManager(uint64_t size);
    virtual ~BufferManager();

      uint64_t size;

    BufferFrame& fixPage(uint64_t pageId, bool exclusive);

    void unfixPage(BufferFrame& frame, bool isDirty);


  private:
    std::unordered_map<uint64_t, BufferFrame> frames;
    TwoQ<uint64_t> twoQ;
  };
}

#endif //__BUFFER_MANAGER_H
