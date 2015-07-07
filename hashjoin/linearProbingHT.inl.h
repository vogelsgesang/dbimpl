#include <cstdint>
#include <cstring>
#include <atomic>

template<typename Hasher>
class LinearProbingHT {
  private:
    // Entry
    struct Entry {
      uint64_t key;
      uint64_t value;
      std::atomic<bool> occupied;
    };

    Entry* entries;
    uint64_t hashTableSize;
    uint64_t keyBits;
    Hasher hashKey;

  public:

    // Constructor
    LinearProbingHT(uint64_t size) {
      hashTableSize = size*2; //load factor 0.5
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
      //allocate memory and prepare it
      entries = new Entry[hashTableSize];
      memset(entries, 0, hashTableSize*sizeof(Entry));
    }

    // Destructor
    ~LinearProbingHT() {
      delete[] entries;
    }

    inline void insert(uint64_t key, uint64_t value) {
      uint64_t hash = hashKey(key);
      uint64_t bucketNr = hash & keyBits;
      //required for compare_exchange_weak
      bool falseValue = false;
      //occupy a place in the hashmap
      do {
        while(entries[bucketNr].occupied) {
          bucketNr = (bucketNr + 1) & keyBits;
        }
      } while(!entries[bucketNr].occupied.compare_exchange_weak(falseValue, true, std::memory_order_seq_cst));
      //initialize the occupied place
      entries[bucketNr].key = key;
      entries[bucketNr].value = value;
    }

    inline uint64_t lookup(uint64_t key) {
      uint64_t hash = hashKey(key);
      uint64_t bucketNr = hash & keyBits;
      uint64_t cnt = 0;
      while(entries[bucketNr].occupied) {
        cnt += entries[bucketNr].key == key;
        bucketNr = (bucketNr + 1) & keyBits;
      }
      return cnt;
    }
};
