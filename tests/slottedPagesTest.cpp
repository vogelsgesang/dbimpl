#include <gtest/gtest.h>
#include <string>

#include "buffer/bufferManager.h"
#include "slottedPages/spSegment.h"

TEST(SlottedPagesTest, storesRecords) {
  dbImpl::BufferManager bm(100);
  dbImpl::SPSegment spSegment(bm, 1);
  
  std::string testStr("Test");
  dbImpl::Record testRecord(testStr.length() + 1, reinterpret_cast<const uint8_t*>(testStr.c_str()));
  uint64_t tid = spSegment.insert(testRecord);

  dbImpl::Record readRecord = spSegment.lookup(tid);
  EXPECT_EQ(testStr.length()+1, readRecord.getLen());
  EXPECT_STREQ(testStr.c_str(), reinterpret_cast<const char*>(readRecord.getData()));

  spSegment.remove(tid);
}

TEST(SlottedPagesTest, reusesTids) {
  dbImpl::BufferManager bm(100);
  dbImpl::SPSegment spSegment(bm, 1);

  std::string testStr("Test");
  dbImpl::Record testRecord(testStr.length() + 1, reinterpret_cast<const uint8_t*>(testStr.c_str()));

  uint64_t tid1 = spSegment.insert(testRecord);
  spSegment.remove(tid1);
  uint64_t tid2 = spSegment.insert(testRecord);

  EXPECT_EQ(tid1, tid2);
  spSegment.remove(tid2);
}

TEST(SlottedPagesTest, iteratesOverAllStoredTuples) {
  dbImpl::BufferManager bm(100);
  dbImpl::SPSegment spSegment(bm, 2);
  
  //insert 2 records
  std::string helloStr("Hello");
  dbImpl::Record helloRecord(helloStr.length() + 1, reinterpret_cast<const uint8_t*>(helloStr.c_str()));
  uint64_t tid1 = spSegment.insert(helloRecord);

  std::string worldStr("World");
  dbImpl::Record worldRecord(worldStr.length() + 1, reinterpret_cast<const uint8_t*>(worldStr.c_str()));
  uint64_t tid2 = spSegment.insert(worldRecord);

  //try to get the records by iterating over the segment
  auto iter = spSegment.begin();

  ASSERT_NE(spSegment.end(), iter);
  dbImpl::Record record = *iter;
  EXPECT_STREQ(helloStr.c_str(), reinterpret_cast<const char*>(record.getData()));
  iter++;

  ASSERT_NE(spSegment.end(), iter);
  record = *iter;
  EXPECT_STREQ(worldStr.c_str(), reinterpret_cast<const char*>(record.getData()));
  iter++;

  ASSERT_EQ(spSegment.end(), iter);
  spSegment.remove(tid1);
  spSegment.remove(tid2);
}
