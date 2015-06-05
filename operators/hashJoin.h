#ifndef _HASHJOIN_H_
#define _HASHJOIN_H_

#include <vector>
#include <unordered_map>
#include <stack>

#include "operators/operator.h"
#include "operators/register.h"

namespace dbImpl {

/*
 * The Hash
 Join operator is initialized with two input operators, and two register IDs. One ID is
 from the left side and one is from the right side.
 */
class HashJoinOperator: public Operator {
private:
  Operator* leftInput;
  Operator* rightInput;
  unsigned leftRegID;
  unsigned rightRegID;
  std::vector<Register*> output;

  std::stack<std::vector<Register*>> outputBuffer; //Necessary since there could be more than one equal key

  std::unordered_multimap<Register, std::vector<Register>> hashTable;

  void buildInput() {
    while (leftInput->next()) {
      std::vector<dbImpl::Register*> input(leftInput->getOutput());
      Register hashReg = *input[leftRegID];
      std::vector<Register> tuple;
      for (unsigned i = 0; i < input.size(); i++) {
        tuple.emplace_back(*input[i]);
      }
      hashTable.emplace(hashReg, tuple);
    }
  }
  bool probeInput() {
    std::vector<dbImpl::Register*> tuple = rightInput->getOutput();
    Register probeReg = *tuple[rightRegID];
    auto range = hashTable.equal_range(probeReg);
    for (auto it = range.first; it != range.second; ++it) {
      std::vector<Register*> tmp;
      for (unsigned i = 0; i < it->second.size(); i++) {
        tmp.emplace_back(&it->second[i]);
      }
      outputBuffer.emplace(tmp);
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
