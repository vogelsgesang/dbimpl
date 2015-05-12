#ifndef _SQL_BOOL_HPP_
#define _SQL_BOOL_HPP_

#include <cstdint>

namespace dbImpl {

  class SqlBool {
    private:
      //private constructor
      //use the static factory functions instead
      SqlBool(bool value, bool valid)
        : value(value), valid(valid) {}

      bool value;
      bool valid;
    public:
      static SqlBool trueValue();
      static SqlBool falseValue();
      static SqlBool unknownValue();

      SqlBool(bool value) : SqlBool(value, true) {}

      bool isTrue();
      bool isFalse();
      bool isUnknown();

      SqlBool operator&& (const SqlBool& rhs) const;
      SqlBool operator|| (const SqlBool& rhs) const;
      SqlBool operator! () const;
  };

} //namespace dbImpl

#endif
