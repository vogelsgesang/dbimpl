namespace dbImpl {

template<typename K, typename Comp>
BTree<K, Comp>::BTree(BufferManager& bm)
:
  bufferManager(bm)
{
  BufferFrame& bf = bufferManager.fixPage(0, true);
  Leaf* root= reinterpret_cast<Leaf*>(bf.getData());
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
    if (BTree::smaller(key, keyChildPIDPairs[mid].first)) {
      right = mid;
    } else if (BTree::smaller(keyChildPIDPairs[mid].first, key)) {
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
    if (BTree::smaller(key, keyTIDPairs[mid].first)) {
      right = mid;
    } else if (BTree::smaller(keyTIDPairs[mid].first, key)) {
      left = mid + 1;
    } else {
      return mid + 1;
    }
  }
  return right;
}

template<typename K, typename Comp>
void BTree<K, Comp>::Leaf::insertIntoLeaf(K key, uint64_t tid) {
  uint64_t pos = findKeyPos(key);
  memmove(&keyTIDPairs[pos + 1], &keyTIDPairs[pos],
      (count - pos) * sizeof(std::pair<K, uint64_t>)); // will probably only work in combination with Buffermanager
  std::pair < K, uint64_t > keyTIDpair(key, tid);
  keyTIDPairs[pos] = keyTIDpair;
  count++;
}

template<typename K, typename Comp>
bool BTree<K, Comp>::insert(K key, uint64_t tid) {
  BufferFrame* bf = &bufferManager.fixPage(rootPID,true);
  Node* curNode = reinterpret_cast<Node*>(bf->getData());
  while (!curNode->isLeaf()) {
    if (!curNode->isFull()) {
      uint64_t pos = curNode->findKeyPos(key);
      curNode = reinterpret_cast<Node*>(
          (pos == curNode->count) ?
              curNode->upper : curNode->keyChildPIDPairs[pos].second);
    } else {
      std::cout << "Inner Node is full -> split" << std::endl;
      throw "TODO Implement Split";
    }
  }
  Leaf* leaf = reinterpret_cast<Leaf*>(curNode);
  if (!leaf->isFull()) {
    leaf->insertIntoLeaf(key, tid);
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

