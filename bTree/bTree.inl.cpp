namespace dbImpl {

template<typename K, typename Comp>
BTree<K, Comp>::BTree(BufferManager& bm) :
    bufferManager(bm) {
  BufferFrame& bf = bufferManager.fixPage(0, true);
  Leaf* root = reinterpret_cast<Leaf*>(bf.getData());
  rootPID = bf.pageId;
  *root = Leaf();
  bufferManager.unfixPage(bf, true);
}

template<typename K, typename Comp>
inline bool BTree<K, Comp>::Node::isFull() {
  return BufferManager::pageSize - sizeof(Node)
      - count * sizeof(std::pair<K, uint64_t>) < sizeof(std::pair<K, uint64_t>);
}
template<typename K, typename Comp>
inline bool BTree<K, Comp>::Leaf::isFull() {
  return BufferManager::pageSize - sizeof(Leaf)
      - count * sizeof(std::pair<K, uint64_t>) < sizeof(std::pair<K, uint64_t>);
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
    if (BTree::smaller(keyChildPIDPairs[mid].first, key)) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }
  return left;

}
template<typename K, typename Comp>
bool BTree<K, Comp>::Leaf::deleteKeyInLeaf(K key) {
  uint64_t pos = findKeyPos(key);
  bool deleted = false;
   if (pos < count && !smaller(key, keyTIDPairs[pos].first)
       && !smaller(keyTIDPairs[pos].first, key)) {
     deleted = true;
     count--;
     memmove(&keyTIDPairs[pos], &keyTIDPairs[pos+1], (count-pos)*sizeof(std::pair<K,uint64_t>));
   }
   return deleted;
}

template<typename K, typename Comp>
inline uint64_t BTree<K, Comp>::Leaf::findKeyPos(const K key) {
  uint64_t mid;
  uint64_t left = 0;
  uint64_t right = count;

  while (right != left) {
    mid = left + ((right - left) / 2);
    if (BTree::smaller(keyTIDPairs[mid].first, key)) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }
  return left;
}

template<typename K, typename Comp>
void BTree<K, Comp>::Leaf::insertIntoLeaf(K key, uint64_t tid) {
  uint64_t pos = findKeyPos(key);
  if (keyTIDPairs[pos].first == key && count != pos) {
    std::cout << "Key " << key << " is already in tree. TID:" << keyTIDPairs[pos].second << std::endl;
    return;
  }
  memmove(&keyTIDPairs[pos + 1], &keyTIDPairs[pos],
      (count - pos) * sizeof(std::pair<K, uint64_t>)); // will probably only work in combination with Buffermanager
  std::pair < K, uint64_t > keyTIDpair(key, tid);
  keyTIDPairs[pos] = keyTIDpair;
  count++;
}
//latch the root, latch the first level, release the root, latch the second level etc,...
template<typename K, typename Comp>
bool BTree<K, Comp>::insert(K key, uint64_t tid) {
  //latch the root
  BufferFrame* curFrame = &bufferManager.fixPage(rootPID, true);
  Node* curNode = reinterpret_cast<Node*>(curFrame->getData());
  BufferFrame* parFrame = NULL;

  while (!curNode->isLeaf()) {
    if (curNode->isFull()) {
      std::cout << "Inner Node is full -> split" << std::endl;
      throw "TODO Implement Split";
    }
    //release the parent node
    if (parFrame != NULL) {
      bufferManager.unfixPage(*parFrame, true); //TODO only set true when parent is really dirty?
    }
    parFrame = curFrame;
    uint64_t pos = curNode->findKeyPos(key);
    uint64_t nextPID =
        (pos == curNode->count) ?
            curNode->upper : curNode->keyChildPIDPairs[pos].second;
    //latch the next level
    curFrame = &bufferManager.fixPage(nextPID, true);
    curNode = reinterpret_cast<Node*>(curFrame->getData());

  }
  Leaf* leaf = reinterpret_cast<Leaf*>(curNode);
  if (!leaf->isFull()) {
    leaf->insertIntoLeaf(key, tid);
    bufferManager.unfixPage(*curFrame, true); //TODO only set true when parent is really dirty?

  } else {
    std::cout << "Leaf Node is full -> split" << std::endl;
    throw "TODO Implement Split";
  }

  if (parFrame != NULL) {
    bufferManager.unfixPage(*parFrame, true);
  }

  return true;
}

template<typename K, typename Comp>
bool BTree<K, Comp>::erase(K key) {
  //latch the root
  BufferFrame* curFrame = &bufferManager.fixPage(rootPID, true);
  Node* curNode = reinterpret_cast<Node*>(curFrame->getData());
  BufferFrame* parFrame = NULL;

  while (!curNode->isLeaf()) {
    if (parFrame != NULL) {
      bufferManager.unfixPage(*parFrame, false);
    }
    parFrame = curFrame;
    uint64_t pos = curNode->findKeyPos(key);
    uint64_t nextPID =
        (pos == curNode->count) ?
            curNode->upper : curNode->keyChildPIDPairs[pos].second;
    //latch the next level
    curFrame = &bufferManager.fixPage(nextPID, true);
    curNode = reinterpret_cast<Node*>(curFrame->getData());
  }

  Leaf* leaf = reinterpret_cast<Leaf*>(curNode);
  bool deleted = leaf->deleteKeyInLeaf(key);
  if (parFrame != NULL) {
    bufferManager.unfixPage(*parFrame, false);
  }
  bufferManager.unfixPage(*curFrame, true);
  return deleted;

}

template<typename K, typename Comp>
optional<uint64_t> BTree<K, Comp>::lookup(K key) {
  //latch the root
  BufferFrame* curFrame = &bufferManager.fixPage(rootPID, false);
  Node* curNode = reinterpret_cast<Node*>(curFrame->getData());
  BufferFrame* parFrame = NULL;
  while (!curNode->isLeaf()) {
    //unlatch parent
    if (parFrame != NULL) {
      bufferManager.unfixPage(*parFrame, false);
    }
    parFrame = curFrame;
    uint64_t pos = curNode->findKeyPos(key);
    uint64_t nextPID =
        (pos == curNode->count) ?
            curNode->upper : curNode->keyChildPIDPairs[pos].second;
    //latch the next level
    curFrame = &bufferManager.fixPage(nextPID, false);
    curNode = reinterpret_cast<Node*>(curFrame->getData());
  }
  Leaf* leaf = reinterpret_cast<Leaf*>(curNode);
  uint64_t pos = leaf->findKeyPos(key);
  uint64_t tid;

  bool found = false;
  if (pos < leaf->count && !smaller(key, leaf->keyTIDPairs[pos].first)
      && !smaller(leaf->keyTIDPairs[pos].first, key)) {
    found = true;
    tid = leaf->keyTIDPairs[pos].second;
  }
  if (parFrame != NULL) {
    bufferManager.unfixPage(*parFrame, false);
  }
  bufferManager.unfixPage(*curFrame, false);
  return optional<uint64_t> { found, tid };
}

template<typename K, typename Comp>
std::vector<uint64_t>::iterator BTree<K, Comp>::lookupRange(K key1, K key2) {
  throw "To be implemented";
}
}

