#include <gtest/gtest.h>

#include "buffer/bufferManager.h"
#include "schema/RelationSchema.h"
#include "schema/SchemaSegment.h"

dbImpl::RelationSchema getTestSchema() {
  uint64_t segmentID = 3;
  uint64_t size = 1;
  return dbImpl::RelationSchema("studenten", {
      dbImpl::AttributeDescriptor("id", dbImpl::TypeTag::Integer, ~0, true),
      dbImpl::AttributeDescriptor("name", dbImpl::TypeTag::Char, 30),
      dbImpl::AttributeDescriptor("semester", dbImpl::TypeTag::Integer)}, {
      0, 4, 10, 23 }, segmentID, size);
}

void checkTestSchema(dbImpl::RelationSchema checked) {
  dbImpl::RelationSchema testSchema = getTestSchema();

  EXPECT_EQ(testSchema.name, checked.name);
  EXPECT_EQ(testSchema.segmentID, checked.segmentID);
  EXPECT_EQ(testSchema.size, checked.size);

  EXPECT_EQ(testSchema.attributes.size(), checked.attributes.size());
  EXPECT_EQ(testSchema.primaryKey.size(), checked.primaryKey.size());

  for (unsigned int i = 0; i < testSchema.attributes.size(); i++) {
    EXPECT_EQ(testSchema.attributes[i].name, checked.attributes[i].name);
    EXPECT_EQ(testSchema.attributes[i].notNull,
        checked.attributes[i].notNull);
    EXPECT_EQ(testSchema.attributes[i].len, checked.attributes[i].len);
  }

  for (unsigned int i = 0; i < testSchema.primaryKey.size(); i++) {
    EXPECT_EQ(testSchema.primaryKey[i], checked.primaryKey[i]);
  }
}

TEST(RelationSchemaTest, isSerializable) {
  dbImpl::Record serializedSchema = getTestSchema().serializeToRecord();
  dbImpl::RelationSchema reloaded(serializedSchema);
  checkTestSchema(reloaded);
}

TEST(RelationSchemaTest, canBeStoredIntoSegment) {
  dbImpl::BufferManager bm(100);
  dbImpl::SchemaSegment segment(bm, 0);

  std::vector<dbImpl::RelationSchema> schema = {getTestSchema()};
  segment.store(schema);
  std::vector<dbImpl::RelationSchema> reloaded = segment.read();

  ASSERT_EQ(1, reloaded.size());
  checkTestSchema(reloaded[0]);
}
