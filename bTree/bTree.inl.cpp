namespace dbImpl {

template<typename K, typename Comp>
BTree<K, Comp>::BTree(BufferManager& bm) :
    bufferManager(bm), elements(0) {
  BufferFrame& bf = bufferManager.fixPage(nextFreePage++, true);
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
bool BTree<K, Comp>::Leaf::deleteKey(K key) {
  uint64_t pos = findKeyPos(key);
  bool deleted = false;
  if (pos < count && !smaller(key, keyTIDPairs[pos].first)
      && !smaller(keyTIDPairs[pos].first, key)) {
    deleted = true;
    count--;
    memmove(&keyTIDPairs[pos], &keyTIDPairs[pos + 1],
        (count - pos) * sizeof(std::pair<K, uint64_t>));
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
inline bool BTree<K,Comp>::isEqual(K key1, K key2){
  return !(smaller(key1,key2) && !smaller(key2,key1));

}
template<typename K, typename Comp>
bool BTree<K, Comp>::Leaf::insertKey(K key, uint64_t tid) {
  uint64_t pos = findKeyPos(key);
  if (BTree<K,Comp>::isEqual(keyTIDPairs[pos].first,key) && count != pos) {
    std::cout << "Key is already in tree. TID:"
        << keyTIDPairs[pos].second << std::endl;
    return false;
  }
  memmove(&keyTIDPairs[pos + 1], &keyTIDPairs[pos],
      (count - pos) * sizeof(std::pair<K, uint64_t>)); // will probably only work in combination with Buffermanager
  std::pair < K, uint64_t > keyTIDpair(key, tid);
  keyTIDPairs[pos] = keyTIDpair;
  count++;
  return true;
}
template<typename K, typename Comp>
inline K BTree<K, Comp>::Node::getMaxKey() {
  return keyChildPIDPairs[count - 1].first;
}
template<typename K, typename Comp>
inline K BTree<K, Comp>::Leaf::getMaxKey() {
  return keyTIDPairs[count - 1].first;
}

template<typename K, typename Comp>
void BTree<K, Comp>::Node::insertKey(K key, uint64_t leftChildPID,
    uint64_t rightChildPID) {

  //Insert Key with pointer to left child
  uint64_t pos = findKeyPos(key);

  memmove(&keyChildPIDPairs[pos + 1], &keyChildPIDPairs[pos],
      sizeof(std::pair<K, uint64_t>) * (count - pos));
  std::pair < K, uint64_t > keyPidPair(key, leftChildPID);
  keyChildPIDPairs[pos] = keyPidPair;

  //Update existing pointer to new (right) child
  if (pos == count) {
    upper = rightChildPID;
  } else {
    keyChildPIDPairs[pos + 1].second = rightChildPID;
  }
  count++;
}

template<typename K, typename Comp>
uint64_t BTree<K, Comp>::Node::split(uint64_t curPID, BufferFrame* newFrame,
    BufferFrame* parent, K key) {
  //Get new node
  Node* newNode = reinterpret_cast<Node*>(newFrame->getData());
  *newNode = Node();

  uint64_t mid = count/2;
  //split current Node
  memmove(&newNode->keyChildPIDPairs[0], &keyChildPIDPairs[mid],
      (count -mid) * sizeof(std::pair<K, uint64_t>));
  newNode->upper = upper;
  newNode->count = count - mid;
  count = mid;
  //biggest key of left node moves to parent
  K splitKey = getMaxKey();
  upper = keyChildPIDPairs[count - 1].second;

  (reinterpret_cast<Node*>(parent->getData()))->insertKey(splitKey, curPID, newFrame->pageId);
  //Determine sibling for further insert process
  return (BTree::smaller(key,splitKey) ? curPID : newFrame->pageId);
}


template<typename K, typename Comp>
BufferFrame* BTree<K, Comp>::Leaf::split(uint64_t curPID, BufferFrame* newFrame,
    BufferFrame* parent) {
  //Get new node
  Leaf* newLeaf = reinterpret_cast<Leaf*>(newFrame->getData());
  *newLeaf = Leaf();
  //split current Node
  uint64_t mid = count / 2;
  memmove(&newLeaf->keyTIDPairs[0], &keyTIDPairs[mid],
      (count - mid) * sizeof(std::pair<K, uint64_t>));

  newLeaf->next = next;
  newLeaf->count = count - mid;
  count = mid;
  next = newFrame->pageId;
  (reinterpret_cast<Node*>(parent->getData()))->insertKey(
      keyTIDPairs[mid - 1].first, curPID, newFrame->pageId);
  return newFrame;

}
template<typename K, typename Comp>
BufferFrame* BTree<K, Comp>::createNewRoot() {
  BufferFrame* newFrame = &bufferManager.fixPage(nextFreePage++, true);
  rootPID = newFrame->pageId;
  Node* newRoot = reinterpret_cast<Node*>(newFrame->getData());
  *newRoot = Node();
  return newFrame;

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
      //std::cout << "Inner Node is full -> split " << key  << std::endl;
      // --> safe inner pages

      if (parFrame == NULL) {
        //Need to create a new root (parent) first
        parFrame = createNewRoot();
      }
      BufferFrame* newFrame = &bufferManager.fixPage(nextFreePage++, true);
      //parFrame = &bufferManager.fixPage(parPID, true);
      uint64_t nextPID = curNode->split(curFrame->pageId, newFrame, parFrame, key);

      //determine correct node and release the other one
      if (nextPID == curFrame->pageId) {
        bufferManager.unfixPage(*newFrame, true);
      } else {
        curNode = reinterpret_cast<Node*>(newFrame->getData());
        bufferManager.unfixPage(*curFrame, true);
        curFrame = newFrame;
      }
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
  if (leaf->isFull()) {
    if (parFrame == NULL) {
      parFrame = createNewRoot();
    }

    BufferFrame* newFrame = &bufferManager.fixPage(nextFreePage++, true);
    leaf->split(curFrame->pageId, newFrame, parFrame);
    if (smaller(key, leaf->getMaxKey())) {
      bufferManager.unfixPage(*newFrame, true);
    } else {
      leaf = reinterpret_cast<Leaf*>(newFrame->getData());
      bufferManager.unfixPage(*curFrame, true);
      curFrame = newFrame;
    }

  }

  if(leaf->insertKey(key, tid)){
    elements++;
  }
  bufferManager.unfixPage(*curFrame, true);
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
  bool deleted = leaf->deleteKey(key);
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

