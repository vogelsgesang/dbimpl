#ifndef _SP_SEGMENT_HPP_
#define _SP_SEGMENT_HPP_

#include <cstdint>

#include "slottedPages/Record.hpp"

namespace dbImpl {

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
       *  * segmentId: which segment should be accessed using this SPSegment instance
       */
      SPSegment(uint32_t segmentId);

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
       * If the TID is currently not in use, it will not be 
       */
      void update(uint64_t tid, const Record& r);

      /**
       * compactifies the page containing the given TID (without taking redirections into account)
       *
       * It does
       *   * try to remove indirections pointing to the current page
       *   * store all slots sequentially and make free space between them available
       */
      void compacitifyPage(uint64_t tid);

      /**
       * compactifies all pages
       */
      void compacitify();
  }

}

#endif