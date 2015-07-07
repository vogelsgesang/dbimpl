#ifndef _CODE_GEN_EXPRESSION_H_
#define _CODE_GEN_EXPRESSION_H_

#include <cstdint>
#include <stdexcept>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/IRBuilder.h>

namespace codegen {

  using namespace llvm;

  class Expression {
    public:
      virtual ~Expression() {}
      //generates the code for this expression
      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) = 0;
      //returns the number of arguments which are taken by this function
      virtual unsigned short argumentCount() = 0;
  };


  class ConstantValue : public Expression {
    public:
      explicit ConstantValue(int64_t value)
        : value(value) {}

      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) {
        return ConstantInt::getSigned(Type::getInt64Ty(ctx), value);
      }

      virtual unsigned short argumentCount() {
        return 0;
      }
    protected:
      int64_t value;
  };


  class ArgumentUsage : public Expression {
    public:
      explicit ArgumentUsage(unsigned short paramNr)
        : paramNr(paramNr) {}

      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) {
        return arguments[paramNr];
      }

      virtual unsigned short argumentCount() {
        return paramNr + 1; //+1 since paramNr starts counting at zero but the return value starts counting at 1
      }
    protected:
      unsigned short paramNr;
  };


  class UnaryExpression : public Expression {
    public:
      explicit UnaryExpression(std::unique_ptr<Expression>&& operand)
        : operand(std::move(operand)) {}

      explicit UnaryExpression(Expression* operand)
        : operand(operand) {}

      virtual unsigned short argumentCount() {
        return operand->argumentCount();
      }

    protected:
      std::unique_ptr<Expression> operand;
  };


  class UnaryMinus : public UnaryExpression {
    public:
      using UnaryExpression::UnaryExpression; //inherit the constructor

      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) {
        return builder.CreateNeg(operand->genCode(mod, ctx, builder, arguments));
      }
  };


  class BinaryExpression : public Expression {
    public:
      BinaryExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

      BinaryExpression(Expression* lhs, Expression* rhs)
        : lhs(lhs), rhs(rhs) {}

      virtual unsigned short argumentCount() {
        return std::max(rhs->argumentCount(), lhs->argumentCount());
      }

    protected:
      std::unique_ptr<Expression> lhs;
      std::unique_ptr<Expression> rhs;
  };


  class Addition : public BinaryExpression {
    public:
      using BinaryExpression::BinaryExpression; //inherit the constructor

      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) {
        return builder.CreateAdd(lhs->genCode(mod, ctx, builder, arguments), rhs->genCode(mod, ctx, builder, arguments));
      }
  };


  class Subtraction : public BinaryExpression {
    public:
      using BinaryExpression::BinaryExpression; //inherit the constructor

      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) {
        return builder.CreateSub(lhs->genCode(mod, ctx, builder, arguments), rhs->genCode(mod, ctx, builder, arguments));
      }
  };


  class Multiplication : public BinaryExpression {
    public:
      using BinaryExpression::BinaryExpression; //inherit the constructor

      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) {
        return builder.CreateMul(lhs->genCode(mod, ctx, builder, arguments), rhs->genCode(mod, ctx, builder, arguments));
      }
  };


  class Division : public BinaryExpression {
    public:
      using BinaryExpression::BinaryExpression; //inherit the constructor

      virtual Value* genCode(Module& mod, LLVMContext& ctx, IRBuilder<>& builder, std::vector<Argument*>& arguments) {
        return builder.CreateSDiv(lhs->genCode(mod, ctx, builder, arguments), rhs->genCode(mod, ctx, builder, arguments));
      }
  };
}

#endif
