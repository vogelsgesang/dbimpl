#include <gtest/gtest.h>

#include "buffer/bufferManager.h"
#include "bTree/bTree.h"

using namespace dbImpl;

struct UInt64Cmp {
   bool operator()(uint64_t a, uint64_t b) const {
      return a<b;
   }
};


TEST(BTreeTest, insertOneNode) {

  BTree<uint64_t,UInt64Cmp> test;
  test.insert(10,50);

  EXPECT_EQ(50,test.lookup(10).value());


}


