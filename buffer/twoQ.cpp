#include "buffer/twoQ.h"

#include <exception>

namespace dbImpl {
  
  uint64_t TwoQ::evict() {
    uint64_t evicted;
    if (!fifoQueue.empty()) {
      evicted = fifoQueue.back();
      fifoMap.erase(evicted);
      fifoQueue.pop_back();
    } else if(!lruQueue.empty()) {
      evicted = lruQueue.back();
      lruMap.erase(evicted);
      lruQueue.pop_back();
    } else {
      throw std::exception();
    }

    return evicted;
  }
  
  // Return PageInfo??
  void TwoQ::access(uint64_t pageId) {
    // Check if in memory
    
    auto it = lruMap.find(pageId);
    
    // in LRU?
    if (it != lruMap.end()) {
      // Move to beginning
      lruQueue.splice(lruQueue.begin(), lruQueue, it->second);
    } else {
      it = fifoMap.find(pageId);
      // in Fifo?
      if (it != fifoMap.end()) {
        // add to lru queue
        lruQueue.push_front(*it->second);
        lruMap[pageId] = lruQueue.begin();
        
        // Remove from FIFO
        fifoQueue.erase(it->second);
        fifoMap.erase(pageId);
      } else {
        // Not in Memory
        // Add to FiFO
        fifoQueue.push_front(pageId);
        fifoMap[pageId] = fifoQueue.begin();
      }
    }
  }
  
  bool TwoQ::empty() {
    return lruQueue.empty() && fifoQueue.empty();
  }
  
}
