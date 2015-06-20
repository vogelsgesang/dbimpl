#ifndef _TABLESCAN_H_
#define _TABLESCAN_H_

#include <stdint.h>
#include <vector>
#include "operators/operator.h"
#include "operators/tupleDeserializer.h"
#include "slottedPages/spSegment.h"

namespace dbImpl {

  class TableScanOperator: public Operator {
    private:
      SPSegment& segment;
      TupleDeserializer deserialize;

      SPSegment::SlotIterator slotIterator;

      std::vector<Register> registers;
      std::vector<const Register*> output;

    public:
      TableScanOperator(SPSegment& segment, const RelationSchema& schema)
      : TableScanOperator(segment, extractTypes(schema)) {}

      TableScanOperator(SPSegment& segment, const std::vector<TypeTag>& types)
      : segment(segment), deserialize(types), slotIterator(segment.end()) {}

      //Reads the next tuple (if any)
      bool next() {
        if (slotIterator == segment.end()) {
          return false;
        } else {
          auto tuple = deserialize(*slotIterator);
          //we must copy the elements explicitely in order
          //to ensure that the memory addresses of the
          //elements do not change
          for(size_t i = 0; i < tuple.size(); i++) {
            registers[i] = tuple[i];
          }
          slotIterator++;
          return true;
        }
      }

      //returns the values of the current tuple.
      std::vector<const Register*> getOutput() {
        return output;
      }

      void open() {
        slotIterator = segment.begin();
        registers.resize(deserialize.getColumnTypes().size());
        output.reserve(registers.size());
        for(unsigned i = 0; i < registers.size(); i++){
          output.push_back(&registers[i]);
        }
      }

      void close(){
        slotIterator = segment.end();
      }
    private:
      static std::vector<TypeTag> extractTypes(const RelationSchema& schema) {
        std::vector<TypeTag> types;
        types.reserve(schema.attributes.size());
        for(auto attribute : schema.attributes) {
          types.push_back(attribute.type);
        }
        return types;
      }
  };

}

#endif //TABLESCAN_H
