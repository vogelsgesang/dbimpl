#include "buffer/bufferManager.h"

namespace dbImpl {

  BufferManager::BufferManager(uint64_t size) : size(size) {
    frames.reserve(size);
  }

  BufferManager::~BufferManager() { }

  BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {

    return frames.find(pageId)->second;
  }

  void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {

  }

}