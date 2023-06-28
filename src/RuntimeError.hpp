#pragma once

#include "TokenType.hpp"

#include <stdexcept>

namespace lox {
struct RuntimeError: std::runtime_error {
  Token token = {};

  RuntimeError(const Token& token, const std::string& what):
    std::runtime_error(what),
    token(token)
  {}
};
}
