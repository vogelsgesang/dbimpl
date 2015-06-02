#ifndef _B_TREE_NODE_H_
#define _B_TREE_NODE_H_

#include <cstdint>
#include <utility>
#include <limits>
#include "buffer/bufferFrame.h"

namespace dbImpl {

  template<typename K,typename Comp>
  struct Node {
    uint64_t count; //number of entries
    uint64_t leafMarker; //set to 0 for all inner nodes
    uint64_t next; //for inner nodes: upper page of right-most child; for leafs: PID of next page
    std::pair<K, uint64_t> keyValuePairs[1];

    inline bool isLeaf();
    uint64_t findKeyPos(const K key, const Comp& smaller);
    bool insertKey(K key, uint64_t tid, const Comp& smaller);
    void insertInnerKey(K key, uint64_t leftChildPID, uint64_t rightChildPID, const Comp& smaller);
    bool deleteKey(K key, const Comp& smaller);
    K split(uint64_t ownPID, BufferFrame* newFrame, BufferFrame* parent, const Comp& comp); //returns the used split key
    Node(bool isLeaf)
      : count(0)
      , leafMarker(isLeaf)
      , next(std::numeric_limits<uint64_t>::max()) {}
  };
}

#include "node.inl.cpp"

#endif
