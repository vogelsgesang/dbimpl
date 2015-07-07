#include <iostream>
#include <vector>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include "expression.h"
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

  //create the expression and the function
  std::unique_ptr<Expression> expression(
    new Addition(
      new ArgumentUsage(1),
      new Subtraction(
        new ArgumentUsage(0),
        new ConstantValue(2)
      )
    )
  );
  ExpressionFunction function("f", std::move(expression));
  Function* jittedFunction = function.genCode(ctx, *module);

  jittedFunction->dump();

  engine->finalizeObject();
  std::vector<GenericValue> args(jittedFunction->getArgumentList().size());
  for(auto& value : args) {
    value.IntVal = APInt(64, 42, true);
  }
  GenericValue result = engine->runFunction(jittedFunction, args);
  outs() << "result: " << result.IntVal << "\n"; 

  return 0;
}
