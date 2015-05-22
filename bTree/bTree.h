#ifndef _B_TREE_H_
#define _B_TREE_H_

#include <cstdint>
#include "utils/optional.h"

namespace dbImpl{


template<typename K,typename Comp>
class BTree {

private:
  struct Node{
    uint64_t lsn;
    uint64_t upper; //upper page of right-most child
    uint64_t count(0); //number of entries
    std::pair<K,uint64_t> keyChildPIDPairs[calculateMaxChildren(false)];
  };

  struct Leaf{
     uint64_t lsn;
     uint64_t isLeaf(~0); //mark as Leaf
     Leaf* next; //next leaf node
     uint64_t count(0); //number of entries
     std::pair<K,uint64_t> keyTIDPairs[calculateMaxChildren(true)];
  };

  inline uint64_t calculateMaxChildren(bool isLeaf);
  inline bool Node::isFull();
  inline bool Node::isLeaf();
  inline uint64_t Node::findKeyPos(const K key);
  inline uint64_t Leaf::findKeyPos(const K key);


  Node root*;
  Comparator smaller;

public:

  bool insert(K key, uint64_t tid);
  bool erase(K key);
  std::experimental::optional<uint64_t> lookup (K key); //Returns a TID or indicates that the key was not found.
  std::vector<uint64_t>::iterator lookupRange(K key1, K key2);
};


}
#include "bTree/bTreeImpl.h"


#endif //_B_TREE_H_