#include <gtest/gtest.h>
#include <vector>
#include "operators/register.h"

using namespace dbImpl;

TEST(Register, storesIntegers) {
  Register r1(1);
  EXPECT_EQ(1, r1.getInteger());
  Register r2;
  r2.setInteger(12);
  EXPECT_EQ(12, r2.getInteger());
  r2.setInteger(42);
  EXPECT_EQ(42, r2.getInteger());
}

TEST(Register, storesStrings) {
  Register r1("hello");
  EXPECT_EQ("hello", r1.getString());
  Register r2;
  r2.setString("hello");
  EXPECT_EQ("hello", r2.getString());
  r2.setString("world");
  EXPECT_EQ("world", r2.getString());
}

TEST(Register, enforcesTypes) {
  Register i(1);
  Register s("2");
  EXPECT_NO_THROW (i.getInteger());
  EXPECT_ANY_THROW(i.getString());
  EXPECT_ANY_THROW(s.getInteger());
  EXPECT_NO_THROW (s.getString());
}

TEST(Register, canChangeType) {
  Register r1("hello");
  EXPECT_EQ("hello", r1.getString());
  r1.setInteger(2);
  EXPECT_EQ(2, r1.getInteger());
  r1.setString("world");
  EXPECT_EQ("world", r1.getString());
}

TEST(Register, supportsAssignments) {
  Register r1("hello");
  Register r2(r1); //test the copy constructor
  Register r3;
  r3 = r2; //test the operator=
  EXPECT_EQ("hello", r1.getString());
  EXPECT_EQ("hello", r2.getString());
  EXPECT_EQ("hello", r3.getString());

  //make sure that copies are really independent
  r1.setString("world");
  EXPECT_EQ("world", r1.getString());
  EXPECT_EQ("hello", r2.getString());
  EXPECT_EQ("hello", r3.getString());
}

TEST(Register, supportsComparisions) {
  Register s1("hallo");
  Register s1_2("hallo");
  Register s2("welt");
  Register i1(5);
  Register i1_2(5);
  Register i2(10);
  EXPECT_NE(s1, s2);
  EXPECT_NE(i1, i2);
  EXPECT_NE(s1, i1);
  EXPECT_NE(s1, i2);
  EXPECT_EQ(s1, s1_2);
  EXPECT_EQ(i1, i1_2);

  EXPECT_TRUE(i1 < i2);
  EXPECT_FALSE(i1 < i1);
  EXPECT_FALSE(i1 < i1_2);
  EXPECT_TRUE(s1 < s2);
  EXPECT_FALSE(s1 < s1);
  EXPECT_FALSE(s1 < s1_2);
}
