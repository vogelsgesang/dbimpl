#include <gtest/gtest.h>

#include "slottedPages/SPSegment.h"
#include "buffer/bufferManager.h"

//this class makes some private functions
//of SPSegment public and therefore allows
//whitebox testing
class WhiteboxSPSegment : public dbImpl::SPSegment {
  public:
    using SPSegment::SPSegment; //inherit constructor
    using SPSegment::compressSpaceIndicator;
};

TEST(SPSegmentTest, spaceIndicatorIsMonotonic) {
  dbImpl::BufferManager bm(100);
  WhiteboxSPSegment segment(bm, 1);
  uint8_t lastResult = 0;
  for(uint32_t i = 0; i <= bm.pageSize; i++) {
    EXPECT_GE(segment.compressSpaceIndicator(i), lastResult);
    lastResult = segment.compressSpaceIndicator(i);
  }
}

TEST(SPSegmentTest, spaceIndicatorUsesOnly4Bits) {
  dbImpl::BufferManager bm(100);
  WhiteboxSPSegment segment(bm, 1);
  for(uint32_t i = 0; i <= bm.pageSize; i++) {
    EXPECT_EQ(0, segment.compressSpaceIndicator(i) & 0xf0);
  }
}

TEST(SPSegmentTest, spaceIndicatorUsesCompleteRange) {
  dbImpl::BufferManager bm(100);
  WhiteboxSPSegment segment(bm, 1);
  EXPECT_EQ(0, segment.compressSpaceIndicator(0));
  uint8_t lastResult = 0;
  for(uint32_t i = 0; i <= bm.pageSize; i++) {
    EXPECT_LE(segment.compressSpaceIndicator(i) - lastResult, 1);
    lastResult = segment.compressSpaceIndicator(i);
  }
  EXPECT_EQ(0x0f, segment.compressSpaceIndicator(bm.pageSize));
}
