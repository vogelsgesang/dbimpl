#include "operators/register.h"

namespace dbImpl {

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
