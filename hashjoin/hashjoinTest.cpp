#include <iostream>
#include <cstdlib>
#include <atomic>
#include <tbb/tbb.h>
#include <unordered_map>
#include <utility>
#include "hashjoin/chainingHT.inl.h"
#include "hashjoin/linearProbingHT.inl.h"
#include "hashjoin/chainingLockingHT.inl.h"
#include "partitionHash.inl.h"

using namespace tbb;
using namespace std;

struct MumurHasher {
  inline uint64_t operator()(uint64_t k) const {
    // MurmurHash64A
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;
    uint64_t h = 0x8445d61a4e774912 ^ (8 * m);
    k *= m;
    k ^= k >> r;
    k *= m;
    h ^= k;
    h *= m;
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return h | (1ull << 63);
  }
};

// Probe hash table and count number of hits
template<typename HtImplementation>
uint64_t testProbe(HtImplementation& ht, uint64_t sizeS, uint64_t* S) {
  std::atomic < uint64_t > hitCounter(0);
  parallel_for(blocked_range < size_t > (0, sizeS),
      [&](const blocked_range<size_t>& range) {
        uint64_t localHitCounter=0;
        for (size_t i=range.begin(); i!=range.end(); ++i) {
          auto lookupRange = ht.lookup(S[i]);
          for (auto it=lookupRange.begin(); it != lookupRange.end(); ++it) {
#ifdef DEBUG
      if(*it != S[i]) {
        cout << "Got entry for key " << *it << " instead of for " << S[i] << std::endl;
      }
#endif
      localHitCounter++;
    }
  }
  hitCounter+=localHitCounter;
});
  return hitCounter;
}

template<typename HtImplementation>
void testHashTable(uint64_t sizeR, uint64_t sizeS, uint64_t* R, uint64_t* S) {
  // Build hash table (multi threaded)
  tick_count buildTS = tick_count::now();

  HtImplementation ht(sizeR);

  parallel_for(blocked_range < size_t > (0, sizeR),
      [&](const blocked_range<size_t>& range) {
        for (size_t i=range.begin(); i!=range.end(); ++i) {
          ht.insert(R[i], R[i]);
        }
      });
  tick_count probeTS = tick_count::now();
  cout << "build:" << (sizeR / 1e6) / (probeTS - buildTS).seconds() << "MT/s ";

  // Probe hash table and count number of hits
  uint64_t hitCounter = testProbe(ht, sizeS, S);
  tick_count stopTS = tick_count::now();
  cout << "probe: " << (sizeS / 1e6) / (stopTS - probeTS).seconds() << "MT/s "
      << "total: " << ((sizeR + sizeS) / 1e6) / (stopTS - buildTS).seconds()
      << "MT/s " << "count: " << hitCounter << endl;
}

int main(int argc, char** argv) {
  if (argc < 4) {
    cout << "usage: " << argv[0] << " <sizeR> <sizeS> <threadCount>" << endl;
    return 1;
  }
  uint64_t sizeR = atoi(argv[1]);
  uint64_t sizeS = atoi(argv[2]);
  unsigned threadCount = atoi(argv[3]);

  task_scheduler_init init(threadCount);

  // Init build-side relation R with random data
  uint64_t* R = static_cast<uint64_t*>(malloc(sizeR * sizeof(uint64_t)));
  parallel_for(blocked_range < size_t > (0, sizeR),
      [&](const blocked_range<size_t>& range) {
        unsigned int seed=range.begin();
        for (size_t i=range.begin(); i!=range.end(); ++i)
        R[i]=rand_r(&seed)%sizeR;
      });

  // Init probe-side relation S with random data
  uint64_t* S = static_cast<uint64_t*>(malloc(sizeS * sizeof(uint64_t)));
  parallel_for(blocked_range < size_t > (0, sizeS),
      [&](const blocked_range<size_t>& range) {
        unsigned int seed=range.begin();
        for (size_t i=range.begin(); i!=range.end(); ++i)
        S[i]=rand_r(&seed)%sizeR;
      });

//  // STL
//  std::cout << "STL                  ";
//  {
//    // Build hash table (single threaded)
//    tick_count buildTS = tick_count::now();
//    unordered_multimap < uint64_t, uint64_t > ht(sizeR);
//    for (uint64_t i = 0; i < sizeR; i++)
//      ht.emplace(R[i], 0);
//    tick_count probeTS = tick_count::now();
//    cout << "build:" << (sizeR / 1e6) / (probeTS - buildTS).seconds()
//        << "MT/s ";
//
//    // Probe hash table and count number of hits
//    std::atomic < uint64_t > hitCounter;
//    hitCounter = 0;
//    parallel_for(blocked_range < size_t > (0, sizeS),
//        [&](const blocked_range<size_t>& range) {
//          uint64_t localHitCounter=0;
//          for (size_t i=range.begin(); i!=range.end(); ++i) {
//            auto range=ht.equal_range(S[i]);
//            for (unordered_multimap<uint64_t,uint64_t>::iterator it=range.first; it!=range.second; ++it) {
//              localHitCounter++;
//            }
//          }
//          hitCounter+=localHitCounter;
//        });
//    tick_count stopTS = tick_count::now();
//    cout << "probe: " << (sizeS / 1e6) / (stopTS - probeTS).seconds() << "MT/s "
//        << "total: " << ((sizeR + sizeS) / 1e6) / (stopTS - buildTS).seconds()
//        << "MT/s " << "count: " << hitCounter << endl;
//  }

  std::cout << "Lockfree partitioning   ";
  {
    uint64_t hits = 0;
    MumurHasher hashKey;
    tick_count buildTS = tick_count::now();
    PartitionHT<MumurHasher> ht(sizeR, threadCount);
    parallel_for(blocked_range < size_t > (0, sizeR),
        [&](const blocked_range<size_t>& range) {
          for (size_t i=range.begin(); i!=range.end(); ++i) {
            ht.rwithHash[i].hash = hashKey(R[i]);
            ht.rwithHash[i].key = R[i];
            ht.rwithHash[i].value = R[i];
          }
        });

    ht.partition();
    tick_count probeTS = tick_count::now();
    cout << "build:" << (sizeR / 1e6) / (probeTS - buildTS).seconds()
        << "MT/s ";

    std::atomic < uint64_t > hitCounter(0);
    parallel_for(blocked_range < size_t > (0, sizeS),
        [&](const blocked_range<size_t>& range) {
          uint64_t localHitCounter=0;
          for (size_t i=range.begin(); i!=range.end(); ++i) {
            localHitCounter += ht.lookup(S[i]);
          }
          hits += localHitCounter;
        });

    tick_count stopTS = tick_count::now();
    cout << "probe: " << (sizeS / 1e6) / (stopTS - probeTS).seconds() << "MT/s "
        << "total: " << ((sizeR + sizeS) / 1e6) / (stopTS - buildTS).seconds()
        << "MT/s " << "count: " << hits << endl;

  }

  //own implementations
//  std::cout << "ChainingLockingHT    ";
//  testHashTable<ChainingLockingHT<MumurHasher>>(sizeR, sizeS, R, S);
//
//  std::cout << "ChainingHT           ";
//  {
//    // Build hash table (multi threaded)
//    tick_count buildTS = tick_count::now();
//    ChainingHT<MumurHasher> ht(sizeR);
//    parallel_for(blocked_range < size_t > (0, sizeR),
//        [&](const blocked_range<size_t>& range) {
//          size_t i = range.begin();
//          ht.insertChunk(range.end() - range.begin(), [&]() {
//                size_t j = i;
//                i++;
//                return std::make_pair(R[j], R[j]);
//              });
//        });
//    tick_count probeTS = tick_count::now();
//    cout << "build:" << (sizeR / 1e6) / (probeTS - buildTS).seconds()
//        << "MT/s ";
//
//    // Probe hash table and count number of hits
//    uint64_t hitCounter = testProbe(ht, sizeS, S);
//    tick_count stopTS = tick_count::now();
//    cout << "probe: " << (sizeS / 1e6) / (stopTS - probeTS).seconds() << "MT/s "
//        << "total: " << ((sizeR + sizeS) / 1e6) / (stopTS - buildTS).seconds()
//        << "MT/s " << "count: " << hitCounter << endl;
//  }
//
//  std::cout << "LinearProbingHT      ";
//  testHashTable<LinearProbingHT<MumurHasher>>(sizeR, sizeS, R, S);
//
  return 0;
}
