#ifndef _SP_SEGMENT_HPP_
#define _SP_SEGMENT_HPP_

#include <cstdint>
#include <iterator>

#include "slottedPages/record.h"

namespace dbImpl {

  class BufferManager;
  class BufferFrame;

  /**
   * Accesses a segment using the slotted pages mechanism.
   *
   * The segment ID this accessor class refers to must be specified in the constructor.
   * The actual records are adressed by using the tuple identifier (TID).
   * Note, that each TID is only unique within a given segment.
   *
   * Do NOT try to use multiple SPSegment instances in order to access the same segment.
   */
  class SPSegment {
    public:
      // Assignment Operator: deleted
      SPSegment& operator=(SPSegment& rhs) = delete;
      // Copy Constructor: deleted
      SPSegment(SPSegment& t) = delete;
      /*
       * actual constructor
       * parameters:
       *  * bm: the buffer manager to be used
       *  * segmentId: which segment should be accessed using this SPSegment instance
       */
      SPSegment(BufferManager& bm, uint32_t segmentId);

      // inserts a new record and returns the tuple identifier of the stored data
      uint64_t insert(const Record& r);

      /*
       * Deletes the tuple identified by the given TID.
       * If the TID is not in use currently, this function is a NOP.
       */
      void remove(uint64_t tid);

      /*
       * loads the contents stored under the given tuple ID.
       * throws if this TID is not in use.
       */
      Record lookup(uint64_t tid);

      /*
       * updates the contents stored under the given TID.
       * If the TID is currently not in use, it will not be created but
       * instead an exception will be thrown.
       */
      void update(uint64_t tid, const Record& r);

      //SlotIterator must be predeclared
      class SlotIterator;

      /*
       * returns an iterator pointing to the first slot.
       * The returned iterator iterates over all slots stored in this segment.
       */
      SlotIterator begin();

      /*
       * returns an iterator pointing one slot past the last slot
       */
      SlotIterator end();


      class SlotIterator : public std::iterator<std::input_iterator_tag, Record> {
        private:
          friend SlotIterator SPSegment::begin();
          friend SlotIterator SPSegment::end();
          SlotIterator(BufferFrame* frame, uint8_t slotNr, BufferManager* bm)
            : bm(bm), currentFrame(frame), slotNr(slotNr) {}
        public: 
          //desctructor, copy constructor, move constructor and assignment operators must
          //be provided for proper buffer managment
          ~SlotIterator();
          SlotIterator(const SlotIterator& other);
          SlotIterator(SlotIterator&& other);
          SlotIterator& operator=(const SlotIterator& other);
          SlotIterator& operator=(SlotIterator&& other);
          //comparision operators
          bool operator==(const SlotIterator& rhs) const;
          bool operator!=(const SlotIterator& rhs) const {return !operator==(rhs);}
          //post- and pre-increment
          SlotIterator& operator++();
          SlotIterator operator++(int) {SlotIterator tmp(*this); operator++(); return tmp;}
          //dereference
          Record operator*();
        private:
          BufferManager* bm;
          BufferFrame* currentFrame;
          //must be bigger than the type of the slotNr used in order
          //to handle overflows approriately
          uint16_t slotNr;
          /*
           * increments the slotNr and goes to the next page if necessary.
           * Might set currentFrame to nullptr, if there is no next page.
           */
          void incrementSlotNr();
          /*
           * increments this iterator until it points to a valid slot
           * again. If it is already pointing to a valid slot, this method does
           * nothing.
           */
          void normalize();
      };

    protected:
      /**
       * compactifies the page
       */
      void compactify(BufferFrame& frame);

      /**
       * returns the first page which is able to store the required amount of data.
       * The returned BufferFrame is already locked exclusively.
       * If no such page exists currently, a new page will be allocated.
       */
      BufferFrame& getFrameForSize(uint64_t size);

      /**
       * Helper function used by insert and update.
       * Saves data into a the specified slot into the frame.
       * If neccessary the page is compactified first.
       * The Record MUST fit onto the page. There are no additional checks for its size!
       * The BufferFrame must be locked exclusively. This function does not unlock the page.
       */
      void emplaceContents(BufferFrame& frame, uint8_t slotNr, const Record& r);

      BufferManager& bm;
      uint32_t segmentId;
  };

}

#endif
