#ifndef _CODE_GEN_EXPRESSION_FUNCTION_H_
#define _CODE_GEN_EXPRESSION_FUNCTION_H_

#include <string>
#include "expression.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>

namespace codegen {

  using namespace llvm;

  //represents a function which evaluates an expression and returns its result
  class ExpressionFunction {
    public:
      ExpressionFunction(std::string functionName, std::shared_ptr<Expression> expression)
        : functionName(functionName), expression(std::move(expression)) {}


      virtual Function* genCode(LLVMContext& ctx, Module& module) {
        //create the function's signature
        unsigned short argumentCount = expression->argumentCount();
        std::vector<Type*> paramTypes(argumentCount, Type::getInt64Ty(ctx));
        FunctionType *functionType = FunctionType::get(Type::getInt64Ty(ctx), paramTypes, false);

        //create the function itself
        Function *function = Function::Create(functionType, Function::ExternalLinkage, functionName, &module);

        //name the arguments and store them in a vector
        std::vector<Argument*> arguments;
        for(auto currArg = function->arg_begin(); currArg != function->arg_end(); currArg++) {
          currArg->setName("param");
          arguments.push_back(currArg);
        }

        //generate the actual code
        BasicBlock* bb = BasicBlock::Create(getGlobalContext(), "entry", function);
        IRBuilder<> builder(bb);
        Value* returnValue = expression->genCode(module, ctx, builder, arguments);
        builder.CreateRet(returnValue);
        verifyFunction(*function);

        return function;
      }

    private:
      std::string functionName;
      std::shared_ptr<Expression> expression;
  };

}

#endif
