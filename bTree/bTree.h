#ifndef _B_TREE_H_
#define _B_TREE_H_

#include <cstdint>
#include <vector>
#include <iterator>
#include <boost/optional.hpp>
#include <limits>
#include <functional>
#include "buffer/bufferManager.h"

using boost::optional;

namespace dbImpl {

template<typename K, typename Comp = std::less<K>>
class BTree {

  private:
    struct Node {
      uint64_t count; //number of entries
      uint64_t leafMarker; //set to 0 for all inner nodes
      uint64_t next; //for inner nodes: upper page of right-most child; for leafs: PID of next page
      std::pair<K, uint64_t> keyValuePairs[1];

      inline bool isLeaf();
      uint64_t findKeyPos(const K key);
      bool insertKey(K key, uint64_t tid);
      void insertInnerKey(K key, uint64_t leftChildPID, uint64_t rightChildPID);
      bool deleteKey(K key);
      K split(uint64_t ownPID, BufferFrame* newFrame, BufferFrame* parent); //returns the used split key
      Node(bool isLeaf)
        : count(0)
        , leafMarker(isLeaf)
        , next(std::numeric_limits<uint64_t>::max()) {}
    };

    inline BufferFrame* createNewRoot();
    BufferFrame* traverseToLeaf(K key, bool exclusiveLeaf);

    uint64_t rootPID;
    uint64_t nextFreePage = 0;
    BufferManager& bufferManager;
    uint64_t elements; //number of elements --> needed for BTree Test
    uint64_t maxNodeSize;

  public:
    BTree<K, Comp>(BufferManager& bm, uint64_t maxNodeSize = std::numeric_limits<uint64_t>::max());
    bool insert(K key, uint64_t tid);
    bool erase(K key);
    optional<uint64_t> lookup(K key); //Returns a TID or indicates that the key was not found.

    std::vector<uint64_t> lookupRange(K key1, K key2);

    inline uint64_t size(){ return elements;}

    void exportAsDot(std::ostream& out);

    static Comp smaller;
    static bool isEqual(K key1, K key2);
};
template<typename K, typename Comp>
Comp BTree<K, Comp>::smaller;
}

#include "bTree.inl.cpp"

#endif //_B_TREE_H_
