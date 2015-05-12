#include <gtest/gtest.h>

#include "slottedPages/freeSpaceInventory.h"
#include "buffer/bufferManager.h"

using namespace dbImpl;

//this class makes some private functions
//public and therefore allows whitebox testing
class WhiteboxFreeSpaceInventory : public FreeSpaceInventory {
  public:
    using FreeSpaceInventory::FreeSpaceInventory; //inherit constructor
    using FreeSpaceInventory::compressSpaceIndicator; //expose compressSpaceIndicator
};

TEST(FreeSpaceInventory, spaceIndicatorIsMonotonic) {
  BufferManager bm(100);
  WhiteboxFreeSpaceInventory fsi(bm, 1);
  uint8_t lastResult = 0;
  for(uint32_t i = 0; i <= bm.pageSize; i++) {
    EXPECT_GE(fsi.compressSpaceIndicator(i), lastResult);
    lastResult = fsi.compressSpaceIndicator(i);
  }
}

TEST(FreeSpaceInventory, spaceIndicatorUsesOnly4Bits) {
  BufferManager bm(100);
  WhiteboxFreeSpaceInventory fsi(bm, 1);
  for(uint32_t i = 0; i <= bm.pageSize; i++) {
    EXPECT_EQ(0, fsi.compressSpaceIndicator(i) & 0xf0);
  }
}

TEST(FreeSpaceInventory, spaceIndicatorUsesCompleteRange) {
  BufferManager bm(100);
  WhiteboxFreeSpaceInventory fsi(bm, 1);
  EXPECT_EQ(0, fsi.compressSpaceIndicator(0));
  uint8_t lastResult = 0;
  for(uint32_t i = 0; i <= bm.pageSize; i++) {
    EXPECT_LE(fsi.compressSpaceIndicator(i) - lastResult, 1);
    lastResult = fsi.compressSpaceIndicator(i);
  }
  EXPECT_EQ(0x0f, fsi.compressSpaceIndicator(bm.pageSize));
}

TEST(FreeSpaceInventory, occupiesASaneNumberOfPages) {
  EXPECT_NE(0, FreeSpaceInventory::inventoryPageCount);
  EXPECT_LE(FreeSpaceInventory::inventoryPageCount, 4096);
}

TEST(FreeSpaceInventory, initallyReturnsTheFirstUsablePage) {
  BufferManager bm(100);
  FreeSpaceInventory fsi(bm, 1);
  fsi.initializeInventory();
  EXPECT_EQ(FreeSpaceInventory::inventoryPageCount, fsi.findFreeSpace(16));
}

TEST(FreeSpaceInventory, managesTheAmountOfFreeSpace) {
  BufferManager bm(100);
  FreeSpaceInventory fsi(bm, 1);
  fsi.initializeInventory();

  int firstPage = fsi.findFreeSpace(16);
  fsi.updateFreeSpace(firstPage, BufferManager::pageSize/4);
  EXPECT_EQ(firstPage, fsi.findFreeSpace(BufferManager::pageSize/8));
  EXPECT_NE(firstPage, fsi.findFreeSpace(BufferManager::pageSize/2));

  int secondPage = fsi.findFreeSpace(BufferManager::pageSize/2);
  fsi.updateFreeSpace(secondPage, BufferManager::pageSize/4);
  EXPECT_EQ(secondPage, fsi.findFreeSpace(BufferManager::pageSize/8));
  EXPECT_NE(secondPage, fsi.findFreeSpace(BufferManager::pageSize/2));
  
  fsi.updateFreeSpace(firstPage, BufferManager::pageSize);
  EXPECT_EQ(firstPage, fsi.findFreeSpace(BufferManager::pageSize/2));
}
