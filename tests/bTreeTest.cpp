#include <gtest/gtest.h>

#include "buffer/bufferManager.h"
#include "bTree/bTree.h"

using namespace dbImpl;

struct UInt64Cmp {
  bool operator()(uint64_t a, uint64_t b) const {
    return a < b;
  }
};

TEST(BTreeTest, insertAndDeleteOneNode) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> test(bm);
  test.insert(10, 50);
  ASSERT_TRUE(test.lookup(10));
  EXPECT_EQ(50, test.lookup(10).get());
  test.erase(10);
  ASSERT_FALSE(test.lookup(10));

}

TEST(BTreeTest, insertandDeleteSortedNodes) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> test(bm);

  for (int i = 0; i < 20; i++) {
    test.insert(i, 2 * i);
  }
  for (int i = 0; i < 20; i++) {
    ASSERT_TRUE(test.lookup(i));
    EXPECT_EQ(i * 2, test.lookup(i).get());
  }
  for (int i = 0; i < 20; i++) {
    test.erase(i);
  }
  for (int i = 0; i < 20; i++) {
    ASSERT_FALSE(test.lookup(i));
  }
}

TEST(BTreeTest, splitLeafs) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> test(bm);
  for (int i = 0; i < 5000; i++) {
    test.insert(i, i);
  }
  for (int i = 0; i < 5000; i++) {
    ASSERT_TRUE(test.lookup(i));
    EXPECT_EQ(i, test.lookup(i).get());
  }
}

TEST(BTreeTest, splitInnerNode) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> test(bm);
  for (uint64_t i = 0; i < 1000000; i++) {
    test.insert(i, i);
  }
  for (int i = 0; i < 1000000; i++) {
    ASSERT_TRUE(test.lookup(i));
    EXPECT_EQ(i, test.lookup(i).get());
  }

}
