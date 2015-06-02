#include "bTree.h"
#include <deque>

namespace dbImpl {

template<typename K, typename Comp>
BTree<K, Comp>::BTree(BufferManager& bm, uint64_t _maxNodeSize)
  : bufferManager(bm)
  , elements(0)
{
  this->maxNodeSize = std::min(_maxNodeSize,
      ((BufferManager::pageSize - sizeof(Node)) / sizeof(std::pair<K, uint64_t>)));
  BufferFrame& bf = bufferManager.fixPage(nextFreePage++, true);
  rootPID = bf.pageId;
  Node* root = reinterpret_cast<Node*>(bf.getData());
  *root = Node(true);
  bufferManager.unfixPage(bf, true);
}

template<typename K, typename Comp>
inline bool BTree<K, Comp>::isEqual(K key1, K key2) {
  return !smaller(key1, key2) && !smaller(key2, key1);
}

template<typename K, typename Comp>
inline bool BTree<K, Comp>::Node::isLeaf() {
  return leafMarker;
}

template<typename K, typename Comp>
inline uint64_t BTree<K, Comp>::Node::findKeyPos(const K key) {
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
bool BTree<K, Comp>::Node::insertKey(K key, uint64_t tid) {
  uint64_t pos = findKeyPos(key);

  //Check if key is not already stored in Tree. Count != pos avoids random matches with old memory data.
  if (count != pos && BTree<K, Comp>::isEqual(keyValuePairs[pos].first, key)) {
    // Key is already in tree
    return false;
  }
  memmove(&keyValuePairs[pos + 1], &keyValuePairs[pos],
      (count - pos) * sizeof(std::pair<K, uint64_t>)); // will probably only work in combination with Buffermanager
  std::pair < K, uint64_t > keyTIDpair(key, tid);
  keyValuePairs[pos] = keyTIDpair;
  count++;
  return true;
}

template<typename K, typename Comp>
void BTree<K, Comp>::Node::insertInnerKey(K key, uint64_t leftChildPID, uint64_t rightChildPID) {
  //Insert Key with pointer to left child
  uint64_t pos = findKeyPos(key);
  memmove(&keyValuePairs[pos + 1], &keyValuePairs[pos],
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
bool BTree<K, Comp>::Node::deleteKey(K key) {
  uint64_t pos = findKeyPos(key);
  bool deleted = false;
  if (pos < count && isEqual(key, keyValuePairs[pos].first)) {
    deleted = true;
    count--;
    memmove(&keyValuePairs[pos], &keyValuePairs[pos + 1],
        (count - pos) * sizeof(std::pair<K, uint64_t>));
  }
  return deleted;
}

template<typename K, typename Comp>
K BTree<K, Comp>::Node::split(uint64_t ownPID, BufferFrame* newFrame,
    BufferFrame* parent) {
  //Get new node
  Node* newNode = reinterpret_cast<Node*>(newFrame->getData());
  *newNode = Node(this->isLeaf());
  //split current Node
  uint64_t mid = count / 2;
  memmove(&newNode->keyValuePairs[0], &keyValuePairs[mid],
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
  (reinterpret_cast<Node*>(parent->getData()))
    ->insertInnerKey(splitKey, ownPID, newFrame->pageId);
  return splitKey;
}

template<typename K, typename Comp>
BufferFrame* BTree<K, Comp>::createNewRoot() {
  BufferFrame* newFrame = &bufferManager.fixPage(nextFreePage++, true);
  rootPID = newFrame->pageId;
  Node* newRoot = reinterpret_cast<Node*>(newFrame->getData());
  *newRoot = Node(false);
  return newFrame;
}

template<typename K, typename Comp>
BufferFrame* BTree<K, Comp>::traverseToLeaf(K key, bool exclusiveLeaf) {
  //latch the root
  BufferFrame* curFrame = &bufferManager.fixPage(rootPID, exclusiveLeaf);
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
            curNode->next : curNode->keyValuePairs[pos].second;
    //latch the next level
    curFrame = &bufferManager.fixPage(nextPID, exclusiveLeaf);
    curNode = reinterpret_cast<Node*>(curFrame->getData());
  }
  if (parFrame != NULL) {
    bufferManager.unfixPage(*parFrame, false);
  }
  return curFrame;
}

//latch the root, latch the first level, release the root, latch the second level etc,...
template<typename K, typename Comp>
bool BTree<K, Comp>::insert(K key, uint64_t tid) {
  //latch the root
  BufferFrame* curFrame = &bufferManager.fixPage(rootPID, true);
  Node* curNode = reinterpret_cast<Node*>(curFrame->getData());
  BufferFrame* parFrame = NULL;
  while (!curNode->isLeaf()) {
    if (curNode->count >= maxNodeSize) {
      // --> split to safe inner pages
      if (parFrame == NULL) {
        //Need to create a new root (parent) first
        parFrame = createNewRoot();
      }
      BufferFrame* newFrame = &bufferManager.fixPage(nextFreePage++, true);
      K splitKey = curNode->split(curFrame->pageId, newFrame, parFrame);

      //determine correct node and release the other one
      if (smaller(key, splitKey)) {
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

    //latch the next level
    uint64_t pos = curNode->findKeyPos(key);
    uint64_t nextPID =
        (pos == curNode->count) ?
            curNode->next : curNode->keyValuePairs[pos].second;
    curFrame = &bufferManager.fixPage(nextPID, true);
    curNode = reinterpret_cast<Node*>(curFrame->getData());
  }

  Node* leaf = reinterpret_cast<Node*>(curNode);
  if (leaf->count >= maxNodeSize) {
    if (parFrame == NULL) {
      parFrame = createNewRoot();
    }

    BufferFrame* newFrame = &bufferManager.fixPage(nextFreePage++, true);
    K splitKey = leaf->split(curFrame->pageId, newFrame, parFrame);
    if (smaller(key, splitKey)) {
      bufferManager.unfixPage(*newFrame, true);
    } else {
      leaf = reinterpret_cast<Node*>(newFrame->getData());
      bufferManager.unfixPage(*curFrame, true);
      curFrame = newFrame;
    }
  }
  if (parFrame != NULL) {
    bufferManager.unfixPage(*parFrame, true); //TODO: only mark dirty when parent was actually updated
  }

  bool insertSuccessful = leaf->insertKey(key, tid);
  if (insertSuccessful) {
    elements++;
  }
  bufferManager.unfixPage(*curFrame, true);
  return insertSuccessful;
}

template<typename K, typename Comp>
bool BTree<K, Comp>::erase(K key) {
  BufferFrame* leafFrame = traverseToLeaf(key, true);
  Node* leaf = reinterpret_cast<Node*>(leafFrame->getData());
  bool deleted = leaf->deleteKey(key);
  bufferManager.unfixPage(*leafFrame, true);
  if (deleted) {
    elements--; //update size of BTree
  }
  return deleted;
}

template<typename K, typename Comp>
optional<uint64_t> BTree<K, Comp>::lookup(K key) {
  BufferFrame* leafFrame = traverseToLeaf(key, false);
  Node* leaf = reinterpret_cast<Node*>(leafFrame->getData());
  uint64_t pos = leaf->findKeyPos(key);
  uint64_t tid = std::numeric_limits<uint64_t>::max();

  bool found = false;
  if (pos < leaf->count && isEqual(key, leaf->keyValuePairs[pos].first)) {
    found = true;
    tid = leaf->keyValuePairs[pos].second;
  }
  bufferManager.unfixPage(*leafFrame, false);
  return optional<uint64_t> { found, tid };
}

//TODO return iterator of vector
template<typename K, typename Comp>
std::vector<uint64_t> BTree<K, Comp>::lookupRange(K key1, K key2) {
  std::vector < uint64_t > resultSet;

  //Get Leaf of lower key
  K leftK, rightK;
  if (smaller(key1, key2)) {
    leftK = key1;
    rightK = key2;
  } else {
    leftK = key2;
    rightK = key1;
  }

  BufferFrame* leafFrame = traverseToLeaf(leftK, false);
  Node* leaf = reinterpret_cast<Node*>(leafFrame->getData());

  uint64_t pos = leaf->findKeyPos(leftK);
  if (pos >= leaf->count || smaller(leaf->keyValuePairs[pos].first, leftK)) {
    //No matching key was found
    bufferManager.unfixPage(*leafFrame, false);
    return resultSet;
  }

  while (true) {
    while (pos < leaf->count) {
      if (smaller(rightK, leaf->keyValuePairs[pos].first)) {
        bufferManager.unfixPage(*leafFrame, false);
        return resultSet;
      }
      resultSet.push_back(leaf->keyValuePairs[pos].second);
      pos++;
    }
    if (leaf->next == std::numeric_limits<uint64_t>::max()) {
      //There is no next leaf --> return
      bufferManager.unfixPage(*leafFrame, false);
      return resultSet;
    } else {
      //Continue in next Leaf. Get it and unfix current Leaf
      uint64_t nextLeafPID = leaf->next;
      bufferManager.unfixPage(*leafFrame, false);
      leafFrame = &bufferManager.fixPage(nextLeafPID, false);
      leaf = reinterpret_cast<Node*>(leafFrame->getData());
      pos = 0;
    }
  }

  bufferManager.unfixPage(*leafFrame, false);
  return resultSet;
}

template<typename K, typename Comp>
void BTree<K, Comp>::exportAsDot(std::ostream& out) {
  out << "digraph bTree {\n";
  std::deque<uint64_t> pidQueue;
  pidQueue.push_back(rootPID);
  while(!pidQueue.empty()) {
    uint64_t pid = pidQueue.back();
    pidQueue.pop_back();
    //fix page
    BufferFrame& bf = bufferManager.fixPage(pid, false);
    Node *currNode = reinterpret_cast<Node*>(bf.getData());
    if(currNode->isLeaf()) {
      out << "node" << pid << " [shape=record, label=\"<count> " << (currNode->count) << " | ";
      for(uint64_t i = 0; i < currNode->count; i++) {
        out << "{ <key" << i << "> " << currNode->keyValuePairs[i].first  << " | "
            << "<tid"   << i << "> " << currNode->keyValuePairs[i].second << " } | ";
      }
      out << "next\"];" << std::endl;
      if(currNode->next != std::numeric_limits<uint64_t>::max()) {
        out << "node" << pid << ":next -> node" << currNode->next << ";" <<std::endl;
      }
    } else {
      out << "node" << pid << " [shape=record, label=\"<count> " << (currNode->count) << " | ";
      for(uint64_t i = 0; i < currNode->count; i++) {
        out << "<ptr"   << i << "> * | "
            << "<key" << i << "> " << currNode->keyValuePairs[i].first  << " | ";
      }
      out << "<ptr" << currNode->count << "> *\"];" << std::endl;
      for(uint64_t i = 0; i < currNode->count; i++) {
        out << "node" << pid << ":ptr" << i << " -> node" << currNode->keyValuePairs[i].second << ";" <<std::endl;
        pidQueue.push_back(currNode->keyValuePairs[i].second);
      }
      out << "node" << pid << ":ptr" << currNode->count << " -> node" << currNode->next << ";" <<std::endl;
      pidQueue.push_back(currNode->next);
    }
    //unfix page
    bufferManager.unfixPage(bf, false);
  }
  out << "}";
}

}
