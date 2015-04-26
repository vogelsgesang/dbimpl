#ifndef __TWO_Q_HPP__
#define __TWO_Q_HPP__

#include <cstdint> // uint64_t
#include <unordered_map>
#include <queue>
#include <list>

namespace dbImpl {
  class TwoQ {
    // Stores pageIds according to the 2Q-strategy.

  private:
    std::list<uint64_t> fifoQueue;
    std::list<uint64_t> lruQueue;
    
    typedef std::list<uint64_t>::iterator list_iterator;
    
    //Hashmaps for O(1) access
    std::unordered_map<uint64_t, list_iterator> fifoMap;
    std::unordered_map<uint64_t, list_iterator> lruMap;
    
  public:
    
    // Evicts a page out of the queues.
    uint64_t evict();
    
    // Coordinates the access of a page.
    void access(uint64_t pageId);
    
    bool empty();
  };
}
#endif
