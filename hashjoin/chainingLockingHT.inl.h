#include <cstdint>
#include "tbb/spin_mutex.h"

#include "hashjoin/chainedRangeIterator.h"

template<typename Hasher>
class ChainingLockingHT {
public:

  // Constructor
  ChainingLockingHT(uint64_t size) {
    hashTableSize = size * 2; //load factor 0.5
    //round hashTableSize to power of two
    hashTableSize--;
    hashTableSize |= hashTableSize >> 1;
    hashTableSize |= hashTableSize >> 2;
    hashTableSize |= hashTableSize >> 4;
    hashTableSize |= hashTableSize >> 8;
    hashTableSize |= hashTableSize >> 16;
    hashTableSize |= hashTableSize >> 32;
    hashTableSize++;
    keyBits = hashTableSize - 1;

    hashTable = new Bucket[hashTableSize];
    entries = new Entry[size];

    nextFreeEntry = entries;
  }

  // Destructor
  ~ChainingLockingHT() {
    delete[] hashTable;
  }

  inline void insert(uint64_t key, uint64_t value) {
    Entry* newEntry = nextFreeEntry++;
    newEntry->key = key;
    newEntry->value = value;

    uint64_t bucketNr = hashKey(key) & keyBits;
    hashTable[bucketNr].lock();
    Entry* firstBucketEntry = hashTable[bucketNr].firstEntry;
    newEntry->next = firstBucketEntry;
    hashTable[bucketNr].firstEntry = newEntry;
    hashTable[bucketNr].unlock();

  }


  Range lookup(uint64_t key) const {
    uint64_t hash = hashKey(key);
    uint64_t bucketNr = hash & keyBits;
    Entry* chainStart = hashTable[bucketNr].firstEntry;
    return Range(chainStart, key);
  }

private:
  struct Bucket {
    Entry* firstEntry;
    tbb::spin_mutex mutex;
    void lock(){
      mutex.lock();
    }
    void unlock(){
      mutex.unlock();
    }


  };

  Entry* entries;
  Entry* nextFreeEntry;

  uint64_t hashTableSize;
  uint64_t keyBits;

  Hasher hashKey;

  Bucket* hashTable;
};

