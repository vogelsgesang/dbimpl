#include <iostream>
#include <vector>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include "expression.h"
#include "metaExpression.h"
#include "expressionFunction.h"

using namespace codegen;

int main(int, char**) {
  LLVMContext ctx;
  Module* module;
  std::unique_ptr<ExecutionEngine> engine;

  //create the engine
  InitializeNativeTarget();
  {
    std::unique_ptr<Module> ownedModule(new Module("Jitted code", ctx));
    module = ownedModule.get();
    std::string errorString;
    engine = std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(ownedModule))
      .setErrorStr(&errorString)
      .setEngineKind(EngineKind::Interpreter)
      .create());
    if(!engine) {
      std::cerr << "unable to create execution engine" << std::endl;
      std::cerr << "  " << errorString << std::endl;
      exit(1);
    }
  }

  //directly create an expression and execute it
  {
    //create the expression and the function
    std::shared_ptr<Expression> expression(
      std::make_shared<Addition>(
        std::make_shared<ArgumentUsage>(1),
        std::make_shared<Subtraction>(
          std::make_shared<ArgumentUsage>(0),
          std::make_shared<ConstantValue>(2)
        )
      )
    );
    ExpressionFunction function("f", expression);
    Function* jittedFunction = function.genCode(ctx, *module);

    //dump its code
    jittedFunction->dump();

    //build it and execute it.
    engine->finalizeObject();
    //fill the vector containing the parameters
    std::vector<GenericValue> args(jittedFunction->getArgumentList().size());
    for(auto& value : args) {
      value.IntVal = APInt(64, 42, true);
    }
    //call it and print its results
    GenericValue result = engine->runFunction(jittedFunction, args);
    outs() << "result: " << result.IntVal << "\n"; 
  }

  { //test the MetaExpression class
    //Use metaexpression in order to build an Expression
    MetaExpression polynomialExpression(std::make_shared<ConstantValue>(0));
    //                                x^4, x^3, x^2, x^1, x^0
    std::vector<int64_t> coefficients{1  , 0  , 3  , 4  , 1  };
    std::shared_ptr<Expression> x = std::make_shared<ArgumentUsage>(0);
    for(auto coefficient : coefficients) {
      polynomialExpression = coefficient + x*polynomialExpression;
    }

    ExpressionFunction polynomialFunc("polynomial", polynomialExpression);
    Function* jittedPolynomial = polynomialFunc.genCode(ctx, *module);

    //dump its code
    jittedPolynomial->dump();

    //build it
    engine->finalizeObject();
    //evaluate the polynomial for some test values
    std::vector<int64_t> polynomialTestValues{0, 1, 2, 5, 10, -1, -2};
    for(int64_t testValue : polynomialTestValues) {
      std::vector<GenericValue> args(1);
      args[0].IntVal = APInt(64, testValue, true);
      GenericValue result = engine->runFunction(jittedPolynomial, args);
      outs() << "polynomial(" << testValue << ") = " << result.IntVal << "\n"; 
    }
  }

  return 0;
}
