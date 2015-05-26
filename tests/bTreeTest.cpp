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

TEST(BTreeTest, splitNodes) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> test(bm);
  for (uint64_t i = 0; i < 1000000; i++) {
    test.insert(i, i);
  }
  EXPECT_EQ(1000000, test.size());
  for (int i = 0; i < 1000000; i++) {
    ASSERT_TRUE(test.lookup(i));
    EXPECT_EQ(i, test.lookup(i).get());
  }

}

TEST(BTreeTest,givenTestCase) {
  BufferManager bm(2000);
  BTree<uint64_t, UInt64Cmp> bTree(bm);
  // Insert values
  uint64_t n = 1000*1000ul;
  for (uint64_t i = 0; i < n; ++i)
    bTree.insert(i, i * i);
  assert(bTree.size() == n);

  // Check if they can be retrieved
  for (uint64_t i = 0; i < n; ++i) {
    assert(bTree.lookup(i));
    boost::optional < uint64_t > tid = bTree.lookup(i);
    assert(tid == i * i);
  }
  std::cout << bTree.size() << std::endl;
  // Delete some values
  for (uint64_t i = 0; i < n; ++i)
    if ((i % 7) == 0)
      bTree.erase(i);

  // Check if the right ones have been deleted
  for (uint64_t i = 0; i < n; ++i) {
    if ((i % 7) == 0) {
      assert(!bTree.lookup(i));
    } else {
      assert(bTree.lookup(i));
      assert(bTree.lookup(i) == i * i);
    }
  }
/* TODO fix erase
  // Delete everything
  for (uint64_t i = 0; i < n; ++i)
    bTree.erase(i);
  std::cout << bTree.size() << std::endl;
  assert(bTree.size() == 0);
  */
}
