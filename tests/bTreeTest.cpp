#include <gtest/gtest.h>

#include "buffer/bufferManager.h"
#include "bTree/bTree.h"

using namespace dbImpl;

struct UInt64Cmp {
  bool operator()(uint64_t a, uint64_t b) const {
    return a < b;
  }
};

TEST(BTreeTest, insertOneNode) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> test(bm);
  test.insert(10, 50);
  ASSERT_TRUE(test.lookup(10));
  EXPECT_EQ(50, test.lookup(10).get());

}

