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
  //bm.~BufferManager();

}
TEST(BTreeTest, insertNodedSorted) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> test(bm);

  for (int i = 10; i < 20; i++) {
    test.insert(i, 2 * i);
  }
  for (int i = 10; i < 20; i++) {
    ASSERT_TRUE(test.lookup(i));
    EXPECT_EQ(i * 2, test.lookup(i).get());
  }
  //bm.~BufferManager();

}

