#ifndef _RELATION_SCHEMA_HPP_
#define _RELATION_SCHEMA_HPP_

#include <vector>
#include <string>
#include <cstdint> //uint64_t
#include "slottedPages/record.h"

namespace dbImpl {
  class Record;

  /**
   * Types
   */
  enum class TypeTag : char {Integer, Char};

  struct AttributeDescriptor {
    std::string name;
    TypeTag type;
    unsigned len;
    bool notNull; 
    AttributeDescriptor() : len(~0), notNull(true) {}
    AttributeDescriptor(std::string name, TypeTag type, unsigned len = ~0, bool notNull = false)
      : name(name), type(type), len(len), notNull(notNull) {}
  };

  /**
   * represents the schema of one relation
   */
  struct RelationSchema {
    RelationSchema() {}

    /*
     * loads a schema from a Record
     */
    explicit RelationSchema(Record& record);

    RelationSchema(
        const std::string& name,
        std::vector<AttributeDescriptor> attributes = {},
        std::vector<unsigned> primaryKeyIdcs = {},
        uint32_t segmentID = 0,
        uint64_t size = 0
      )
      : name(name),
        attributes(attributes),
        primaryKey(primaryKeyIdcs),
        segmentID(segmentID),
        size(size)
      {}

    /*
     * serializes a schema into a Record
     */
    Record serializeToRecord() const;

    std::string name;
    std::vector<AttributeDescriptor> attributes;
    std::vector<unsigned> primaryKey;
    uint32_t segmentID;
    uint64_t size; //[in pages]
  };

  std::ostream& operator<< (std::ostream& out, const TypeTag& type);
  std::ostream& operator<< (std::ostream& out, const AttributeDescriptor& attr);
  std::ostream& operator<< (std::ostream& out, const RelationSchema& schema);

}

#endif
