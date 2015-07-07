#include <cstdint>
#include <cstring>
#include <atomic>
#include <utility>

template<typename Hasher>
class ChainingHT {
  public:
    struct Entry {
      uint64_t key;
      uint64_t value;
      Entry* next;
    };

  private:
    std::atomic<Entry*>* hashTable;
    uint64_t hashTableSize;
    uint64_t keyBits;
    Hasher hashKey;

  public:
    // Constructor
    ChainingHT(uint64_t size, Hasher hasher = Hasher())
      : hashKey(hasher)
    {
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
      hashTable = new std::atomic<Entry*>[hashTableSize];
      memset(hashTable, 0, hashTableSize*sizeof(std::atomic<Entry*>));
    }

    // Destructor
    ~ChainingHT() {
      delete[] hashTable;
    }

    inline void insert(Entry* entry) {
      //allocate a new element for the chain
      uint64_t hash = hashKey(entry->key);
      uint64_t bucketNr = hash & keyBits;
      //insert element into chain
      entry->next = hashTable[bucketNr].exchange(entry);
    }

    uint64_t lookup(uint64_t key) const {
      uint64_t hash = hashKey(key);
      uint64_t bucketNr = hash & keyBits;
      Entry* currentEntry = hashTable[bucketNr];
      uint64_t cnt = 0;
      while(currentEntry != nullptr) {
        cnt += currentEntry->key == key;
        currentEntry = currentEntry->next;
      }
      return cnt;
    }
};
