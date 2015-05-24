#ifndef _B_TREE_H_
#define _B_TREE_H_

#include <cstdint>
//#include "utils/optional.h"
#include <boost/optional.hpp>

using boost::optional;

namespace dbImpl {

template<typename K, typename Comp>
class BTree {


private:

  struct Node {
    uint64_t lsn;
    uint64_t upper; //upper page of right-most child
    uint64_t count = 0; //number of entries
    std::pair<K, uint64_t> keyChildPIDPairs[1];
    inline bool isFull();
    inline bool isLeaf();
    inline uint64_t findKeyPos(const K key);
  };

  struct Leaf {
    uint64_t lsn;
    uint64_t isLeaf = ~0; //mark as Leaf
    Leaf* next; //next leaf node
    uint64_t count = 0; //number of entries
    std::pair<K, uint64_t> keyTIDPairs[1];
    inline uint64_t findKeyPos(const K key);
    inline bool isFull();
    void insertIntoLeaf(K key, uint64_t tid);
    bool deleteKeyInLeaf(K key);
  };


  uint64_t rootPID;
  BufferManager& bufferManager;

public:
  BTree<K, Comp>(BufferManager& bm);
  bool insert(K key, uint64_t tid);
  bool erase(K key);
  optional<uint64_t> lookup(K key); //Returns a TID or indicates that the key was not found.
  std::vector<uint64_t>::iterator lookupRange(K key1, K key2);

  static Comp smaller;
};

}
#include "bTree.inl.cpp"

#endif //_B_TREE_H_
