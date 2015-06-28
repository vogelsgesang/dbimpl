#ifndef _CHAINED_RANGEITERATOR_BUCKETS_H_
#define _CHAINED_RANGEITERATOR_BUCKETS_H_

#include <cstdint>

const uint_8 BUCKETSIZE = 4;

struct Entry {
  uint64_t key;
  uint64_t value;
};

//Fill Bucket reversely
struct Bucket {
  uint_8 firstFreeEntry = BUCKETSIZE + 1;
  Entry* entries = new Entry[BUCKETSIZE];
  Bucket* next = nullptr;
  inline bool isFull() {
    return firstFreeEntry; //== 0 if bucket is full
  }
  inline uint8_t keyInBucket(uint64_t key) {
    for (uint8_t i = BUCKETSIZE - 1; i >= 0; --i) {
      if (key == entries[i].key) {
        return i;
      }
    }
    return -1;
  }
};

//this iterator is used to iterate over the results of a lookup
class RangeIteratorBuckets {
public:
  RangeIteratorBuckets(Bucket* bucket, uint64_t key) :
      bucket(bucket), key(key) {
  }

  uint64_t operator*() const {
    return bucket->entries[bucket->keyInBucket(key)]->value;
  }

  RangeIteratorBuckets& operator++() {
    do {
      bucket = bucket->next;
    } while (bucket != nullptr && keyInBucket(key) == -1);
    return *this;
  }

  RangeIteratorBuckets operator++(int) {
    RangeIteratorBuckets r(*this);
    operator++();
    return r;
  }

  bool operator==(const RangeIteratorBuckets& rhs) const {
    return bucket == rhs.bucket && key == rhs.key;
  }

  bool operator!=(const RangeIteratorBuckets& rhs) const {
    return !operator==(rhs);
  }

private:
  Bucket* bucket;
  uint64_t key;
};

//represents the results of a lookup
class RangeBuckets {
public:
  RangeBuckets(Bucket* chainStart, uint64_t key) :
      firstElement(chainStart), key(key) {
    while (firstElement->next != nullptr && firstElement->keyInBucket(key) == -1) {
      firstElement = firstElement->next;
    }
  }

  RangeIteratorBuckets begin() {
    return RangeIteratorBuckets(firstElement, key);
  }

  RangeIteratorBuckets end() {
    return RangeIteratorBuckets(nullptr, key);
  }

private:
  Bucket* firstElement;
  uint64_t key;
};

#endif
