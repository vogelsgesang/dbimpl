#ifndef _TABLESCAN_H_
#define _TABLESCAN_H_

#include <stdint.h>
#include <vector>
#include "operators/operator.h"
#include "operators/relation.h"

namespace dbImpl {
class TableScanOperator: public Operator {
private:
  Relation relation;

  uint64_t tid;
  unsigned limit;

  std::vector<Register> registers;
public:
  TableScanOperator(Relation rel) :
      relation(rel)

  {
  }
  ;

  //Reads the next tuple (if any)
  bool next() {
    if (tid >= limit) {
      return false;
    }
    std::vector<Register>& tuple = relation.get(tid);
    for (auto& cell : tuple) {
      registers.push_back(cell);
    }
    tid++;
    return true;

  }

  //returns the values of the current tuple.
  std::vector<Register*> getOutput() {
    std::vector<Register*> output;
    output.reserve(tid*relation.getNumAttributes());
    for(uint64_t i = 0; i < tid*relation.getNumAttributes(); i++){
      output.push_back(&registers[i]);

    }
    return output;
  }

  void open() {
    tid = 0;
    limit = relation.getNumTuples();
    registers.clear();
    registers.reserve(limit * relation.getNumAttributes());
  }
  void close(){
  }



};


}

#endif //TABLESCAN_H
