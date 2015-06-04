#ifndef _OPERATOR_H_
#define _OPERATOR_H_

#include <vector>
#include "operators/register.h"

namespace dbImpl {

class Operator {
public:

  //Open the operator
  virtual void open() = 0;

  //Produce the next tuple
  virtual bool next() = 0;

  //Get all produced values
  virtual std::vector<Register*> getOutput() = 0;

  //Close the operator
  virtual void close() = 0;

  virtual ~Operator();
};

}

#endif //OPERTOR_H
