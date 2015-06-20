#ifndef _TUPLE_DESERIALIZER_H_
#define _TUPLE_DESERIALIZER_H_

#include <cstdint>
#include <vector>
#include "slottedPages/record.h"
#include "operators/register.h"

namespace dbImpl {

  class TupleDeserializer {
    private:
      std::vector<TypeTag> columnTypes;
    public:
      TupleDeserializer(std::vector<TypeTag> columnTypes)
        : columnTypes(columnTypes) {}

      std::vector<Register> operator()(const Record& rec) {
        std::vector<Register> values;
        values.reserve(columnTypes.size());
        const uint8_t* currPos = rec.getData();
        const uint8_t* endPos = currPos + rec.getLen();
        for(auto& type : columnTypes) {
          switch(type) {
            case TypeTag::Integer:
              if(currPos + sizeof(int) > endPos) {
                throw std::runtime_error("Read out of record's bounds");
              }
              values.push_back(Register(*reinterpret_cast<const int*>(currPos)));
              currPos += sizeof(int);
              break;
            case TypeTag::Char:
              { //these brackets are neccessary for scoping
                if(currPos + sizeof(size_t) > endPos) {
                  throw std::runtime_error("Read out of record's bounds");
                }
                size_t stringSize = *reinterpret_cast<const size_t*>(currPos);
                currPos += sizeof(size_t);
                if(currPos + stringSize > endPos) {
                  throw std::runtime_error("Read out of record's bounds");
                }
                values.push_back(Register(std::string(reinterpret_cast<const char*>(currPos), stringSize)));
                currPos += stringSize;
                break;
              }
            default:
              throw std::runtime_error("Unknown data type in register");
          }
        }
        if(currPos != endPos) {
          throw std::runtime_error("Record's data is not completely parsed into tuple");
        }
        return values;
      }
  };

}

#endif
