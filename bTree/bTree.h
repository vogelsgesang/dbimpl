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
      uint64_t upper; //upper page of right-most child
      uint64_t count; //number of entries
      std::pair<K, uint64_t> keyChildPIDPairs[1];

      inline bool isFull();
      inline bool isLeaf();
      inline K getMaxKey();
      uint64_t findKeyPos(const K key);
      uint64_t split(uint64_t curPID, BufferFrame* newFrame, BufferFrame* parent, K key);
      void insertKey(K key, uint64_t leftChildPID, uint64_t rightChildPID);
      Node() : count(0) {}
    };

    struct Leaf {
      uint64_t isLeaf; //mark as Leaf
      uint64_t next; //next leaf node
      uint64_t count; //number of entries
      std::pair<K, uint64_t> keyTIDPairs[1];

      inline uint64_t findKeyPos(const K key);
      inline bool isFull();
      inline K getMaxKey();
      bool insertKey(K key, uint64_t tid);
      bool deleteKey(K key);
      BufferFrame* split(uint64_t curPID, BufferFrame* newFrame, BufferFrame* parent);
      Leaf()
        : isLeaf(std::numeric_limits<uint64_t>::max())
        , next(std::numeric_limits<uint64_t>::max())
        , count(0) {};
    };

    inline BufferFrame* createNewRoot();
    BufferFrame* traverseToLeaf(K key, bool exclusiveLeaf);

    uint64_t rootPID;
    uint64_t nextFreePage = 0;
    BufferManager& bufferManager;
    uint64_t elements; //number of elements --> needed for BTree Test

  public:
    BTree<K, Comp>(BufferManager& bm);
    bool insert(K key, uint64_t tid);
    bool erase(K key);
    optional<uint64_t> lookup(K key); //Returns a TID or indicates that the key was not found.

    std::vector<uint64_t> lookupRange(K key1, K key2);

    inline uint64_t size(){ return elements;}

    static Comp smaller;
    static bool isEqual(K key1, K key2);
};
}

#include "bTree.inl.cpp"

#endif //_B_TREE_H_
