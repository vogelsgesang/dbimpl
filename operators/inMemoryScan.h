#ifndef _IN_MEMORY_SCAN_H_
#define _IN_MEMORY_SCAN_H_

#include "operators/operator.h"

namespace dbImpl {

  class InMemoryScanOperator : public Operator {
    public:
      InMemoryScanOperator(const std::vector<std::vector<Register>>& tuples)
        : tuples(tuples) {}

      bool next() {
        if(tupleIter != tuples.end()) {
          for(size_t i = 0; i < tupleIter->size(); i++) {
            registers[i] = (*tupleIter)[i];
          }
          tupleIter++;
          return true;
        } else {
          return false;
        }
      }

      //returns the values of the current tuple.
      std::vector<const Register*> getOutput() {
        return output;
      }

      void open() {
        if(tuples.size() > 0) {
          registers.resize(tuples[0].size());
          output.reserve(tuples[0].size());
          for(size_t i = 0; i < registers.size(); i++) {
            output.push_back(&registers[i]);
          }
        }
        tupleIter = tuples.begin();
      }

      void close() {
        registers.clear();
        output.clear();
      }

    private:
      std::vector<const Register*> output;
      std::vector<Register> registers;
      std::vector<std::vector<Register>> tuples;
      std::vector<std::vector<Register>>::const_iterator tupleIter;
  };

}

#endif
