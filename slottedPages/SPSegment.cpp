#include "slottedPages/SPSegment.h"
#include "buffer/bufferManager.h"
#include <stdexcept>
#include <cstring>

namespace dbImpl {

  //describes one slot on a page
  union SlotDescriptor {
    struct InplaceDescriptor {
      uint8_t redirectionMarker;
      uint8_t migratedPageMarker;
      uint32_t offset : 24;
      uint32_t len : 24;

      InplaceDescriptor(uint32_t offset, uint32_t len)
        : redirectionMarker(0), migratedPageMarker(0),
          offset(offset), len(len) {}
    } inplace;

    uint64_t redirectionTid;

    bool isRedirection() const {  
      return inplace.redirectionMarker == 0xff;
    }

    bool isMigratedSlot() const {
      return inplace.migratedPageMarker;
    }
  };

  //describes a slotted page
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

  union TupleIdentifier {
    struct {
      uint64_t pageId : 56;
      uint8_t slotNr : 8;
    } interpreted;
    uint64_t opaque;
  };

  SPSegment::SPSegment(BufferManager& bm, uint32_t segmentId)
    : bm(bm), segmentId(segmentId) {}

  uint64_t SPSegment::insert(const Record& r) {
    //get a page for this record
    BufferFrame& frame = getFrameForSize(r.getLen() + sizeof(SlotDescriptor));
    SPHeader* header = reinterpret_cast<SPHeader*>(frame.getData());
    //compactify if necessary
    if(header->dataStart - sizeof(SPHeader) - header->nrAllocatedSlots * sizeof(SlotDescriptor) < r.getLen() + sizeof(SlotDescriptor)) {
      compactify(frame);
    }
    SlotDescriptor* slots = reinterpret_cast<SlotDescriptor*> (header + 1);
    //allocate a slot descriptor
    //try to find one which was already allocated
    while(header->firstFreeSlot < header->nrAllocatedSlots && slots[header->firstFreeSlot].inplace.offset != 0) {
      header->firstFreeSlot++;
    }
    if(header->firstFreeSlot >= header->nrAllocatedSlots) {
      //no free slot found? => allocate a new one.
      header->nrAllocatedSlots++;
    }
    header->firstFreeSlot++;
    uint8_t slotNr = header->firstFreeSlot;
    SlotDescriptor* slotDescriptor = &slots[slotNr];
    //update the header & fill in the slot's descriptor
    header->dataStart -= r.getLen();
    header->freeSpace -= r.getLen();
    slotDescriptor->inplace = SlotDescriptor::InplaceDescriptor(header->dataStart, r.getLen());
    //save record
    memcpy(reinterpret_cast<uint8_t*>(frame.getData()) + slotDescriptor->inplace.offset, r.getData(), r.getLen());
    bm.unfixPage(frame, true);
    //build and return the TID
    TupleIdentifier tid;
    tid.interpreted.pageId = frame.pageId;
    tid.interpreted.slotNr = slotNr;
    return tid.opaque;
  }

  /*
  void SPSegment::remove(uint64_t tid) {
    //load page
    //if is redirection
    //  remove(redirected)
    //update slot descriptor to offset = 0; len = 0
  }
  */

  Record SPSegment::lookup(uint64_t opaqueTid) {
    TupleIdentifier tid;
    tid.opaque = opaqueTid;
    if(bm.getSegmentIdForPageId(tid.interpreted.pageId) != segmentId) {
      throw std::runtime_error("given TID does not belong to the segment managed by this SPSegment instance");
    }
    //load page
    BufferFrame& frame = bm.fixPage(tid.interpreted.pageId, false);
    uint32_t slotNr = tid.interpreted.slotNr;
    //obtain pointers to header & slot descriptors
    SPHeader* header = reinterpret_cast<SPHeader*>(frame.getData());
    //check slot number
    if(slotNr > header->nrAllocatedSlots) {
      bm.unfixPage(frame, false);
      throw std::runtime_error("slot id above number of allocated slots on page");
    }
    SlotDescriptor* slots = reinterpret_cast<SlotDescriptor*> (header + 1);
    SlotDescriptor slot = slots[slotNr];
    //redirected?
    if(slot.isRedirection()) {
      //follow redirection
      bm.unfixPage(frame, false);
      return lookup(slot.redirectionTid);
    } else {
      //valid slot?
      if(slot.inplace.offset == 0) {
        bm.unfixPage(frame, false);
        throw std::runtime_error("slot id above number of allocated slots on page");
      }
      //load data into record
      char* data = reinterpret_cast<char*> (frame.getData()) + slot.inplace.offset;
      Record r(slot.inplace.len, data);
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
  BufferFrame& SPSegment::getFrameForSize(uint64_t size) {
    if(size > BufferManager::pageSize - sizeof(SPHeader)) {
      throw std::runtime_error("Record larger than maximum supported record size.");
    }
    //search through all pages of this segement
    for(uint32_t i = 0; i < std::numeric_limits<uint32_t>::max(); i++) {
      uint64_t pageId = bm.buildPageId(segmentId, i);
      BufferFrame* frame = &bm.fixPage(pageId, false);
      SPHeader* header = reinterpret_cast<SPHeader*> (frame->getData());
      //does the data fit into this page?
      //dataStart should never be 0. If it is 0, this means that the page was not initialized so far.
      bool mightFit = header->dataStart == 0 || header->freeSpace >= size;
      bm.unfixPage(*frame, false);
      if(mightFit) {
        //try to lock it with write permissions
        frame = &bm.fixPage(pageId, true);
        header = reinterpret_cast<SPHeader*> (frame->getData());
        //recheck the conditions (might have changed while no lock was held)
        if(header->dataStart == 0) {
          //uninitialized page => initialize it
          *header = SPHeader();
          return *frame;
        }
        if(header->freeSpace >= size) {
          return *frame;
        }
      }
    }
    throw std::runtime_error("No page within SPSegment found");
  }

  void SPSegment::compactify(BufferFrame&) {
    throw std::runtime_error("compatify not implemented");
  }

}
