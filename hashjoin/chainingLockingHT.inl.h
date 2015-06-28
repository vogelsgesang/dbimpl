#include <cstdint>
#include "tbb/spin_mutex.h"

#include "hashjoin/chainedRangeIterator.h"

template<typename Hasher>
class ChainingLockingHT {
  typedef typename tbb::spin_mutex BucketLock;
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

    hashTable = new Entry*[hashTableSize];
    memset(hashTable, 0, hashTableSize * sizeof(Entry*));
    entries = new Entry[size];
    bucketLocks = new BucketLock[hashTableSize];

    nextFreeEntry = entries - 1;
  }

  // Destructor
  ~ChainingLockingHT() {
    delete[] hashTable;
    delete[] bucketLocks;
    delete[] entries;
  }

  inline void insert(uint64_t key, uint64_t value) {
    Entry* newEntry = ++nextFreeEntry; //TODO atomic?
    newEntry->key = key;
    newEntry->value = value;

    //Determine and lock bucket
    uint64_t bucketNr = hashKey(key) & keyBits;
    bucketLocks[bucketNr].lock();

    //Redirect Bucket pointer to new Entry (in all cases --> branchfreeness)
    Entry* firstBucketEntry = hashTable[bucketNr];
    newEntry->next = firstBucketEntry;
    hashTable[bucketNr] = newEntry;

    bucketLocks[bucketNr].unlock();
  }

  Range lookup(uint64_t key) const {
    uint64_t hash = hashKey(key);
    uint64_t bucketNr = hash & keyBits;
    Entry* chainStart = hashTable[bucketNr];
    return Range(chainStart, key);
  }

private:

  Entry* entries;
  Entry* nextFreeEntry;

  uint64_t hashTableSize;
  uint64_t keyBits;

  Hasher hashKey;

  Entry** hashTable;
  BucketLock* bucketLocks;

};

