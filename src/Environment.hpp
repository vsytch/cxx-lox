#pragma once

#include "Object.hpp"
#include "RuntimeError.hpp"
#include "TokenType.hpp"

#include <fmt/format.h>

#include <string>
#include <unordered_map>

namespace lox {
struct Environment {
  std::shared_ptr<Environment> enclosing;
  std::unordered_map<std::string, Object> values;

  auto get(const Token& name) -> Object {
    using namespace fmt;

    if (values.contains(name.lexeme)) {
      return values[name.lexeme];
    }

    if (enclosing) return enclosing->get(name);

    throw RuntimeError{name, format("Undefined variable '{}'.", name.lexeme)};
  }
  
  auto assign(const Token& name, const Object& value) -> void {
    using namespace fmt;

    if (values.contains(name.lexeme)) {
      values[name.lexeme] = value;
      return;
    }

    if (enclosing) {
      enclosing->assign(name, value);
      return;
    }

    throw RuntimeError{name, format("Undefined variable {}.", name.lexeme)};
  }

  auto define(const std::string& name, const Object& value) -> void {
    values.insert({name, value});
  }
};
}
