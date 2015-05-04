#ifndef __TWO_Q_HPP__
#define __TWO_Q_HPP__

#include <unordered_map>
#include <list>
#include <ostream>

namespace dbImpl {

  /*
   * Manages elements according to the 2Q-strategy.
   */
  template<typename T>
  class TwoQ {
    public:
      // Evicts a page out of the queues.
      T evict();
      
      // Informs the queue about the access of a page.
      void access(T pageId);
      
      bool empty();

    private:
      // FIFO and LRU queues are managed using std::lists
      std::list<T> fifoQueue;
      std::list<T> lruQueue;
      
      typedef typename std::list<T>::iterator list_iterator;
      
      // Hashmaps are used for O(1) access to the queue elements
      std::unordered_map<T, list_iterator> fifoMap;
      std::unordered_map<T, list_iterator> lruMap;

      template<typename T2>
      friend std::ostream& operator<< (std::ostream& stream, const TwoQ<T2>& queue);
  };

  template<typename T>
  std::ostream& operator<< (std::ostream& out, const TwoQ<T>& queue);
}

#include "buffer/twoQ.inl.cpp"

#endif
