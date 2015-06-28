#ifndef _CHAINED_RANGEITERATOR_H_
#define _CHAINED_RANGEITERATOR_H_

#include <cstdint>

struct Entry {
  uint64_t key;
  uint64_t value;
  Entry* next;
};

//this iterator is used to iterate over the results of a lookup
class RangeIterator {
  public:
    RangeIterator(Entry* entry, uint64_t key) :
      entry(entry), key(key) {
      }

    uint64_t operator*() const {
      return entry->value;
    }

    RangeIterator& operator++() {
      do {
        entry = entry->next;
      } while (entry != nullptr && entry->key != key);
      return *this;
    }

    RangeIterator operator++(int) {
      RangeIterator r(*this);
      operator++();
      return r;
    }

    bool operator==(const RangeIterator& rhs) const {
      return entry == rhs.entry && key == rhs.key;
    }

    bool operator!=(const RangeIterator& rhs) const {
      return !operator==(rhs);
    }

  private:
    Entry* entry;
    uint64_t key;
};

//represents the results of a lookup
class Range {
  public:
    Range(Entry* chainStart, uint64_t key)
    : firstElement(chainStart), key(key) {
      while(firstElement != nullptr && firstElement->key != key) {
        firstElement = firstElement->next;
      }
    }

    RangeIterator begin() {
      return RangeIterator(firstElement, key);
    }

    RangeIterator end() {
      return RangeIterator(nullptr, key);
    }

  private:
    Entry* firstElement;
    uint64_t key;
};

#endif
