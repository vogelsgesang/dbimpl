#ifndef _TUPLE_SERIALIZER_H_
#define _TUPLE_SERIALIZER_H_

#include <cstdint>
#include <vector>
#include "slottedPages/record.h"
#include "operators/register.h"

namespace dbImpl {

  class TupleSerializer {
    public:
      Record operator()(const std::vector<Register>& values) {
        //calculate needed memory size
        size_t recordSize = 0;
        for(auto& v : values) {
          switch(v.getType()) {
            case TypeTag::Integer:
              recordSize += sizeof(int);
              break;
            case TypeTag::Char:
              recordSize += sizeof(size_t); //for storing the string length
              recordSize += v.getString().size();
              break;
            default:
              throw std::runtime_error("Unknown data type in register");
          }
        }
        //create and fill the record
        Record rec(recordSize);
        uint8_t* currPos = rec.getData();
        for(auto& v : values) {
          switch(v.getType()) {
            case TypeTag::Integer:
              *reinterpret_cast<int*>(currPos) = v.getInteger();
              currPos += sizeof(int);
              break;
            case TypeTag::Char:
              *reinterpret_cast<size_t*>(currPos) = v.getString().size();
              currPos += sizeof(size_t);
              memcpy(currPos, v.getString().c_str(), v.getString().size());
              currPos += v.getString().size();
              break;
            default:
              throw std::runtime_error("Unknown data type in register");
          }
        }
        return rec;
      }
  };

}

#endif
