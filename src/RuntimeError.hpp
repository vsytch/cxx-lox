#pragma once

#include "TokenType.hpp"

#include <stdexcept>
#include <utility>

namespace lox {
struct RuntimeError: std::runtime_error {
  Token token = {};

  RuntimeError(Token token, const std::string& what):
    std::runtime_error(what),
    token(std::move(token))
  {}
};
}
