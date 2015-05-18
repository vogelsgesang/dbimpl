#include "RelationSchema.h"
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <stdexcept>

namespace dbImpl {

char serializeType(const TypeTag type) {
  switch (type) {
  case TypeTag::Integer:
    return 'i';
  case TypeTag::Char:
    return 'c';
  }
  throw std::runtime_error("could not serialize attribute type");
}

TypeTag loadType(char type) {
  if (type == 'i') {
    return TypeTag::Integer;
  } else if (type == 'c') {
    return TypeTag::Char;
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
  curOffset += sizeof(size);

  memcpy(&segmentID, data + curOffset, sizeof(segmentID));
  curOffset += sizeof(segmentID);

  uint64_t attSize;
  memcpy(&attSize, data + curOffset, sizeof(attSize));
  curOffset += sizeof(attSize);

  uint64_t keySize;
  memcpy(&keySize, data + curOffset, sizeof(keySize));
  curOffset += sizeof(keySize);

  attributes.reserve(attSize);
  for (uint64_t i = 0; i < attSize; i++) {
    AttributeDescriptor a;
    a.name = std::string(reinterpret_cast<const char*>(data + curOffset));
    curOffset += a.name.size() + 1;

    char type = data[curOffset];
    curOffset += sizeof(char);

    a.type = loadType(type);
    if (a.type == TypeTag::Integer) {
      a.len = ~0;
    } else {
      memcpy(&a.len, data + curOffset, sizeof(a.len));
      curOffset += sizeof(a.len);
    }

    memcpy(&a.notNull, data + curOffset, sizeof(bool));
    curOffset += sizeof(bool);

    attributes.push_back(a);
  }

  //Parse Primary Key
  primaryKey.reserve(keySize);

  for (uint64_t i = 0; i < keySize; i++) {
    unsigned k;
    memcpy(&k, data + curOffset, sizeof(k));
    curOffset += sizeof(k);
    primaryKey.push_back(k);
  }

  if(curOffset != record.getLen()) {
    throw std::runtime_error("actually consumed number of bytes does not match the record's size");
  }
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
    if(a.type == TypeTag::Char) {
      memSize += sizeof(unsigned);
    }
    memSize += sizeof(bool);
  }

  uint64_t keySize = primaryKey.size();
  memSize += keySize * sizeof(unsigned);

  uint8_t* data = new uint8_t[memSize];
  uint8_t* curPos = data;

  strcpy(reinterpret_cast<char*>(curPos), name.c_str());
  curPos += name.size() + 1;

  memcpy(curPos, &size, sizeof(size));
  curPos += sizeof(size);

  memcpy(curPos, &segmentID, sizeof(segmentID));
  curPos += sizeof(segmentID);

  uint64_t attSize = attributes.size();
  memcpy(curPos, &attSize, sizeof(attSize));
  curPos += sizeof(attSize);

  memcpy(curPos, &keySize, sizeof(keySize));
  curPos += sizeof(keySize);

  for (const AttributeDescriptor& a : attributes) {
    strcpy(reinterpret_cast<char*>(curPos), a.name.c_str());
    curPos += a.name.size() + 1;

    char type = serializeType(a.type);
    memcpy(curPos, &type, sizeof(type));
    curPos += sizeof(type);

    if (a.type != TypeTag::Integer) {
      memcpy(curPos, &a.len, sizeof(a.len));
      curPos += sizeof(a.len);
    }
    memcpy(curPos, &a.notNull, sizeof(bool));
    curPos += sizeof(bool);
  }

  for (unsigned key : primaryKey) {
    memcpy(curPos, &key, sizeof(key));
    curPos += sizeof(key);
  }

  if(static_cast<unsigned>(curPos - data) != memSize) {
    throw std::runtime_error("actual size does not match calculated size");
  }

  return Record(curPos - data, data);
}

} //namespace dbimpl
