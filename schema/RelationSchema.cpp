#include "RelationSchema.h"
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <iostream>

namespace dbImpl {

static std::string serializeType(const Types::Tag type) {
	switch (type) {
	case Types::Tag::Integer:
		return "Integer";

	case Types::Tag::Char: {
		return "Char";
	}
	}
	std::cerr << "could not serialize Type" << std::endl;
	return "";
}

static Types::Tag loadType(std::string typeInfo) {
	if (typeInfo.compare("Integer") == 0)
		return Types::Tag::Integer;
	else //(typeInfo.compare("Char") == 0)
	return Types::Tag::Char;


}

RelationSchema* RelationSchema::loadFromRecord(Record& record) {

	const char* data = record.getData();
	if (data == NULL) {
		std::cerr << "Read data of relation schema is null" << std::endl;

	}

	std::stringstream ss(data);

	//Parse first line (name + Attribute Size + PrimaryKey Size)

	ss >> name;
	std::string tmp;
	ss >> size;
	ss >> segmentID;
	uint64_t attSize;
	ss >> attSize;
	uint64_t keySize;
	ss >> keySize;

	//Parse Attributes

	attributes.resize(attSize);
	for (uint64_t i = 0; i < attSize; i++) {
		AttributeDescriptor a;
		ss >> a.name;
		ss >> tmp;
		loadType (tmp);
		ss >> a.len;
		ss >> tmp;
		if (tmp.compare("1") == 0) {
			a.notNull = true;
		}
		attributes.push_back(a);

	}

	//Parse Primary Key
	primaryKey.resize(keySize);
	for (uint64_t i = 0; i < keySize; i++) {
		unsigned k;
		ss >> k;
		primaryKey.push_back(k);
	}

	return this;

}

Record RelationSchema::serializeToRecord() {

	std::stringstream recData;
	recData << name << " " << size << " " << segmentID << " "
			<< attributes.size() << " " << primaryKey.size() << std::endl;
	//Attributes
	for (const AttributeDescriptor& a : attributes) {
		recData << a.name << " " << serializeType(a.type) << " " << a.len << " "
				<< (a.notNull ? "1" : "0") << std::endl;
	}
	for (unsigned key : primaryKey) {
		recData << key << " ";

	}
	recData << std::endl;

	recData.seekg(0, std::ios::end);
	unsigned len = recData.tellg();

	// create new Record
	std::string tmp = recData.str();
	const char* data = tmp.c_str();
	return Record(len, data);

}

} //namespace dbimpl
