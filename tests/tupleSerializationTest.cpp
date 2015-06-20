#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "operators/register.h"
#include "operators/tupleSerializer.h"
#include "operators/tupleDeserializer.h"

using namespace dbImpl;

TEST(TupleSerialization, allowsRoundtrips) {
  TupleSerializer serialize;
  TupleDeserializer deserialize({TypeTag::Integer, TypeTag::Char, TypeTag::Integer});

  Record r = serialize({Register(42), Register("The answer to life"), Register(666)});
  std::vector<Register> decoded = deserialize(r);
  ASSERT_EQ(3, decoded.size());
  EXPECT_EQ(42, decoded.at(0).getInteger());
  EXPECT_EQ("The answer to life", decoded.at(1).getString());
  EXPECT_EQ(666, decoded.at(2).getInteger());
}

TEST(TupleSerialization, allowsNullCharactersInStrings) {
  TupleSerializer serialize;
  TupleDeserializer deserialize({TypeTag::Char});

  Record r = serialize({Register(std::string("null\0char", 9))});
  std::vector<Register> decoded = deserialize(r);
  ASSERT_EQ(1, decoded.size());
  EXPECT_EQ(std::string("null\0char", 9), decoded.at(0).getString());
}

TEST(TupleSerialization, detectsInvalidDeserialization) {
  TupleSerializer serialize;
  TupleDeserializer deserialize({TypeTag::Char});

  Record r = serialize({Register(100)});
  EXPECT_ANY_THROW(deserialize(r)); //record too short

  Record r2 = serialize({Register("Hello"), Register(1)});
  EXPECT_ANY_THROW(deserialize(r2)); //record too long
}
