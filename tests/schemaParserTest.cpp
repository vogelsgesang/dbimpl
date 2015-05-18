#include <gtest/gtest.h>
#include <sstream>

#include "schema/RelationSchema.h"
#include "schema/SchemaParser.h"

TEST(SchemaParserTest, parsesAnEmptySchemaDefinitions) {
  dbImpl::SchemaParser parser;
  std::string schemaDefinition(" \n \t ");
  std::istringstream in(schemaDefinition);

  std::vector<dbImpl::RelationSchema> schema = parser.parse(in);
  EXPECT_EQ(0, schema.size());
}

TEST(SchemaParserTest, parsesValidSchemaDefinitions) {
  dbImpl::SchemaParser parser;
  std::string schemaDefinition(
      "CREATE TABLE test1 ("
      "  attr1 integer,"
      "  attr2 INTEGER,"
      "  attr3 INTEGER NOT NULL,"
      "  attr4 CHAR(1),"
      "  attr5 CHAR(10) not NULL,"
      "  PRIMARY key (attr2, attr4)"
      ");"
      "CREATE TABLE test2 ("
      "  attr1 integer"
      ");"
      );
  std::istringstream in(schemaDefinition);
  
  std::vector<dbImpl::RelationSchema> schema = parser.parse(in);

  ASSERT_EQ(2, schema.size());
  EXPECT_EQ(std::string("test1"), schema[0].name);
  EXPECT_EQ(std::string("test2"), schema[1].name);

  ASSERT_EQ(5, schema[0].attributes.size());

  EXPECT_EQ(std::string("attr1"),     schema[0].attributes[0].name);
  EXPECT_EQ(dbImpl::TypeTag::Integer, schema[0].attributes[0].type);
  EXPECT_FALSE(                       schema[0].attributes[0].notNull);

  EXPECT_EQ(std::string("attr2"),     schema[0].attributes[1].name);
  EXPECT_EQ(dbImpl::TypeTag::Integer, schema[0].attributes[1].type);
  EXPECT_FALSE(                       schema[0].attributes[1].notNull);

  EXPECT_EQ(std::string("attr3"),     schema[0].attributes[2].name);
  EXPECT_EQ(dbImpl::TypeTag::Integer, schema[0].attributes[2].type);
  EXPECT_TRUE(                        schema[0].attributes[2].notNull);

  EXPECT_EQ(std::string("attr4"),     schema[0].attributes[3].name);
  EXPECT_EQ(dbImpl::TypeTag::Char,    schema[0].attributes[3].type);
  EXPECT_EQ(1,                        schema[0].attributes[3].len);
  EXPECT_FALSE(                       schema[0].attributes[3].notNull);

  EXPECT_EQ(std::string("attr5"),     schema[0].attributes[4].name);
  EXPECT_EQ(dbImpl::TypeTag::Char,    schema[0].attributes[4].type);
  EXPECT_EQ(10,                       schema[0].attributes[4].len);
  EXPECT_TRUE(                        schema[0].attributes[4].notNull);

  EXPECT_EQ(std::string("attr1"),     schema[1].attributes[0].name);
  EXPECT_EQ(dbImpl::TypeTag::Integer, schema[1].attributes[0].type);
  EXPECT_FALSE(                       schema[1].attributes[0].notNull);

  ASSERT_EQ(2, schema[0].primaryKey.size());
  EXPECT_EQ(1, schema[0].primaryKey[0]);
  EXPECT_EQ(3, schema[0].primaryKey[1]);
  ASSERT_EQ(0, schema[1].primaryKey.size());
}

TEST(SchemaParserTest, rejectsTruncatedSchemaDefinitions) {
  dbImpl::SchemaParser parser;
  std::string schemaDefinition(
      "CREATE TABLE test("
      "  attr1 integer,"
      );
  std::istringstream in(schemaDefinition);

  EXPECT_ANY_THROW(std::vector<dbImpl::RelationSchema> schema = parser.parse(in));
}
