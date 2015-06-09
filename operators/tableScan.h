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
      return false;
    }
    std::vector<Register>& tuple = relation.get(tid);
    registers.clear();
    for (auto& cell : tuple) {
      registers.push_back(cell);
    }
    tid++;
    return true;

  }

  //returns the values of the current tuple.
  std::vector<Register*> getOutput() {
    return output;
  }

  void open() {
    tid = 0;
    limit = relation.getNumTuples();
    registers.resize(relation.getNumAttributes());
    output.reserve(relation.getNumAttributes());
    for(unsigned i = 0; i < registers.size(); i++){
      output.push_back(&registers[i]);
    }

  }
  void close(){
  }



};


}

#endif //TABLESCAN_H
