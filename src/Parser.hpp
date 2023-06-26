#pragma once

#include "Ast.hpp"
#include "Lox.hpp"
#include "TokenType.hpp"

#include <cstddef>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace lox {
struct Parser {
  struct ParseError{};

  std::vector<lox::Token> tokens = {};
  std::size_t current = {};

  auto parse() -> Expr {
    using namespace std;

    try {
      return expression();
    } catch (const ParseError&) {
      return monostate{};
    }
  }

  auto expression() -> Expr {
    return equality();
  }

  auto equality() -> Expr {
    using enum TokenType;

    auto&& expr = comparison();
    while (match({BANG_EQUAL, EQUAL_EQUAL})) {
      auto&& op = previous();
      auto&& right = comparison();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto comparison() -> Expr {
    using enum TokenType;

    auto&& expr = term();
    while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
      auto&& op = previous();
      auto&& right = term();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto term() -> Expr {
    using enum TokenType;

    auto&& expr = factor();
    while (match({MINUS, PLUS})) {
      auto&& op = previous();
      auto&& right = factor();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto factor() -> Expr {
    using enum TokenType;

    auto&& expr = unary();
    while (match({SLASH, STAR})) {
      auto&& op = previous();
      auto&& right = unary();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto unary() -> Expr {
    using enum TokenType;

    if (match({BANG, MINUS})) {
      auto&& op = previous();
      auto&& right = unary();
      return make_unique<Unary>(std::move(op), std::move(right));
    }

    return primary();
  }

  auto primary() -> Expr {
    using enum TokenType;
    using namespace std;

    if (match({FALSE})) return make_unique<Literal>(false);
    if (match({TRUE})) return make_unique<Literal>(true);
    if (match({NIL})) return make_unique<Literal>(monostate{});

    if (match({NUMBER, STRING})) {
      return make_unique<Literal>(std::move(previous().literal));
    }

    if (match({LEFT_PAREN})) {
      auto&& expr = expression();
      consume(RIGHT_PAREN, "Expect ')' after expression.");
      return make_unique<Grouping>(std::move(expr));
    }

    throw error(peek(), "Expect expression.");
  }

  auto match(const std::vector<TokenType>& types) -> bool {
    for (auto&& type: types) {
      if (check(type)) {
        advance();
        return true;
      }
    }

    return false;
  }

  auto consume(const TokenType type, const std::string& message) -> Token {
    if (check(type)) return advance();

    throw error(peek(), message);
  }

  auto check(const TokenType type) -> bool {
    if (isAtEnd()) return false;
    return peek().type == type;
  }

  auto advance() -> Token {
    if (!isAtEnd()) current++;
    return previous();
  }

  auto isAtEnd() -> bool {
    using enum TokenType;

    return peek().type == LOX_EOF;
  }

  auto peek() -> Token {
    return tokens[current];
  }

  auto previous() -> Token {
    return tokens[current - 1];
  }

  auto error(const Token& token, const std::string& message) -> ParseError {
    ::lox::error(token, message);
    return {};
  }

  auto syncrhonize() -> void {
    using enum TokenType;

    advance();
    while(!isAtEnd()) {
      if (previous().type == SEMICOLON) return;

      switch(peek().type) {
        case CLASS:
        case FOR:
        case FUN:
        case IF:
        case PRINT:
        case RETURN:
        case VAR:
        case WHILE:
          return;
      }
    }
  
    advance();
  }
};
}
