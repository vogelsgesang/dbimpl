#include "slottedPages/SPSegment.h"
#include "buffer/bufferManager.h"
#include <stdexcept>

namespace dbImpl {

  struct SPHeader {
    uint16_t dataStart; //the offset at which data starts
    uint16_t freeSpace; //number of bytes which would be available 
    uint8_t nrAllocatedSlots; //the number of allocated slot descriptors
    uint8_t firstFreeSlot; //the index of the first free slot

    SPHeader()
      : dataStart(BufferManager::pageSize),
        freeSpace(BufferManager::pageSize - sizeof(SPHeader)),
        nrAllocatedSlots(0),
        firstFreeSlot(0) {}
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

  /*
  uint64_t SPSegment::insert(const Record& r) {
    //find page with free space
    //if data does not fit directly
    //  compactify
    //save data
    //update free space
  }

  void SPSegment::remove(uint64_t tid) {
    //load page
    //if is redirection
    //  remove(redirected)
    //update slot descriptor to offset = 0; len = 0
  }
  */

  Record SPSegment::lookup(uint64_t tid) {
    uint32_t pageId = getPageIdFromTID(tid);
    if(bm.getSegmentIdForPageId(pageId) != segmentId) {
      throw std::runtime_error("given TID does not belong to the segment managed by this SPSegment instance");
    }
    //load page
    BufferFrame& frame = bm.fixPage(pageId, false);
    uint32_t slotNr = getSlotNrFromTID(tid);
    //obtain pointers to header & slot descriptors
    SPHeader* header = reinterpret_cast<SPHeader*>(frame.getData());
    uint64_t* slotDescriptors = reinterpret_cast<uint64_t*> (
        reinterpret_cast<char*> (frame.getData()) + sizeof(SPHeader));
    //check slot number
    if(slotNr > header->nrAllocatedSlots) {
      bm.unfixPage(frame, false);
      throw std::runtime_error("slot id above number of allocated slots on page");
    }
    //get the slot's descriptor
    SlotDescriptor descriptor(slotDescriptors[slotNr]);
    //redirected?
    if(descriptor.isRedirection()) {
      //follow redirection
      bm.unfixPage(frame, false);
      return lookup(static_cast<uint64_t>(descriptor));
    } else {
      //load data into record
      char* data = reinterpret_cast<char*> (frame.getData())
        + sizeof(SPHeader) + descriptor.getOffset();
      Record r(descriptor.getLength(), data);
      bm.unfixPage(frame, false);
      return r;
    }
  }
/*
  void SPSegment::update(uint64_t tid, const Record& r) {
    //if fits onto own page
    //  if is redirected
    //    free guest slot
    //  if must be compactified
    //    compactify
    //  store data on current page
    //else
    //  if is not redirected
    //    store using insert(...)
    //    store redirection
    //  else
    //    if updated value fits on guest page
    //      update data on guest page
    //    else
    //      free guest slot
    //      store using insert(...)
    //      update redirection
  }
*/

}
