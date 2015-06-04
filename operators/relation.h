#ifndef _RELATION_H_
#define _RELATION_H_

#include <vector>
#include "operators/register.h"
#include <schema/relationSchema.h>

namespace dbImpl {

class Relation {

public:
  Relation() :
      numAttributes(0),
      numTuples(0)
{}
  ;

  std::vector<Register>& get(unsigned index) {
    if (numTuples <= index) {
      throw "Tuple does not exist";
    }
    return tuples[index];

  }

  void insert(std::vector<Register>&& tuple) {
    numTuples++;
    tuples.push_back(tuple);
  }

  void addAttribute(std::string name, TypeTag type){
    if(numTuples != 0) {
      throw "Cannot add attributes since the relation is not empty anymore";
    }
    Attribute newA(name,type);
    attributes.push_back(newA);
    numAttributes++;

  }
  unsigned getNumTuples(){
    return numTuples;
  }
  unsigned getNumAttributes(){
    return numAttributes;
  }

private:
  struct Attribute {
    std::string name;
    TypeTag type;
    Attribute(std::string name, TypeTag type):
      name(name),
      type(type)
    {};

  };

  std::vector<Attribute> attributes;
  std::vector<std::vector<Register>> tuples;
  unsigned numAttributes;
  unsigned numTuples;

};

}

#endif //RELATION_H
