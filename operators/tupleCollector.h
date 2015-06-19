#ifndef _COLLECTOR_H_
#define _COLLECTOR_H_

#include <vector>
#include "operators/operator.h"
#include "operators/register.h"

namespace dbImpl {

  class TupleCollector {
    private:
      Operator* input;

    public:
      TupleCollector(Operator* input)
        : input(input) {}

      std::vector<std::vector<Register>> collect() {
        input->open();
        std::vector<std::vector<Register>> collectedTuples;
        while(input->next()) {
          std::vector<Register> tuple;
          for(auto element : input->getOutput()) {
            tuple.push_back(*element);
          }
          collectedTuples.push_back(tuple);
        }
        input->close();
        return collectedTuples;
      }
  };

}

#endif
