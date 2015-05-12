#ifndef _FREE_SPACE_INVENTORY_HPP_
#define _FREE_SPACE_INVENTORY_HPP_

#include <cstdint>

namespace dbImpl {

  class BufferManager;

  /*
   * straight forward implementation of a FreeSpaceInventory
   */
  class FreeSpaceInventory {
    public:
      // Assignment Operator: deleted
      FreeSpaceInventory& operator=(FreeSpaceInventory& rhs) = delete;
      // Copy Constructor: deleted
      FreeSpaceInventory(FreeSpaceInventory& t) = delete;
      /*
       * actual constructor
       * parameters:
       *  * bm: the buffer manager to be used
       *  * segmentId: which segment should be managed by this FreeSpaceInventory instance
       */
      FreeSpaceInventory(BufferManager& bm, uint32_t segmentId);

      //initializes all pages to be completely empty
      void initializeInventory();

      /*
       * finds the first part of the segment which is able to store at least the
       * given number of bytes.
       */
      uint32_t findFreeSpace(uint32_t requiredBytes);

      //updates the free space indicated in the free space directory
      void updateFreeSpace(uint32_t partId, uint32_t freeBytes);

      static const uint16_t inventoryPageCount; //how many pages are used by the inventory

    protected:
      /*
       * compresses the number of free/required bytes given into
       * a 4bit integer. This mapping is done using a combination
       * of linear and logarithmic mapping of the given byte number
       * relative to the pageSize.
       */
      uint8_t compressSpaceIndicator(uint32_t byteCount);

      BufferManager& bm;
      uint32_t segmentId;
  };

}

#endif
