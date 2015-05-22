namespace dbImpl {
template<typename K, typename Comp>
inline uint64_t BTree<K, Comp>::calculateMaxChildren(bool isLeaf) {
  uint64_t nodeSize = 3 * sizeof(uint64_t);
  if (isLeaf)
    nodeSize += sizeof(Leaf*);
  return (BufferManager::pageSize - nodeSize) / (sizeof(std::pair<K, uint64_t>));
}


template<typename K, typename Comp>
inline bool BTree<K, Comp>::Node::isFull() {
  return count >= calculateMaxChildren(false);
}
template<typename K, typename Comp>
inline bool BTree<K, Comp>::Leaf::isFull() {
  return count >= calculateMaxChildren(true);
}

template<typename K, typename Comp>
inline bool BTree<K, Comp>::Node::isLeaf() {
  return upper == ~0;
}

template<typename K, typename Comp>
inline uint64_t BTree<K, Comp>::Node::findKeyPos(const K key) {
  uint64_t left = 0;
  uint64_t right = count;
  uint64_t mid;
  while (right != left) {
    mid = left + ((right - left) / 2);
    if (smaller(key, keyChildPIDPairs[mid]->first)) {
      right = mid;
    } else if (smaller(keyChildPIDPairs[mid]->first, key)) {
      left = mid + 1;
    } else {
      return mid + 1;
    }
  }
  return right;

}

template<typename K, typename Comp>
inline uint64_t BTree<K, Comp>::Leaf::findKeyPos(const K key) {
  uint64_t mid;
  uint64_t left = 0;
  uint64_t right = count;

  while (right != left) {
    mid = left + ((right - left) / 2);
    if (smaller(key, keyTIDPairs[mid]->first)) {
      right = mid ;
    } else if (smaller(keyTIDPairs[mid]->first, key)) {
      left = mid + 1;
    } else {
      return mid+1;
    }
  }
  return right;
}

template<typename K, typename Comp>
void BTree<K, Comp>::Leaf::insertIntoLeaf(K key, uint64_t tid) {
  uint64_t pos = findKeyInNode(key);
  memmove(keyTIDPairs[pos+1], keyTIDPairs[pos],(count-pos)*sizeof(std::pair<K,uint64_t>));
  keyTIDPairs[pos](key, tid);
  count++;
}



template<typename K, typename Comp>
bool BTree<K, Comp>::insert(K key, uint64_t tid) {
  Node* curNode = root;
  while(!curNode->isLeaf()){
    if(!curNode->isFull()){
      uint64_t pos = curNode->findKeyPos(key);
      curNode = (pos == curNode->count) ? curNode->upper : curNode->keyChildPIDPairs[pos]->second;
    } else {
      std::cout << "Inner Node is full -> split" << std::endl;
      throw "TODO Implement Split";
    }
  }
  Leaf* leaf = reinterpret_cast<Leaf*>(curNode);
  if(!leaf->isFull()){
    leaf->insertIntoLeaf(key,tid);
  } else {
    std::cout << "Leaf Node is full -> split" << std::endl;
    throw "TODO Implement Split";
  }

  return true;
}

template<typename K, typename Comp>
bool BTree<K, Comp>::erase(K key) {
  throw "To be implemented";
}

template<typename K, typename Comp>
std::experimental::optional<uint64_t> BTree<K, Comp>::lookup(K key) {
  throw "To be implemented";
}

template<typename K, typename Comp>
std::vector<uint64_t>::iterator BTree<K, Comp>::lookupRange(K key1, K key2) {
  throw "To be implemented";
}
}

