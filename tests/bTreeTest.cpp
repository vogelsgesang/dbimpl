#include <gtest/gtest.h>


#include <string>
#include <cstdint>
#include <cassert>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <string.h>

#include "bTree/bTree.h"
#include "buffer/bufferManager.h"

using namespace dbImpl;

typedef uint64_t TID;

/* Comparator functor for uint64_t*/
struct MyCustomUInt64Cmp {
   bool operator()(uint64_t a, uint64_t b) const {
      return a<b;
   }
};

template <unsigned len>
struct Char {
   char data[len];
};

/* Comparator functor for char */
template <unsigned len>
struct MyCustomCharCmp {
   bool operator()(const Char<len>& a, const Char<len>& b) const {
      return memcmp(a.data, b.data, len) < 0;
   }
};

typedef std::pair<uint32_t, uint32_t> IntPair;


/* Comparator for IntPair */
struct MyCustomIntPairCmp {
   bool operator()(const IntPair& a, const IntPair& b) const {
     if (a.first < b.first)
         return true;
      else
         return (a.first == b.first) && (a.second < b.second);
   }
};


template <class T>
const T& getKey(const uint64_t& i);

template <>
const uint64_t& getKey(const uint64_t& i) { return i; }

std::vector<std::string> char20;
template <>
const Char<20>& getKey(const uint64_t& i) {
   std::stringstream ss;
   ss << i;
   std::string s(ss.str());
   char20.push_back(std::string(20-s.size(), '0')+s);
   return *reinterpret_cast<const Char<20>*>(char20.back().data());
}

std::vector<IntPair> intPairs;
template <>
const IntPair& getKey(const uint64_t& i) {
   intPairs.push_back(std::make_pair(i/3, 3-(i%3)));
   return intPairs.back();
}

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

  uint64_t n = 100000;
  for (uint64_t i = 0; i < n; i++) {
    bTree.insert(i, i);
  }
  std::vector<uint64_t> vec = bTree.lookupRange(n-9000,n-5000);
  EXPECT_EQ(vec.size(),9000-5000+1);
  EXPECT_EQ(n-9000,vec[0]);
  EXPECT_EQ(n-5000,vec[vec.size()-1]);
}


template <class T, class CMP>
void test(uint64_t n) {
   // Set up stuff, you probably have to change something here to match to your interfaces
   BufferManager bm(100);
   // ...
   BTree<T, CMP> bTree(bm);

   // Insert values
   for (uint64_t i=0; i<n; ++i)
      bTree.insert(getKey<T>(i),static_cast<TID>(i*i));
   EXPECT_EQ(n,bTree.size());
   assert(bTree.size()==n);

   // Check if they can be retrieved
   for (uint64_t i=0; i<n; ++i) {
      assert(bTree.lookup(getKey<T>(i)));
      boost::optional<TID> tid = bTree.lookup(getKey<T>(i));
      assert(tid==i*i);
   }

   // Delete some values
   for (uint64_t i=0; i<n; ++i)
      if ((i%7)==0)
         bTree.erase(getKey<T>(i));

   // Check if the right ones have been deleted
   for (uint64_t i=0; i<n; ++i) {
      if ((i%7)==0) {
         assert(!bTree.lookup(getKey<T>(i)));
      } else {
         assert(bTree.lookup(getKey<T>(i))==i*i);
      }
   }

   // Delete everything
   for (uint64_t i=0; i<n; ++i)
      bTree.erase(getKey<T>(i));
   assert(bTree.size()==0);
}
TEST(BTreeTest, Uint64_t) {
  uint64_t n = 1000*1000ul;
  test<uint64_t, MyCustomUInt64Cmp>(n);
}
TEST(BTreeTest, 20CharsString) {
  uint64_t n = 1000ul;
  test<Char<20>, MyCustomCharCmp<20>>(n);
}
TEST(BTreeTest, CompoundKey) {
  uint64_t n = 1000ul;
  test<IntPair, MyCustomIntPairCmp>(n);
}


