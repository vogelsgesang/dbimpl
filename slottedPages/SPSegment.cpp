#include "slottedPages/SPSegment.h"
#include "buffer/bufferManager.h"

namespace dbImpl {

  struct SPHeader {
    uint16_t dataStart; //the offset at which data starts
    uint16_t freeSpace; //number of bytes which would be available 
    uint8_t nrAllocatedSlots; //the number of allocated slot descriptors
    uint8_t firstFreeSlot; //the index of the first free slot
  };

  //represents one slot descriptor
  class SlotDescriptor {
    public:
      explicit SlotDescriptor(uint64_t descriptor) : descriptor(descriptor) {}

      operator uint64_t() const {
        return descriptor;
      }

      bool isRedirection() const {  
        uint64_t tidInvalidMarker = 0xffl << 56;
        return (descriptor & tidInvalidMarker) == tidInvalidMarker;
      }

      uint32_t isMigratedSlot() const {
        uint64_t migratedPageMarker = 0xffl << 48;
        return (descriptor & migratedPageMarker) == migratedPageMarker;
      }

      uint32_t getOffset() const {
        return (descriptor >> 24) & 0xfff;
      }

      uint32_t getLength() const {
        return descriptor & 0xfff;
      }

    private:
      uint64_t descriptor;
  };

  uint64_t getPageIdFromTID(uint64_t tid) {
    return tid >> 8;
  }

  uint64_t getSlotNrFromTID(uint64_t tid) {
    return tid & 0xff;
  }

  SPSegment::SPSegment(BufferManager& bm, uint32_t segmentId)
    : bm(bm), segmentId(segmentId) {}

}
