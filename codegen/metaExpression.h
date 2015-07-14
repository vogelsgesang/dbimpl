#ifndef _CODE_GEN_METAEXPRESSION_H_
#define _CODE_GEN_METAEXPRESSION_H_

#include <cstdint>
#include <stdexcept>
#include <memory>
#include "expression.h"

namespace codegen {

  //This class helps to build expressions by providing the common math operators
  class MetaExpression {
    public:
      explicit MetaExpression(std::shared_ptr<Expression> expression)
        : currentExpression(expression) {}

      //provide access to the wrapped Expression
      //(implicitely callable)
      operator std::shared_ptr<Expression>() { return currentExpression; }

      //addition, substraction, multiplication, division with other expressions
      MetaExpression& operator+=(std::shared_ptr<Expression> rhs) { currentExpression = std::make_shared<Addition      >(currentExpression, rhs); return *this; }
      MetaExpression& operator-=(std::shared_ptr<Expression> rhs) { currentExpression = std::make_shared<Subtraction   >(currentExpression, rhs); return *this; }
      MetaExpression& operator*=(std::shared_ptr<Expression> rhs) { currentExpression = std::make_shared<Multiplication>(currentExpression, rhs); return *this; }
      MetaExpression& operator/=(std::shared_ptr<Expression> rhs) { currentExpression = std::make_shared<Division      >(currentExpression, rhs); return *this; }

      MetaExpression operator+(std::shared_ptr<Expression> rhs) { MetaExpression ret(*this); ret += rhs; return ret; }
      MetaExpression operator-(std::shared_ptr<Expression> rhs) { MetaExpression ret(*this); ret -= rhs; return ret; }
      MetaExpression operator*(std::shared_ptr<Expression> rhs) { MetaExpression ret(*this); ret *= rhs; return ret; }
      MetaExpression operator/(std::shared_ptr<Expression> rhs) { MetaExpression ret(*this); ret /= rhs; return ret; }

      //addition, substraction, multiplication, division with constant values
      MetaExpression& operator+=(int64_t rhs) { return (*this += std::make_shared<ConstantValue>(rhs)); }
      MetaExpression& operator-=(int64_t rhs) { return (*this -= std::make_shared<ConstantValue>(rhs)); }
      MetaExpression& operator*=(int64_t rhs) { return (*this *= std::make_shared<ConstantValue>(rhs)); }
      MetaExpression& operator/=(int64_t rhs) { return (*this /= std::make_shared<ConstantValue>(rhs)); }

      MetaExpression operator+(int64_t rhs) { MetaExpression ret(*this); ret += rhs; return ret; }
      MetaExpression operator-(int64_t rhs) { MetaExpression ret(*this); ret -= rhs; return ret; }
      MetaExpression operator*(int64_t rhs) { MetaExpression ret(*this); ret *= rhs; return ret; }
      MetaExpression operator/(int64_t rhs) { MetaExpression ret(*this); ret /= rhs; return ret; }

      //preincrement
      MetaExpression& operator++() { *this += 1; return *this; }
      //predecrement
      MetaExpression& operator--() { *this -= 1; return *this; }
      //postdecrement
      MetaExpression  operator++(int) { MetaExpression copy(*this); ++*this; return copy; }
      //postincrement
      MetaExpression  operator--(int) { MetaExpression copy(*this); --*this; return copy; }

    private:
      std::shared_ptr<Expression> currentExpression;
  };

  //the operators must be defined with the opposite order too
  MetaExpression operator+(std::shared_ptr<Expression> lhs, MetaExpression rhs) { return rhs + lhs; }
  MetaExpression operator-(std::shared_ptr<Expression> lhs, MetaExpression rhs) { return rhs - lhs; }
  MetaExpression operator*(std::shared_ptr<Expression> lhs, MetaExpression rhs) { return rhs * lhs; }
  MetaExpression operator/(std::shared_ptr<Expression> lhs, MetaExpression rhs) { return rhs / lhs; }
  MetaExpression operator+(int64_t lhs, MetaExpression rhs) { return rhs + lhs; }
  MetaExpression operator-(int64_t lhs, MetaExpression rhs) { return rhs - lhs; }
  MetaExpression operator*(int64_t lhs, MetaExpression rhs) { return rhs * lhs; }
  MetaExpression operator/(int64_t lhs, MetaExpression rhs) { return rhs / lhs; }

}

#endif
