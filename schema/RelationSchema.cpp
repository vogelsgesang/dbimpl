#include "RelationSchema.h"
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <stdexcept>

namespace dbImpl {

char serializeType(const Types::Tag type) {
  switch (type) {
  case Types::Tag::Integer:
    return 'i';
  case Types::Tag::Char: 
    return 'c';
  }
  throw std::runtime_error("could not serialize attribute type");
}

Types::Tag loadType(char type) {
  if (type == 'i') {
    return Types::Tag::Integer;
  } else if (type == 'c') {
    return Types::Tag::Char;
  } else {
    throw std::runtime_error("could not deserialize attribute type");
  }
}

RelationSchema::RelationSchema(Record& record) {
  const uint8_t* data = record.getData();
  if (data == NULL || record.getLen() == 0) {
    throw std::runtime_error("loadFromRecord called with an empty record");
  }

  unsigned curOffset = 0;
  name = std::string(reinterpret_cast<const char*>(data + curOffset));
  curOffset += name.size() + 1;

  memcpy(&size, data + curOffset, sizeof(size));
  curOffset += sizeof(size) + 1;

  memcpy(&segmentID, data + curOffset, sizeof(segmentID));
  curOffset += sizeof(segmentID) + 1;

  uint64_t attSize;
  memcpy(&attSize, data + curOffset, sizeof(attSize));
  curOffset += sizeof(attSize) + 1;

  uint64_t keySize;
  memcpy(&keySize, data + curOffset, sizeof(keySize));
  curOffset += sizeof(keySize) + 1;

  attributes.reserve(attSize);
  for (uint64_t i = 0; i < attSize; i++) {
    AttributeDescriptor a;
    a.name = std::string(reinterpret_cast<const char*>(data + curOffset));
    curOffset += a.name.size() + 1;

    char type = data[curOffset];
    curOffset += sizeof(char) + 1;

    a.type = loadType(type);
    if (a.type == Types::Tag::Integer) {
      a.len = ~0;
    } else {
      memcpy(&a.len, data + curOffset, sizeof(a.len));
      curOffset += sizeof(a.len) + 1;
    }

    memcpy(&a.notNull, data + curOffset, sizeof(bool));
    curOffset += sizeof(bool) + 1;

    attributes.push_back(a);
  }

  //Parse Primary Key
  primaryKey.reserve(keySize);

  for (uint64_t i = 0; i < keySize; i++) {
    unsigned k;
    memcpy(&k, data + curOffset, sizeof(k));
    curOffset += sizeof(k) + 1;
    primaryKey.push_back(k);
  }

  #ifdef DEBUG
  if(curOffset != record.getLen()) {
    throw std::runtime_error("actually consumed number of bytes does not match the record's size");
  }
  #endif
}

Record RelationSchema::serializeToRecord() const {
  unsigned memSize = 0;
  memSize += name.size()+1;
  memSize += sizeof(size);
  memSize += sizeof(segmentID);
  memSize += sizeof(uint64_t)*2; //Attribute SIze + Primary Key Size

  for (const AttributeDescriptor& a : attributes) {
    memSize += a.name.size()+1;
    memSize += sizeof(char);
    memSize += sizeof(unsigned);
    memSize += sizeof(bool);
  }

  uint64_t keySize = primaryKey.size();
  memSize += keySize * sizeof(unsigned);

  uint8_t* data = new uint8_t[memSize];
  uint8_t* curPos = data;

  strcpy(reinterpret_cast<char*>(curPos), name.c_str());
  curPos += name.size() + 1;

  memcpy(curPos, &size, sizeof(size));
  curPos += sizeof(size) + 1;

  memcpy(curPos, &segmentID, sizeof(segmentID));
  curPos += sizeof(segmentID) + 1;

  uint64_t attSize = attributes.size();
  memcpy(curPos, &attSize, sizeof(attSize));
  curPos += sizeof(attSize) + 1;


  memcpy(curPos, &keySize, sizeof(keySize));
  curPos += sizeof(keySize) + 1;

  for (const AttributeDescriptor& a : attributes) {
    strcpy(reinterpret_cast<char*>(curPos), a.name.c_str());
    curPos += a.name.size() + 1;

    char type = serializeType(a.type);
    memcpy(curPos, &type, sizeof(type));
    curPos += sizeof(type) + 1;

    if (a.type != Types::Tag::Integer) {
      memcpy(curPos, &a.len, sizeof(a.len));
      curPos += sizeof(a.len) + 1;
    }
    memcpy(curPos, &a.notNull, sizeof(bool));
    curPos += sizeof(bool) + 1;
  }

  for (unsigned key : primaryKey) {
    memcpy(curPos, &key, sizeof(key));
    curPos += sizeof(key) + 1;
  }

  #ifdef DEBUG
  if(static_cast<unsigned>(curPos - data) != memSize) {
    throw std::runtime_error("actual size does not match calculated size");
  }
  #endif

  return Record(curPos - data, data);
}

} //namespace dbimpl
