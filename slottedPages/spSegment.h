#ifndef _SP_SEGMENT_HPP_
#define _SP_SEGMENT_HPP_

#include <cstdint>

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

      BufferManager& bm;
      uint32_t segmentId;
  };

}

#endif
