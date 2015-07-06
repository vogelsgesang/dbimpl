#include <cmath>
#include <utility>
#include <tbb/tbb.h>
#include "tbb/blocked_range.h"
#include <vector>
#include <cstdint>
#include <thread>

#include "partitionWorker.h"

template<typename Hasher>
class PartitionHT {

private:

  HashKeyEntry* entries;

  HashTableBucket* hashTable;
  uint64_t hashTableSize;
  Hasher hashKey;
  uint64_t size; //input
  unsigned threadCount;
  unsigned B;
  PartitionWorker<Hasher>* workers;

  uint64_t partSize;
  uint64_t* partitionUpperbounds;

public:
  HashKeyEntry* rwithHash;
  // Constructor
  PartitionHT(uint64_t size, unsigned threads) :
      size(size) {

    //Calculate log2 of Threadcount(=Number of Partitions)
    B = 0;
    unsigned i = threads;
    while (i >>= 1) {
      ++B;
    }

    //round hashTableSize to power of two
    threadCount = std::pow(2, B);

    partSize = size / threadCount;

    hashTable = new HashTableBucket[hashTableSize];
    memset(hashTable, 0, hashTableSize * sizeof(HashTableBucket));

    //allocate memory and prepare it
    entries = new HashKeyEntry[size];
    rwithHash = new HashKeyEntry[size];

    workers = new PartitionWorker<Hasher> [threadCount];
    partitionUpperbounds = new uint64_t[threadCount];

  }

// Destructor

  ~PartitionHT() {

    //delete[] workers;
    //delete[] rwithHash;
    // delete[] hashTable;
  }

  void partition() {

    //calculate partitionInfos
    std::vector < std::thread > threads;
    threads.reserve(threadCount);

    uint64_t curElements = 0;
    //build histograms
    for (unsigned i = 0; i < threadCount; ++i) {
      workers[i] = PartitionWorker < Hasher
          > (rwithHash, B, i, threadCount, entries, curElements, (
              i != threadCount - 1 ? curElements + partSize : size), hashTable);
      curElements += partSize;
      threads.emplace_back(
          std::thread(&PartitionWorker < Hasher > ::buildHistorgam,
              workers[i]));
    }

    for (auto& t : threads) {
      t.join();
    }

    //calculate prefix sums
    for (unsigned i = 0; i < threadCount; ++i) {
      for (unsigned j = 1; j < threadCount; ++j) {
        workers[j].prefixSums[i] = workers[j - 1].prefixSums[i]
            + workers[j - 1].histogram[i];
      }
      if (i > 0)
        workers[0].prefixSums[i + 1] = workers[threadCount - 1].prefixSums[i]
            + workers[threadCount - 1].histogram[i];
      partitionUpperbounds[i] = workers[threadCount - 1].prefixSums[i]
          + workers[threadCount - 1].histogram[i];
    }

    //start partitioning
    for (unsigned i = 0; i < threadCount; ++i) {
      threads[i] = std::thread(&PartitionWorker < Hasher > ::buildPartition,
          workers[i]);
    }

    for (auto& t : threads) {
      t.join();
    }

    delete[] rwithHash;

    //build input
    //create partial Hashtables
    for (unsigned i = 0; i < threadCount; ++i) {
      uint64_t start = (i == 0 ? 0 : partitionUpperbounds[i - 1]);
      uint64_t end = partitionUpperbounds[i];
      uint64_t sizePartTable = end - start;

      sizePartTable *= 2;
      //round hashTableSize to power of two
      sizePartTable--;
      sizePartTable |= sizePartTable >> 1;
      sizePartTable |= sizePartTable >> 2;
      sizePartTable |= sizePartTable >> 4;
      sizePartTable |= sizePartTable >> 8;
      sizePartTable |= sizePartTable >> 16;
      sizePartTable |= sizePartTable >> 32;

      sizePartTable++;
      hashTable[i].keyBits = sizePartTable - 1;
      hashTable[i].hashTable = new HashTableEntry[sizePartTable];

      threads[i] = std::thread(&PartitionWorker < Hasher > ::buildInput,
          workers[i], start, end);
    }
    for (auto& t : threads) {
      t.join();
    }

  }

  inline void insert(uint64_t key, uint64_t value) {

  }
  unsigned lookup(uint64_t key) {

    //determine partition
    uint64_t hashValue = hashKey(key);
    HashTableBucket* partition = &hashTable[hashValue >> (64 - B)];
    HashTableEntry* partHashTable = partition->hashTable;
    unsigned hits = 0;
    if (partHashTable != nullptr) {

      uint64_t bucketNr = (hashValue & partition->keyBits);

      while (partHashTable[bucketNr].occupied) {
        hits += partHashTable[bucketNr].key == key;
        bucketNr = ((bucketNr + 1) & partition->keyBits);

      }

    }
    return hits;

  }
};

