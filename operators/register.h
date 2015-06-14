#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <string>
#include <ostream>
#include <cstdint>
#include <cstring>

namespace dbImpl {

  class Register {
    public:
      uint8_t state; //undefined --> state = ~0

      Register()
        : state(~0) {};

      Register(const int intValue)
        : state(0) {
          setInteger(intValue);
      };

      Register(const std::string strValue)
        : state(1) {
        setString(strValue);
      };

      uint8_t getState() const {
        return state;
      }

      int getInteger() const{
        int intValue;
        std::memcpy(&intValue,value,sizeof(int));
        return intValue;
      }

      void setInteger(const int i){
        value = new uint8_t[sizeof(int)];
        std::memcpy(value, &i, sizeof(int));
        state = 0;
      }

      std::string getString() const{
        std::string stringValue(reinterpret_cast<char*>(value));
        return stringValue;
      }

      void setString(const std::string& s){
        value = new uint8_t[s.size()+1];
        strcpy(reinterpret_cast<char*>(value),s.c_str());
        state = 1;
      }

      bool operator<(Register r) const {
        if (state != r.state) {
          return false;
        }
        if (state == 0) {
          return getInteger() < r.getInteger();
        }
        if (state == 1) {
          return getString() < r.getString();
        }
        //undefined
        return false;
      }

      bool operator==(Register r) const {
        if (state != r.state) {
          return false;
        }
        if (state == 0) {
          return getInteger() == r.getInteger();
        }
        if (state == 1) {
          return getString() == r.getString();
        }
        //undefined
        return false;
      }

      bool operator!=(Register r) const {
        return !(*this == r);
      }

      std::size_t hash() const {
        if (state == 0) {
          std::hash<int> hash_int;
          return hash_int(getInteger());
        }
        if (state == 1) {
          std::hash < std::string > hash_str;
          return hash_str(getString());
        }
        throw "Cannot hash value. State is undefined";
      }

    private:
      uint8_t* value;
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
