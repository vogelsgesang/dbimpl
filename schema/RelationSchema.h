#ifndef _RELATION_SCHEMA_HPP_
#define _RELATION_SCHEMA_HPP_

#include <vector>
#include <string>
#include <cstdint> //uint64_t
#include <slottedPages/Record.hpp>

namespace dbImpl {
  class Record;

  /**
   * Types
   */
  namespace Types {
     enum class Tag : char {Integer, Char};
  }

  struct AttributeDescriptor {
     std::string name;
     Types::Tag type;
     unsigned len;
     bool notNull;
     AttributeDescriptor() : len(~0), notNull(true) {}
  };

  /**
   * represents the schema of one relation
   */
  struct RelationSchema {
    /*
     * loads a schmema from a Record
     */
    static RelationSchema loadFromRecord(Record& record);
    /*
     * serializes a schema into a Record
     */
    Record serializeToRecord();

    RelationSchema(const std::string& name) : name(name) {}
    

    std::string name;
    std::vector<AttributeDescriptor> attributes;
    std::vector<unsigned> primaryKey;
    uint64_t size; //[in pages]
  };

}

#endif
