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
  uint64_t curPosInOutput; //to prevent unnecessary write operations

  std::vector<Register> registers;
  std::vector<Register*> output;
public:
  TableScanOperator(Relation rel) :
      relation(rel)

  {
  }
  ;

  //Reads the next tuple (if any)
  bool next() {
    if (tid >= limit) {
r      return false;
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
    for(; i < tid*relation.getNumAttributes(); curPosInOutput++){
      output.push_back(&registers[curPosInOutput]);

    }
    return output;
  }

  void open() {
    tid = 0;
    limit = relation.getNumTuples();
    registers.clear();
    registers.reserve(limit * relation.getNumAttributes());
    output.reserve(limit * relation.getNumAttributes());
  }
  void close(){
  }



};


}

#endif //TABLESCAN_H
