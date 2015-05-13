#ifndef __SCHEMA_H
#define __SCHEMA_H

struct Relation {
	struct Attribute {
		std::string name;
		Types::Tag type;

	};

	std::string name;
	uint64_t segment_id;
	uint64_t size; // [in pages]
	vector<Attribute> attributes;


	//all Attributes and Types

};

#endif
