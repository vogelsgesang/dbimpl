#ifndef _PRINTOPERATOR_H_
#define _PRINTOPERATOR_H_

#include <stdint.h>
#include <vector>
#include "operators/operator.h"
#include "operators/relation.h"

namespace dbImpl {
class PrintOperator: public Operator {
private:
  Operator* input;
  std::ostream& outstream;


public:
  PrintOperator(Operator* input, std::ostream& outstream) :
      input(input), outstream(outstream)

  {
  }
  ;

//pos merken?
  bool next() {
    if (input->next()) {
      std::vector<dbImpl::Register*> registers = input->getOutput();

      for (unsigned i = 0; i < registers.size(); i++) {
        outstream << *registers[i];
        if (i < (registers.size()-1)) {
          outstream << " ";
        }
      }
      outstream << std::endl;

      return true;
    }
    return false;

  }
  std::vector<Register*> getOutput();
  void open() {
    input->open();

  }
  void close() {
    input->close();
  }

};

}

#endif //PRINTOPERATOR_H
