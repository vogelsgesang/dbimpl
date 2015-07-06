#ifndef _PARTITION_WORKER_H_
#define _PARTITION_WORKER_H_

#include <cstdint>

struct HashKeyEntry {
  uint64_t key;
  uint64_t value;
  uint64_t hash;
};

struct HashTableEntry {
  bool occupied;
  uint64_t key;
  uint64_t value;

};

struct HashTableBucket {
  uint64_t keyBits;
  HashTableEntry* hashTable;
};

template<typename Hasher>
class PartitionWorker {

  Hasher hashKey;
public:
  HashKeyEntry* R;
  unsigned B; //bits per partition
  unsigned workerId;

  unsigned numPartitions;

  uint64_t* histogram;
  uint64_t* prefixSums;

  HashKeyEntry* entries;
  uint64_t startRange;
  uint64_t endRange;

  HashTableBucket* hashTable;

  PartitionWorker(HashKeyEntry* R, unsigned B, unsigned workerId,
      unsigned numPartitions, HashKeyEntry* entries, uint64_t startRange,
      uint64_t endRange, HashTableBucket* hashTable) :
      R(R), B(B), workerId(workerId), numPartitions(numPartitions), entries(
          entries), startRange(startRange), endRange(endRange), hashTable(
          hashTable) {
    histogram = new uint64_t[numPartitions];
    memset(histogram, 0, numPartitions * sizeof(uint64_t));

    prefixSums = new uint64_t[numPartitions];
    if (workerId == 0)
      memset(prefixSums, 0, numPartitions * sizeof(uint64_t));

  }
  PartitionWorker() {

  }
  ~PartitionWorker() {
  }

  void buildHistorgam() {
    for (uint64_t i = startRange; i < endRange; ++i) {
      ++histogram[R[i].hash >> (64 - B)];
    }
  }

  void buildPartition() {
    for (uint64_t i = startRange; i < endRange; ++i) {
      memcpy(&entries[prefixSums[R[i].hash >> (64 - B)]++], &R[i],
          sizeof(HashKeyEntry));
    }

  }

  void buildInput(uint64_t start, uint64_t end) {
    HashTableEntry* partHashtable = hashTable[workerId].hashTable;
    uint64_t keyBits = hashTable[workerId].keyBits;
    for (uint64_t i = start; i < end; ++i) {
      HashKeyEntry* e = &entries[i];
      uint64_t bucketNr = (e->hash & keyBits);

      //occupy a place in the hashmap
      while (partHashtable[bucketNr].occupied) {
        bucketNr = ((bucketNr + 1) & keyBits);
      }

      //initialize the occupied place
      partHashtable[bucketNr].occupied = true;
      partHashtable[bucketNr].key = e->key;
      partHashtable[bucketNr].value = e->value;

    }

  }

};
#endif

