#ifndef _PRINTOPERATOR_H_
#define _PRINTOPERATOR_H_

#include <cstdint>
#include <vector>
#include <ostream>
#include "operators/operator.h"
#include "operators/register.h"

namespace dbImpl {

  class PrintOperator: public Operator {
    private:
      Operator* input;
      std::ostream& outstream;

    public:
      PrintOperator(Operator* input, std::ostream& outstream)
        : input(input), outstream(outstream) {}

      bool next() {
        if (input->next()) {
          std::vector<const Register*> registers = input->getOutput();
          for (unsigned i = 0; i < registers.size(); i++) {
            outstream << *registers[i];
            if (i < registers.size() - 1) {
              outstream << " | ";
            }
          }
          outstream << std::endl;
          return true;
        }
        return false;
      }

      std::vector<const Register*> getOutput(){
        return input->getOutput();
      }

      void open() {
        input->open();
      }

      void close() {
        input->close();
      }
  };
}

#endif //PRINTOPERATOR_H
