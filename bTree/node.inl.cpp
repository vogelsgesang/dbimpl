#include "node.h"
#include <cstring>
#include "utils/isEqual.h"

namespace dbImpl {

template<typename K, typename Comp>
inline bool Node<K, Comp>::isLeaf() {
  return leafMarker;
}

template<typename K, typename Comp>
inline uint64_t Node<K, Comp>::findKeyPos(const K key, const Comp& smaller) {
  uint64_t left = 0;
  uint64_t right = count;
  while (right != left) {
    uint64_t mid = left + ((right - left) / 2);
    if (smaller(keyValuePairs[mid].first, key)) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }
  return left;
}

template<typename K, typename Comp>
bool Node<K, Comp>::insertKey(K key, uint64_t tid, const Comp& smaller) {
  uint64_t pos = findKeyPos(key, smaller);

  //Check if key is not already stored in Tree. Count != pos avoids random matches with old memory data.
  if (count != pos && isEqual(keyValuePairs[pos].first, key, smaller)) {
    // Key is already in tree
    return false;
  }
  std::memmove(&keyValuePairs[pos + 1], &keyValuePairs[pos],
      (count - pos) * sizeof(std::pair<K, uint64_t>)); // will probably only work in combination with Buffermanager
  std::pair < K, uint64_t > keyTIDpair(key, tid);
  keyValuePairs[pos] = keyTIDpair;
  count++;
  return true;
}

template<typename K, typename Comp>
void Node<K, Comp>::insertInnerKey(K key, uint64_t leftChildPID, uint64_t rightChildPID, const Comp& smaller) {
  //Insert Key with pointer to left child
  uint64_t pos = findKeyPos(key, smaller);
  std::memmove(&keyValuePairs[pos + 1], &keyValuePairs[pos],
      sizeof(std::pair<K, uint64_t>) * (count - pos));
  std::pair < K, uint64_t > keyPidPair(key, leftChildPID);
  keyValuePairs[pos] = keyPidPair;

  //Update existing pointer to new (right) child
  if (pos == count) {
    next = rightChildPID;
  } else {
    keyValuePairs[pos + 1].second = rightChildPID;
  }
  count++;
}

template<typename K, typename Comp>
bool Node<K, Comp>::deleteKey(K key, const Comp& smaller) {
  uint64_t pos = findKeyPos(key, smaller);
  bool deleted = false;
  if (pos < count && isEqual(key, keyValuePairs[pos].first, smaller)) {
    deleted = true;
    count--;
    std::memmove(&keyValuePairs[pos], &keyValuePairs[pos + 1],
        (count - pos) * sizeof(std::pair<K, uint64_t>));
  }
  return deleted;
}

template<typename K, typename Comp>
K Node<K, Comp>::split(uint64_t ownPID, BufferFrame* newFrame, BufferFrame* parent, const Comp& smaller) {
  //Get new node
  Node<K, Comp>* newNode = reinterpret_cast<Node<K, Comp>*>(newFrame->getData());
  *newNode = Node<K, Comp>(this->isLeaf());
  //split current Node
  uint64_t mid = count / 2;
  std::memmove(&newNode->keyValuePairs[0], &keyValuePairs[mid],
      (count - mid) * sizeof(std::pair<K, uint64_t>));
  newNode->count = count - mid;
  newNode->next = next;
  if(this->isLeaf()) {
    count = mid;
    next = newFrame->pageId;
  } else {
    count = mid-1;
    next = keyValuePairs[count].second;
  }
  //biggest key of left node moves to parent
  K splitKey = keyValuePairs[mid-1].first;
  (reinterpret_cast<Node<K, Comp>*>(parent->getData()))
    ->insertInnerKey(splitKey, ownPID, newFrame->pageId, smaller);
  return splitKey;
}

}
