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
      Register() : type(TypeTag::Invalid) {}

      Register(const int intValue)
        : Register() {
        setInteger(intValue);
      };

      Register(const std::string& strValue)
        : Register() {
        setString(strValue);
      };

      Register(const Register& rhs)
        : Register() {
        switch(rhs.type) {
          case TypeTag::Integer:
            setInteger(rhs.value.integer);
            break;
          case TypeTag::Char:
            setString(rhs.value.str);
            break;
          default:
            throw std::runtime_error("Unknown data type in register");
        }
      }

      ~Register() {
        deconstructValue();
      }

      Register& operator=(const Register& rhs) {
        switch(rhs.type) {
          case TypeTag::Integer:
            setInteger(rhs.value.integer);
            break;
          case TypeTag::Char:
            setString(rhs.value.str);
            break;
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
        if(type != TypeTag::Integer) {
          deconstructValue();
          type = TypeTag::Integer;
        }
        value.integer = i;
      }

      std::string getString() const{
        if(type != TypeTag::Char) {
          throw std::runtime_error("type mismatch while reading string from register");
        }
        return value.str;
      }

      void setString(const std::string& s){
        if(type != TypeTag::Char) {
          deconstructValue();
          type = TypeTag::Char;
        }
        new(&value.str) std::string(s);
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

      void deconstructValue() {
        switch(type) {
          case TypeTag::Integer:
            break;
          case TypeTag::Char:
            value.str.~basic_string(); //explicitely call the desctructor
            break;
          case TypeTag::Invalid:
            break;
          default:
            throw std::runtime_error("Unknown data type in register");
        }
      }
  };

  std::ostream& operator<<(std::ostream& out, const Register& reg);
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
