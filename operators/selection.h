#ifndef _SELECTION_H_
#define _SELECTION_H_

#include <vector>
#include "operators/operator.h"

namespace dbImpl {

  /*
   * The Selection operator is initialized with an input operator, a register ID and a constant.
   */
  class SelectionOperator: public Operator {
    private:
      Operator* input;
      std::vector<const Register*> output;
      unsigned attID;
      Register c;

    public:
      SelectionOperator(Operator* input, unsigned attID, Register c)
        : input(input), attID(attID), c(c) {}

      bool next() {
        while (input->next()) {
          std::vector<const Register*> tuple = input->getOutput();
          if (*tuple[attID] == c) {
            output = tuple;
            return true;
          }
        }
        return false;
      }

      //returns the values of the current tuple.
      std::vector<const Register*> getOutput() {
        return output;
      }

      void open() {
        input->open();
      }

      void close() {
        input->close();
      }
  };

}
#endif //SELECTION_H
