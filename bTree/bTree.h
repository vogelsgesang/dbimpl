#ifndef _B_TREE_H_
#define _B_TREE_H_

#include <cstdint>
#include <vector>
#include <iterator>
#include <boost/optional.hpp>
#include <limits>
#include <functional>
#include "buffer/bufferManager.h"

namespace dbImpl {

  template<typename K, typename Comp = std::less<K>>
  class BTree {

    private:
      inline BufferFrame* createNewRoot();
      BufferFrame* traverseToLeaf(K key, bool exclusiveLeaf);

      uint64_t rootPID;
      uint64_t nextFreePage = 0;
      BufferManager& bufferManager;
      uint64_t elements; //number of elements --> needed for BTree Test
      uint64_t maxNodeSize;
      const Comp& smaller;

    public:
      BTree<K, Comp>(BufferManager& bm, const Comp& comp = Comp(), uint64_t maxNodeSize = std::numeric_limits<uint64_t>::max());
      bool insert(K key, uint64_t tid);
      bool erase(K key);
      boost::optional<uint64_t> lookup(K key); //Returns a TID or indicates that the key was not found.

      std::vector<uint64_t> lookupRange(K key1, K key2);

      inline uint64_t size(){ return elements;}

      void exportAsDot(std::ostream& out);
  };
}

#include "bTree.inl.cpp"

#endif //_B_TREE_H_
