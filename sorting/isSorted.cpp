#include "sorting/isSorted.h"
#include "utils/sequenceReader.h"

namespace dbImpl {

  bool isSorted(int fdTest, uint64_t size, uint64_t memSize) {
    SequenceReader<uint64_t> reader(fdTest, 0, size, memSize/sizeof(uint64_t));

    uint64_t lastValue = 0;

    while(!reader.empty()) {
      uint64_t value = reader.consumeElement();
      if(value < lastValue) return false;
      lastValue = value;
    }
    return true;
  }
}
