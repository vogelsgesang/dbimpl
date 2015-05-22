template<typename K, typename Comp>
BTree<K, Comp>::BTree()
:
	
{};

template<typename K, typename Comp>
inline uint64_t BTree<K,Comp>::calculateMaxChildren(bool isLeaf){
    uint64_t nodeSize = 3*sizeof(uint64_t);
    if(isLeaf) nodeSize += sizeof(Leaf*);
    return (BufferManager::pageSize - nodeSize)/(sizeof(std::pair<K,uint64_t>);
};

template<typename K, typename Comp>
  inline bool BTree<K,Comp>::Node::isFull(){
    return count >= calculateMaxChildren(false);
  };
template<typename K, typename Comp>
  inline bool BTree<K,Comp>::Node::isLeaf(){
    return upper == ~0;
  };

template<typename K, typename Comp>
  inline uint64_t BTree<K,Comp>::Node::findKeyPos(const K key){
    uint64_t left = 0;
    uint64_t right = count;
    uint64_t mid;
    while(right != left){
      mid = left + ((right-left)/2);
      if(smaller(key,keyChildPIDPairs[mid]->first)){
        right = mid;
      } else if(smaller(keyChildPIDPairs[mid]->first,key)) {
        left = mid+1;
      } else {
        return mid+1;
      }
    }
    return right;

  };
template<typename K, typename Comp>
  inline uint64_t BTree<K,Comp>::Leaf::findKeyPos(const K key){
    uint64_t notFound = -1;
    uint64_t mid;
    uint64_t left = 0;
    uint64_t right = count;
 
    while (right > left)   
    {
      mid = left + ((right-left)/2);
      if(smaller(key,keyTIDPairs[mid]->first)){
        right = mid-1;
      } else if(smaller(keyTIDPairs[mid]->first,key)) {
        left = mid+1;
      } else {
        return mid;
      }
    }
  return notFound;
  };