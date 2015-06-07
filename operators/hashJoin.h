#ifndef _HASHJOIN_H_
#define _HASHJOIN_H_

#include <vector>
#include <unordered_map>
#include <stack>

#include "operators/operator.h"
#include "operators/register.h"

namespace dbImpl {


class HashJoinOperator: public Operator {
private:
  Operator* leftInput;
  Operator* rightInput;
  unsigned leftRegID;
  unsigned rightRegID;
  std::vector<Register*> output;

  std::stack<std::vector<Register*>> outputBuffer; //stores all tuples that are found in one(!) probe run

  std::unordered_multimap<Register, std::vector<Register>> hashTable;

  //build the input hashtable out of the leftInput
  void buildInput() {
    while (leftInput->next()) {
      std::vector<dbImpl::Register*> input(leftInput->getOutput());
      Register hashReg = *input[leftRegID];
      std::vector<Register> tuple;
      for (unsigned i = 0; i < input.size(); i++) {
        tuple.emplace_back(*input[i]);
      }
      hashTable.emplace(hashReg, std::move(tuple));
    }
  }

  //probe the input with one tuple of the rightInput
  //store all matching tuples into outputBuffer
  //return false iff no tuples were found
  bool probeInput() {
    std::vector<dbImpl::Register*> rightTuple = rightInput->getOutput();
    Register probeReg = *rightTuple[rightRegID];
    auto range = hashTable.equal_range(probeReg);
    for (auto it = range.first; it != range.second; ++it) {
      std::vector<Register*> tmp;
      //insert values of left table
      for (unsigned i = 0; i < it->second.size(); i++) {
        tmp.emplace_back(&it->second[i]);
      }
      //insert values of right table
      for (unsigned i = 0; i < rightTuple.size(); i++) {
        tmp.emplace_back(rightTuple[i]);
      }

      outputBuffer.emplace(std::move(tmp));

    }
    return !outputBuffer.empty();

  }

public:
  HashJoinOperator(Operator* leftInput, Operator* rightInput,
      unsigned leftRegId, unsigned rightRegId) :
      leftInput(leftInput), rightInput(rightInput), leftRegID(leftRegId), rightRegID(
          rightRegId) {
  }
  ;

  bool next() {
    //Check if there are tuples of the previous probe run (Only one tuple per next call should be produced)
    if (!outputBuffer.empty()) {
      output = outputBuffer.top();
      outputBuffer.pop();
      return true;
    }

    //Else: Get next probe element
    while (rightInput->next()) {
      if (probeInput()) {
        output = outputBuffer.top();
        outputBuffer.pop();
        return true;
      }
    }
    return false;
  }

  std::vector<Register*> getOutput() {
    return output;

  }

  void open() {
    leftInput->open();
    buildInput();
    rightInput->open();

  }
  void close() {
    leftInput->close();
    rightInput->close();
  }

};

}

#endif //_HASHJOIN_H_
