#ifndef _PROJECTION_H_
#define _PROJECTION_H_

#include <vector>
#include "operators/operator.h"


namespace dbImpl {

/*
 * The Projection operator is initialized with an input operator and a
 list of register IDs (indexes into the register vector) it should project to
 */
class ProjectionOperator: public Operator {
private:
  Operator* input;
  std::vector<Register*> output;
  std::vector<unsigned> regIDs;

public:
  ProjectionOperator(Operator* input, std::vector<unsigned> regIDs) :
      input(input), regIDs(regIDs)

  {
  }
  ;

  //Reads the next tuple (if any)
  bool next() {
    if (input->next()) {
      std::vector<dbImpl::Register*> tuple = input->getOutput();
      output.clear();
      for (unsigned i : regIDs) {
        output.push_back(tuple[i]);

      }
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
    output.reserve(regIDs.size());

  }
  void close() {
    input->close();
  }

};

}

#endif //PROJECTION_H
