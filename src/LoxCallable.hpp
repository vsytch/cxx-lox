#pragma once

namespace lox {
struct Interpreter;
}

#include "Interpreter.hpp"
#include "Object.hpp"

#include <vector>

namespace lox {
struct LoxCallable {
  virtual
  ~LoxCallable() = 0;

  virtual
  auto arity() -> size_t = 0;

  virtual
  auto call(Interpreter& interpreter, std::vector<Object>&& arguments) -> Object = 0;
};
}