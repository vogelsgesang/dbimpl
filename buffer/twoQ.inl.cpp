#include "buffer/twoQ.h"

#include <stdexcept>

namespace dbImpl {
  
  template<typename T>
  T TwoQ<T>::evict() {
    T evicted;
    if (!fifoQueue.empty()) {
      evicted = fifoQueue.back();
      fifoMap.erase(evicted);
      fifoQueue.pop_back();
    } else if(!lruQueue.empty()) {
      evicted = lruQueue.back();
      lruMap.erase(evicted);
      lruQueue.pop_back();
    } else {
      throw std::runtime_error("evict() called on empty 2Q");
    }

    return evicted;
  }
  
  template<typename T>
  void TwoQ<T>::access(T pageId) {
    // Check if in LRU?
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

  template<typename T>
  void TwoQ<T>::erase(T pageId) {
    //erase from LRU queue
    auto it1 = lruMap.find(pageId);
    if(it1 != lruMap.end()) {
      lruQueue.erase(it1->second);
      lruMap.erase(it1);
    }
    //erase from FIFO queue
    auto it2 = fifoMap.find(pageId);
    if(it2 != fifoMap.end()) {
      fifoQueue.erase(it2->second);
      fifoMap.erase(it2);
    }
  }
  
  template<typename T>
  bool TwoQ<T>::empty() {
    return lruQueue.empty() && fifoQueue.empty();
  }

  template<typename T>
  std::ostream& operator<< (std::ostream& out, const TwoQ<T>& queue) {
    out << "fifo: ";
    if(queue.fifoQueue.empty()) {
      out << "empty ";
    } else {
      auto it = queue.fifoQueue.begin();
      while(it != queue.fifoQueue.end()) {
        out << *it;
        it++;
        if(it != queue.fifoQueue.end()) {
          out << ", ";
        }
      }
    }
    out << "; lru: ";
    if(queue.lruQueue.empty()) {
      out << "empty";
    } else {
      auto it = queue.lruQueue.begin();
      while(it != queue.lruQueue.end()) {
        out << *it;
        it++;
        if(it != queue.lruQueue.end()) {
          out << ", ";
        }
      }
    }
    return out;
  }
  
}
