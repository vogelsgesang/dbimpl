#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <string>
#include <functional>
#include <stdint.h>

namespace dbImpl {

class Register {
public:

  uint8_t state; //undefined --> state = ~0

  Register():
    state(~0)
  {};
  Register(const int intValue):
    state(0),
    intValue(intValue)
  {};
  Register(const std::string strValue):
    state(1),
    stringValue(strValue)
  {};

  uint8_t getState() const {
    return state;
  }
  int getInteger() const{
    return intValue;
  }
  void setInteger(const int i){
    intValue = i;
    state = 0;
  }
  std::string getString() const{
    return stringValue;
  }

  void setString(const std::string& s){
    stringValue = s;
    state = 1;
  }

  bool operator<(Register r) const {
    if (state != r.state) {
      return false;
    }
    if (state == 0) {
      return intValue < r.getInteger();
    }
    if (state == 1) {
      return stringValue < r.getString();
    }
    //undefined
    return false;
  }
  bool operator==(Register r) const {
    if (state != r.state) {
      return false;
    }
    if (state == 0) {
      return intValue == r.getInteger();
    }
    if (state == 1) {
      return stringValue == r.getString();
    }
    //undefined
    return false;
  }

  std::size_t hash() const {
    if (state == 0) {
      std::hash<int> hash_int;
      return hash_int(intValue);
    }
    if (state == 1) {
      std::hash < std::string > hash_str;
      return hash_str(stringValue);
    }
    throw "Cannot hash value. State is undefined";
  }

private:
  int intValue; // state = 0
  std::string stringValue; // state = 1

};


std::ostream& operator<<(std::ostream& out, const Register& reg){
  if(reg.getState() == 0) {
    out << reg.getInteger();
  } else if(reg.getState() == 1){
    out << reg.getString();
  }
  else{
    out << "Undefined";
  }
  return out;
}



}
namespace std {

  template <>
  struct hash<dbImpl::Register>
  {
    std::size_t operator()(const dbImpl::Register& reg) const
    {
      return reg.hash();
    }
  };
}


#endif //REGISTER_H
