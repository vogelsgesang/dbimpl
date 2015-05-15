#include "RelationSchema.h"
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <stdexcept>

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
	if (typeInfo.compare("Integer") == 0) {
		return Types::Tag::Integer;
  } else if(typeInfo.compare("Char") == 0) {
    return Types::Tag::Char;
  } else {
    throw std::runtime_error("invalid type: " + typeInfo);
  }
}

RelationSchema::RelationSchema(Record& record) {
	const char* data = record.getData();
	if (data == NULL || record.getLen() == 0) {
    throw std::runtime_error("loadFromRecord called with an empty record");
	}

	std::stringstream ss(data);

	//Parse first line (name + Attribute Size + PrimaryKey Size)
	ss >> name;
	ss >> size;
	ss >> segmentID;
	uint64_t attSize;
	ss >> attSize;
	uint64_t keySize;
	ss >> keySize;

	//Parse Attributes
	std::string tmp;
	attributes.resize(attSize);
	for (uint64_t i = 0; i < attSize; i++) {
		AttributeDescriptor a;
		ss >> a.name;
		ss >> tmp;
		a.type = loadType (tmp);
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
}

Record RelationSchema::serializeToRecord() const {

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
	const char* data = tmp.c_str(); //TODO: doesn't work
	return Record(len, data);
}

} //namespace dbimpl
