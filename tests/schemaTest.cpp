#include <gtest/gtest.h>

#include "schema/RelationSchema.h"

TEST(RelationSchemaTest, serializesRelation) {
  uint64_t segmentID = 3;
  uint64_t size = 1;
  dbImpl::RelationSchema studentenSchema("studenten", {
      dbImpl::AttributeDescriptor("id", dbImpl::Types::Tag::Integer, ~0, true),
      dbImpl::AttributeDescriptor("name", dbImpl::Types::Tag::Char, 30),
      dbImpl::AttributeDescriptor("semester", dbImpl::Types::Tag::Integer)}, {
      0, 4, 10, 23 }, segmentID, size);

  dbImpl::Record serializedSchema = studentenSchema.serializeToRecord();
  dbImpl::RelationSchema reloaded(serializedSchema);

  EXPECT_EQ(std::string("studenten"), reloaded.name);
  EXPECT_EQ(segmentID, reloaded.segmentID);
  EXPECT_EQ(size, reloaded.size);

  EXPECT_EQ(studentenSchema.attributes.size(), reloaded.attributes.size());
  EXPECT_EQ(studentenSchema.primaryKey.size(), reloaded.primaryKey.size());

  for (unsigned int i = 0; i < studentenSchema.attributes.size(); i++) {
    EXPECT_EQ(studentenSchema.attributes[i].name, reloaded.attributes[i].name);
    EXPECT_EQ(studentenSchema.attributes[i].notNull,
        reloaded.attributes[i].notNull);
    EXPECT_EQ(studentenSchema.attributes[i].len, reloaded.attributes[i].len);
  }

  for (unsigned int i = 0; i < studentenSchema.primaryKey.size(); i++) {
    EXPECT_EQ(studentenSchema.primaryKey[i], reloaded.primaryKey[i]);

  }

}
