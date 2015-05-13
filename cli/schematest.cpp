#include <gtest/gtest.h>

#include "schema/RelationSchema.h"

using namespace dbimpl;

TEST(RelationSchemaTest, serializesRelation) {
    std::string name = "Test";
    std::vector<AttributeDescriptor> attributes;
    std::vector<unsigned> primaryKey;
    uint64_t size = 1; 
	uint64_t segmentID = 1;

	AttributeDescriptor a1 = new AttributeDescriptor();
	a1.name = "A1";
	a1.type = Types::Tag::Char;
	a1.len = 5;
	a1.notNull = true;
	
	//TODO...

  
}

