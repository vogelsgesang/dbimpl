#include <gtest/gtest.h>

#include "schema/RelationSchema.h"

TEST(RelationSchemaTest, serializesRelation) {
  dbImpl::RelationSchema studentenSchema("studenten",
      {
        dbImpl::AttributeDescriptor("id", dbImpl::Types::Tag::Integer, 0, true),
        dbImpl::AttributeDescriptor("name", dbImpl::Types::Tag::Char, 30),
        dbImpl::AttributeDescriptor("semester", dbImpl::Types::Tag::Integer),
      },
      { 0 },
      3,1
    );
	
  dbImpl::Record serializedSchema = studentenSchema.serializeToRecord();
  dbImpl::RelationSchema reloaded(serializedSchema);

  EXPECT_EQ(std::string("studenten"), reloaded.name);
	//TODO...
}
