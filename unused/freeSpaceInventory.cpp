#include "slottedPages/freeSpaceInventory.h"
#include "buffer/bufferManager.h"
#include <cstring>

namespace dbImpl {

  FreeSpaceInventory::FreeSpaceInventory(BufferManager& bm, uint32_t segmentId)
    : bm(bm), segmentId(segmentId) {}

  uint8_t FreeSpaceInventory::compressSpaceIndicator(uint32_t byteCount) {
    uint8_t relativeSize = byteCount * 255 / bm.pageSize;
    if(relativeSize >= 128) {
      //linear mapping for upper half
      return relativeSize >> 4;
    } else {
      //logarithmic mapping for range < 128
      uint8_t result = 0;
      while(relativeSize) {
        result++;
        relativeSize >>= 1;
      }
      return result;
    }
  }

  //the number of pages which are administered by each page
  //For simplicity, I assume that every page has an FSIHeader although this is not the case
  const uint16_t pagesAdministeredPerPage = BufferManager::pageSize * 2;

  //how many pages are needed to store information
  const uint16_t FreeSpaceInventory::inventoryPageCount = 2;

  void FreeSpaceInventory::initializeInventory() {
    for(uint16_t partNr = 0; partNr < inventoryPageCount; partNr++) {
      BufferFrame& frame = bm.fixPage(BufferManager::buildPageId(segmentId, partNr), true);
      std::memset(frame.getData(), 255, BufferManager::pageSize);
      bm.unfixPage(frame, true);
    }
  }

  uint32_t FreeSpaceInventory::findFreeSpace(uint32_t requiredBytes) {
    uint8_t minIndicator = compressSpaceIndicator(requiredBytes);
    minIndicator++;
    //for a big number of required bytes we are unable to find a page where this fits
    if(minIndicator > 0x0f) {
      throw std::runtime_error("unable to find a page for so many bytes");
    }
    //initialize page and offset
    uint32_t currentPosition = inventoryPageCount; //make sure that we never give away pages used by the inventory itself
    uint32_t currentPartNr = 0;
    uint32_t currentOffset = 0;
    //scan until we find a free slot
    BufferFrame *currentPage = &bm.fixPage(BufferManager::buildPageId(segmentId, currentPartNr), false);
    uint8_t *currentData = reinterpret_cast<uint8_t*>(currentPage->getData());
    while(true) {
      if(currentData[currentOffset] >> 4 >= minIndicator ) {
        bm.unfixPage(*currentPage, false);
        return currentPosition;
      }
      if((currentData[currentOffset] & 0x0f) >= minIndicator) {
        bm.unfixPage(*currentPage, false);
        return currentPosition;
      }
      currentOffset++;
      currentPosition++;
      if(currentOffset > pagesAdministeredPerPage/2) {
        bm.unfixPage(*currentPage, false);
        currentPartNr++;
        if(currentPartNr >= inventoryPageCount) {
          throw std::runtime_error("unable to find a page for so many bytes");
        }
        currentOffset = 0;
        currentPage = &bm.fixPage(BufferManager::buildPageId(segmentId, currentPartNr), false);
        currentData = reinterpret_cast<uint8_t*>(currentPage->getData());
      }
    }
  }

  void FreeSpaceInventory::updateFreeSpace(uint32_t partId, uint32_t freeBytes) {
    uint32_t fsiPartId = partId / pagesAdministeredPerPage;
    uint32_t fsiPageId = BufferManager::buildPageId(segmentId, fsiPartId);
    uint32_t offsetInPage = (partId % pagesAdministeredPerPage) / 2;
    //update the corresponding entry in the FSI table
    uint8_t minIndicator = compressSpaceIndicator(freeBytes);
    BufferFrame& fsiPage = bm.fixPage(fsiPageId, true);
    uint8_t *data = reinterpret_cast<uint8_t*>(fsiPage.getData());
    uint8_t entry = data[offsetInPage];
    if(partId % 2) {
      entry &= 0xf0;
      entry |= minIndicator;
    } else {
      entry &= 0x0f;
      entry |= minIndicator << 4;
    }
    bm.unfixPage(fsiPage, true);
  }

}
