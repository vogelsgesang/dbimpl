#include <cstdint>
#include "tbb/spin_mutex.h"

#include "hashjoin/chainedRangeIteratorBuckets.h"

template<typename Hasher>
class ChainingLockingHTBuckets {
  typedef typename tbb::spin_mutex BucketLock;
public:

  // Constructor
  ChainingLockingHTBuckets(uint64_t size) {
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
    bucketLocks = new BucketLock[hashTableSize];

  }

  // Destructor
  ~ChainingLockingHTBuckets() {
    delete[] hashTable;
    delete[] bucketLocks;
  }

  inline void insert(uint64_t key, uint64_t value) {
    Entry newEntry;
    newEntry->key = key;
    newEntry->value = value;

    //Determine and lock bucket
    uint64_t bucketNr = hashKey(key) & keyBits;
    bucketLocks[bucketNr].lock();

    //Redirect Bucket pointer to new Entry (in all cases --> branchfreeness)
    Bucket* bucket = &hashTable[bucketNr];

    if (bucket->isFull()) {
      Bucket overflowBucket;
      overflowBucket.entries[--overflowBucket.firstFreeEntry] = newEntry;
      std::swap(overflowBucket, *bucket);
      bucket->next = &overflowBucket;
    } else {
      bucket->entries[--bucket->firstFreeEntry] = newEntry;
    }
    bucketLocks[bucketNr].unlock();
  }

  RangeBuckets lookup(uint64_t key) const {
    uint64_t hash = hashKey(key);
    uint64_t bucketNr = hash & keyBits;
    Bucket* chainStart = &hashTable[bucketNr];
    return RangeBuckets(chainStart, key);
  }

private:
  uint64_t hashTableSize;
  uint64_t keyBits;

  Hasher hashKey;

  Bucket* hashTable;
  BucketLock* bucketLocks;

};

