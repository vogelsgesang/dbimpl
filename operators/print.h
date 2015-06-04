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
  unsigned curPos;

public:
  PrintOperator(Operator* input, std::ostream& outstream) :
      input(input), outstream(outstream), curPos(0)

  {
  }
  ;

//pos merken?
  bool next() {
    if (input->next()) {
      std::vector<dbImpl::Register*> registers = input->getOutput();
      //Start from CurrentPos. We do not want to print the first results again
      for (; curPos < registers.size(); curPos++) {
        outstream << *registers[curPos];
        if (curPos < (registers.size()-1)) {
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
    curPos = 0;
    input->open();

  }
  void close() {
    input->close();
  }

};

}

#endif //PRINTOPERATOR_H
