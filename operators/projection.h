#ifndef _PROJECTION_H_
#define _PROJECTION_H_

#include <vector>
#include "operators/operator.h"

namespace dbImpl {

  class ProjectionOperator: public Operator {
    private:
      Operator* input;
      std::vector<const Register*> output;
      std::vector<unsigned> regIDs;

    public:
      ProjectionOperator(Operator* input, const std::vector<unsigned>& regIDs)
        : input(input), regIDs(regIDs) {}

      //Reads the next tuple (if any)
      bool next() {
        if (input->next()) {
          std::vector<const Register*> tuple = input->getOutput();
          output.clear();
          for (unsigned i : regIDs) {
            output.push_back(tuple[i]);
          }
          return true;
        }
        return false;
      }

      //returns the values of the current tuple.
      std::vector<const Register*> getOutput() {
        return output;
      }

      void open() {
        input->open();
        output.reserve(regIDs.size());
      }

      void close() {
        input->close();
        output.clear();
      }
  };

}

#endif //PROJECTION_H
