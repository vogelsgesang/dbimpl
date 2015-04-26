#include <gtest/gtest.h>
#include <exception>

#include "buffer/twoQ.h"

using namespace dbImpl;


TEST(TwoQTest, behavesLikeAFifo) {
  TwoQ twoQ;
  
  EXPECT_TRUE(twoQ.empty());
  
  twoQ.access(1);
  twoQ.access(2);
  twoQ.access(3);
  
  EXPECT_EQ(1, twoQ.evict());
  EXPECT_EQ(2, twoQ.evict());
  EXPECT_EQ(3, twoQ.evict());

  EXPECT_TRUE(twoQ.empty());
}

TEST(TwoQTest, evictThrowsWhenEmpty) {
  TwoQ twoQ;
  
  EXPECT_TRUE(twoQ.empty());
  EXPECT_ANY_THROW(twoQ.evict());
}

TEST(TwoQTest, doesntDuplicatePages) {
  TwoQ twoQ;
  
  EXPECT_TRUE(twoQ.empty());
  
  twoQ.access(1);
  twoQ.access(1);
  
  EXPECT_EQ(1, twoQ.evict());
  EXPECT_ANY_THROW(twoQ.evict());
  EXPECT_TRUE(twoQ.empty());
}

TEST(TwoQTest, movesToLRUQueues) {
  TwoQ twoQ;
  
  EXPECT_TRUE(twoQ.empty());
  
  twoQ.access(1);
  twoQ.access(2);
  twoQ.access(3);
  twoQ.access(1);
  
  EXPECT_EQ(2, twoQ.evict());
  EXPECT_EQ(3, twoQ.evict());
  
  twoQ.access(4);
  twoQ.access(5);

  EXPECT_EQ(4, twoQ.evict());
  EXPECT_EQ(5, twoQ.evict());
  EXPECT_EQ(1, twoQ.evict());

  EXPECT_TRUE(twoQ.empty());
}

TEST(TwoQTest, behavesLikeALRU) {
  TwoQ twoQ;
  
  EXPECT_TRUE(twoQ.empty());
  
  twoQ.access(1);
  twoQ.access(2);
  twoQ.access(2);
  twoQ.access(1);
  
  EXPECT_EQ(2, twoQ.evict());
  
  twoQ.access(2);
  twoQ.access(2);
  twoQ.access(1);
  twoQ.access(1);
  twoQ.access(2);
  
  EXPECT_EQ(1, twoQ.evict());
}






