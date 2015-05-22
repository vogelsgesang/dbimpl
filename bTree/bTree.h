#ifndef _B_TREE_H_
#define _B_TREE_H_

#include <cstdint>
#include "utils/optional.h"

namespace dbImpl {

template<typename K, typename Comp>
class BTree {

private:
  inline uint64_t calculateMaxChildren(bool isLeaf);
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
  };

  Comp smaller;
  Node* root;

public:

  bool insert(K key, uint64_t tid);
  bool erase(K key);
  std::experimental::optional<uint64_t> lookup(K key); //Returns a TID or indicates that the key was not found.
  std::vector<uint64_t>::iterator lookupRange(K key1, K key2);
};

}
#include "bTree.inl.cpp"

#endif //_B_TREE_H_
