#pragma once

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace lox {
enum class TokenType: std::uint8_t {
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  LOX_EOF,
};

struct Token {
  TokenType type = {};
  std::string lexeme = {};
  /*Object*/ void* literal = {};
  std::size_t line = {};

  operator std::string() const {
    using namespace fmt;
    using namespace magic_enum;

    return format("{} {} {}", enum_name(type), lexeme, "Object object");
  }
};
}

namespace fmt {
template<>
struct formatter<lox::Token> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const lox::Token& token, FormatContext& ctx) const {
    return format_to(ctx.out(), "{}", std::string(token));
  }
};
}
