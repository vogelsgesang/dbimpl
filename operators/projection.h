#ifndef _PROJECTION_H_
#define _PROJECTION_H_

#include <vector>
#include "operators/operator.h"

/*
 * The Projection operator is initialized with an input operator and a
 list of register IDs (indexes into the register vector) it should project to
 */

namespace dbImpl {
class ProjectionOperator: public Operator {
private:
  Operator* input;
  std::vector<Register*> output;
  std::vector<unsigned> regIDs;
  uint64_t curRegID;

public:
  ProjectionOperator(Operator* input, std::vector<unsigned> regIDs) :
      input(input), regIDs(regIDs)

  {
  }
  ;

  //Reads the next tuple (if any)
  bool next() {
    if (input->next()) {
      std::vector<dbImpl::Register*> registers = input->getOutput();
      for (unsigned i : regIDs) {
        curRegID += i;
        output.push_back(registers[curRegID]);

      }
      curRegID = registers.size();
      return true;

    }
    return false;

  }

  //returns the values of the current tuple.
  std::vector<Register*> getOutput() {
    return output;

  }

  void open() {
    input->open();
    curRegID = 0;
    output.reserve(regIDs.size());
    output.clear();

  }
  void close() {
    input->close();
  }

};

}

#endif //PROJECTION_H
