#include <cstdint>
#include <cstring>
#include <atomic>

#include <iostream>

template<typename Hasher>
class ChainingHT {
  private:
     // Chained tuple entry
     struct Entry {
        uint64_t key;
        uint64_t value;
        Entry* next;
     };

     Entry* firstEntry;
     std::atomic<Entry*> nextFreeEntry;
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
       hashTable--;
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
       firstEntry = new Entry[size];
       nextFreeEntry = firstEntry;
     }

     // Destructor
     ~ChainingHT() {
       delete[] firstEntry;
       delete[] hashTable;
     }

     inline void insert(uint64_t key, uint64_t value) {
       //allocate a new element for the chain
       Entry* entry = nextFreeEntry.fetch_add(1);
       uint64_t hash = hashKey(key);
       uint64_t bucketNr = hash & keyBits;
       entry->key = key;
       entry->value = value;
       //insert element into chain
       entry->next = hashTable[bucketNr].exchange(entry);
     }

     //this iterator is used to iterate over the results of a lookup
     class RangeIterator {
       public:
         RangeIterator(Entry* entry, uint64_t key) : entry(entry), key(key) {}

         uint64_t operator*() const {
           return entry->value;
         }

         RangeIterator& operator++() {
            do {
             entry = entry->next;
            } while(entry != nullptr && entry->key != key);
            return *this;
         }

         RangeIterator operator++(int) { RangeIterator r(*this); operator++(); return r; };

         bool operator== (const RangeIterator& rhs) const {
           return entry == rhs.entry && key == rhs.key;
         }

         bool operator!= (const RangeIterator& rhs) const {
           return !operator==(rhs);
         }

       private:
         Entry* entry;
         uint64_t key;
     };

     //represents the results of a lookup
     class Range {
       public:
         Range(Entry* chainStart, uint64_t key) : chainStart(chainStart), key(key) {}

         RangeIterator begin() {
           return RangeIterator(chainStart, key);
         }

         RangeIterator end() {
           return RangeIterator(nullptr, key);
         }

       private:
         Entry* chainStart;
         uint64_t key;
     };

     Range lookup(uint64_t key) const {
       uint64_t hash = hashKey(key);
       uint64_t bucketNr = hash & keyBits;
       Entry* chainStart = hashTable[bucketNr];
       return Range(chainStart, key);
     }
};
