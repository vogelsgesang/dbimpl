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

    //this class is just a marker type
    class EndIterator {};

    //this iterator is used to iterate over the results of a lookup
    class RangeIterator {
      public:
        RangeIterator(Entry* entries, uint64_t keyBits, uint64_t bucketNr, uint64_t key)
          : entries(entries), keyBits(keyBits), bucketNr(bucketNr), key(key) {}

        uint64_t operator*() const {
          return entries[bucketNr].value;
        }

        RangeIterator& operator++() {
          do {
            bucketNr = (bucketNr + 1) & keyBits;
          } while(entries[bucketNr].occupied && entries[bucketNr].key != key);
          return *this;
        }

        RangeIterator operator++(int) { RangeIterator r(*this); operator++(); return r; };

        bool operator== (const EndIterator&) const {
          return !entries[bucketNr].occupied;
        }

        bool operator!= (const EndIterator& rhs) const {
          return !operator==(rhs);
        }

      private:
        Entry* entries;
        uint64_t keyBits;
        uint64_t bucketNr;
        uint64_t key;
    };

    //represents the results of a lookup
    class Range {
      public:
        Range(Entry* entries, uint64_t keyBits, uint64_t startBucketNr, uint64_t key)
          : entries(entries), keyBits(keyBits), startBucketNr(startBucketNr), key(key) {}

        RangeIterator begin() {
          return RangeIterator(entries, keyBits, startBucketNr, key);
        }

        EndIterator end() {
          return EndIterator();
        }

      private:
        Entry* entries;
        uint64_t keyBits;
        uint64_t startBucketNr;
        uint64_t key;
    };

    inline Range lookup(uint64_t key) {
      uint64_t hash = hashKey(key);
      uint64_t bucketNr = hash & keyBits;
      return Range(entries, keyBits, bucketNr, key);
    }
};
