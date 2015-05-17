#include <gtest/gtest.h>
#include <string>

#include "buffer/bufferManager.h"
#include "slottedPages/spSegment.h"

TEST(SlottedPagesTest, storesRecords) {
  dbImpl::BufferManager bm(100);
  dbImpl::SPSegment spSegment(bm, 1);
  
  std::string testStr("Test");
  dbImpl::Record testRecord(testStr.length(), reinterpret_cast<const uint8_t*>(testStr.c_str()));
  uint64_t tid = spSegment.insert(testRecord);

  dbImpl::Record readRecord = spSegment.lookup(tid);
  EXPECT_EQ(testStr.length(), readRecord.getLen());
  EXPECT_STREQ(testStr.c_str(), readRecord.getData());
}
