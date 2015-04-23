#include "logic/sqlBool.h"
#include <cstdint>

namespace dbImpl {

  SqlBool SqlBool::trueValue() {
    return SqlBool(true);
  }

  SqlBool SqlBool::falseValue() {
    return SqlBool(false);
  }

  SqlBool SqlBool::unknownValue() {
    return SqlBool(false, false);
  } 


  bool SqlBool::isTrue() {
    return valid && value;
  }

  bool SqlBool::isFalse() {
    return valid && !value;
  }

  bool SqlBool::isUnknown() {
    return !valid;
  }


  SqlBool SqlBool::operator! () const {
    return SqlBool(!this->value, this->valid);
  }

  SqlBool SqlBool::operator&& (const SqlBool& rhs) const {
    return SqlBool(this->value && rhs.value, this->valid && rhs.valid);
  }

  SqlBool SqlBool::operator|| (const SqlBool& rhs) const {
    return SqlBool(this->value || rhs.value, this->valid && rhs.valid);
  }

}
