#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <string>
#include <ostream>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include "schema/relationSchema.h"

namespace dbImpl {

  class Register {
    public:
      Register() {}

      Register(const int intValue) {
          setInteger(intValue);
      };

      Register(const std::string& strValue) {
        setString(strValue);
      };

      Register(const Register& reg) {
        type = reg.type;
        switch(type) {
          case TypeTag::Integer:
            value.integer = reg.value.integer;
          case TypeTag::Char:
            value.str = reg.value.str;
          default:
            throw std::runtime_error("Unknown data type in register");
        }
      }

      Register& operator=(const Register& rhs) {
        type = rhs.type;
        switch(type) {
          case TypeTag::Integer:
            value.integer = rhs.value.integer;
          case TypeTag::Char:
            value.str = rhs.value.str;
          default:
            throw std::runtime_error("Unknown data type in register");
        }
        return *this;
      }

      TypeTag getType() const {
        return type;
      }

      int getInteger() const {
        if(type != TypeTag::Integer) {
          throw std::runtime_error("type mismatch while reading integer from register");
        }
        return value.integer;
      }

      void setInteger(const int i){
        type = TypeTag::Integer;
        value.integer = i;
      }

      std::string getString() const{
        if(type != TypeTag::Char) {
          throw std::runtime_error("type mismatch while reading string from register");
        }
        return value.str;
      }

      void setString(const std::string& s){
        type = TypeTag::Char;
        value.str = s;
      }

      bool operator<(Register r) const {
        if (type != r.type) {
          return type < r.type;
        }
        switch(type) {
          case TypeTag::Integer:
            return value.integer < r.value.integer;
          case TypeTag::Char:
            return value.str < r.value.str;
          default:
            throw std::runtime_error("Unknown data type in register");
        }
        //undefined
        return false;
      }

      bool operator==(const Register& r) const {
        if (type != r.type) {
          return false;
        }
        switch(type) {
          case TypeTag::Integer:
            return value.integer == r.value.integer;
          case TypeTag::Char:
            return value.str == r.value.str;
          default:
            throw std::runtime_error("Unknown data type in register");
        }
      }

      bool operator!=(const Register& r) const {
        return !(*this == r);
      }

      std::size_t hash() const {
        switch(type) {
          case TypeTag::Integer:
            std::hash<int> hash_int;
            return hash_int(value.integer);
          case TypeTag::Char:
            std::hash < std::string > hash_str;
            return hash_str(value.str);
          default:
            throw std::runtime_error("Unknown data type in register");
        }
      }

    private:
      union ValueHolder {
        int integer;
        std::string str;
        ValueHolder() {};
        ~ValueHolder() {};
      } value;

      TypeTag type;
  };

  std::ostream& operator<<(std::ostream& out, const Register& reg){
    switch(reg.getType()) {
      case TypeTag::Integer:
        out << reg.getInteger();
        break;
      case TypeTag::Char:
        out << reg.getString();
        break;
      default:
        out << "Undefined";
        break;
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
