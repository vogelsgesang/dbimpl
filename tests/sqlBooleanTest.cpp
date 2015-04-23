#include <gtest/gtest.h>

#include "logic/sqlBool.h"
using namespace dbImpl;

TEST(SqlBooleanTest, representsStates) {
  EXPECT_TRUE (SqlBool::trueValue().isTrue());
  EXPECT_FALSE(SqlBool::trueValue().isFalse());
  EXPECT_FALSE(SqlBool::trueValue().isUnknown());

  EXPECT_FALSE(SqlBool::falseValue().isTrue());
  EXPECT_TRUE (SqlBool::falseValue().isFalse());
  EXPECT_FALSE(SqlBool::falseValue().isUnknown());

  EXPECT_FALSE(SqlBool::unknownValue().isTrue());
  EXPECT_FALSE(SqlBool::unknownValue().isFalse());
  EXPECT_TRUE (SqlBool::unknownValue().isUnknown());
}

TEST(SqlBooleanTest, inversionWorks) {
  SqlBool notTrue = !SqlBool::trueValue();
  EXPECT_FALSE(notTrue.isTrue());
  EXPECT_TRUE (notTrue.isFalse());
  EXPECT_FALSE(notTrue.isUnknown());

  SqlBool notFalse = !SqlBool::falseValue();
  EXPECT_TRUE (notFalse.isTrue());
  EXPECT_FALSE(notFalse.isFalse());
  EXPECT_FALSE(notFalse.isUnknown());

  SqlBool notUnknown = !SqlBool::unknownValue();
  EXPECT_FALSE(notUnknown.isTrue());
  EXPECT_FALSE(notUnknown.isFalse());
  EXPECT_TRUE (notUnknown.isUnknown());
}

TEST(SqlBooleanTest, andWorks) {
  SqlBool _true    = SqlBool::trueValue();
  SqlBool _false   = SqlBool::falseValue();
  SqlBool _unknown = SqlBool::unknownValue();

  EXPECT_TRUE((_true  && _true ).isTrue());
  EXPECT_TRUE((_true  && _false).isFalse());
  EXPECT_TRUE((_false && _true ).isFalse());
  EXPECT_TRUE((_false && _false).isFalse());

  EXPECT_TRUE((_unknown && _true   ).isUnknown());
  EXPECT_TRUE((_unknown && _false  ).isUnknown());
  EXPECT_TRUE((_true    && _unknown).isUnknown());
  EXPECT_TRUE((_false   && _unknown).isUnknown());
  EXPECT_TRUE((_unknown && _unknown).isUnknown());
}

TEST(SqlBooleanTest, orWorks) {
  SqlBool _true    = SqlBool::trueValue();
  SqlBool _false   = SqlBool::falseValue();
  SqlBool _unknown = SqlBool::unknownValue();

  EXPECT_TRUE((_true  || _true ).isTrue());
  EXPECT_TRUE((_true  || _false).isTrue());
  EXPECT_TRUE((_false || _true ).isTrue());
  EXPECT_TRUE((_false || _false).isFalse());

  EXPECT_TRUE((_unknown || _true   ).isUnknown());
  EXPECT_TRUE((_unknown || _false  ).isUnknown());
  EXPECT_TRUE((_true    || _unknown).isUnknown());
  EXPECT_TRUE((_false   || _unknown).isUnknown());
  EXPECT_TRUE((_unknown || _unknown).isUnknown());
}
