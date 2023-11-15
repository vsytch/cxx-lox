#pragma once

#include "Ast.hpp"
#include "Environment.hpp"
#include "Interpreter.hpp"
#include "LoxCallable.hpp"
#include "Object.hpp"
#include "Return.hpp"

namespace lox {
struct LoxFunction: public LoxCallable {
  Function* declaration;
  std::shared_ptr<Environment> closure;

  ~LoxFunction() override = default;

  auto arity() -> size_t override {
    return declaration->params.size();
  }

  auto call(Interpreter& interpreter, std::vector<Object>&& arguments) -> Object override {
    using namespace std;
    
    auto&& environment = make_shared<Environment>(closure);
    for (size_t i = 0; i < declaration->params.size(); i++) {
      environment->define(declaration->params[i].lexeme, std::move(arguments[i]));
    }

    try {
      interpreter.executeBlock(declaration->body, environment);
    } catch (const ReturnException& returnValue) {
      return returnValue.value;
    }
    return {};
  }
};
}