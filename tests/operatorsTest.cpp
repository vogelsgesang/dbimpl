#include <gtest/gtest.h>

#include "operators/register.h"

using namespace dbImpl;

TEST(Operators, Register) {
  Register s1("hallo");
  Register s2("welt");
  ASSERT_FALSE(s1 == s2);

  s2.setString("hallo");
  EXPECT_EQ(s1,s2);
  EXPECT_EQ(s1.getString(),"hallo");
  ASSERT_TRUE(s1 == s2);

  Register i1(5);
  Register i2(10);
  ASSERT_TRUE(i1 < i2);
  EXPECT_EQ(i1.getInteger(),5);
  EXPECT_EQ(i2.getInteger(),10);


}
