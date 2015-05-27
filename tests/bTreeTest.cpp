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
  BTree<uint64_t, UInt64Cmp> bTree(bm);
  bTree.insert(10, 50);
  ASSERT_TRUE(bTree.lookup(10));
  EXPECT_EQ(50, bTree.lookup(10).get());
  bTree.erase(10);
  ASSERT_FALSE(bTree.lookup(10));

}
TEST(BTreeTest, EmptyTree) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> bTree(bm);
  EXPECT_EQ(0,bTree.size());
  ASSERT_FALSE(bTree.lookup(50));
  ASSERT_FALSE(bTree.erase(50));

  std::vector<uint64_t> vec = bTree.lookupRange(0,~0);
  EXPECT_EQ(vec.size(),0);

}

TEST(BTreeTest, lookupRange) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> bTree(bm);

  uint64_t n = 1000000;
  for (uint64_t i = 0; i < n; i++) {
    bTree.insert(i, i);
  }
  std::vector<uint64_t> vec = bTree.lookupRange(100000,n-5000);
  EXPECT_EQ(vec.size(),n-100000-5000+1);
  EXPECT_EQ(100000,vec[0]);
  EXPECT_EQ(n-5000,vec[vec.size()-1]);

}


TEST(BTreeTest,givenTestCase) {
  BufferManager bm(50);
  BTree<uint64_t, UInt64Cmp> bTree(bm);
  // Insert values
  uint64_t n = 1000 * 1000ul;
  for (uint64_t i = 0; i < n; ++i)
    bTree.insert(i, i * i);
  assert(bTree.size() == n);

  // Check if they can be retrieved
  for (uint64_t i = 0; i < n; ++i) {
    assert(bTree.lookup(i));
    boost::optional < uint64_t > tid = bTree.lookup(i);
    assert(tid == i * i);
  }

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

  // Delete everything
  for (uint64_t i = 0; i < n; ++i)
    bTree.erase(i);
  assert(bTree.size() == 0);

}

