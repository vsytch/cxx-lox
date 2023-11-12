#pragma once

#include "Ast.hpp"
#include "Lox.hpp"
#include "TokenType.hpp"

#include <cstddef>
#include <cstdio>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace lox {
struct Parser {
  struct ParseError{};

  std::vector<lox::Token> tokens = {};
  std::size_t current = {};

  auto parse() -> std::vector<Stmt> {
    using namespace std;

    auto&& statements = vector<Stmt>{};
    while (!isAtEnd()) {
      statements.push_back(declaration());
    }

    return statements;
  }

  auto expression() -> Expr {
    return assignment();
  }

  auto declaration() -> Stmt {
    using enum TokenType;
    using namespace std;

    try {
      if (match<VAR>()) return varDeclaration();
      return statement();
    } catch (const ParseError& err) {
      synchronize();
      return monostate{};
    }
  }

  auto statement() -> Stmt {
    using enum TokenType;
    using namespace std;

    if (match<FOR>()) return forStatement();
    if (match<IF>()) return ifStatement();
    if (match<PRINT>()) return printStatement();
    if (match<WHILE>()) return whileStatement();
    if (match<LEFT_BRACE>()) return make_unique<Block>(block());

    return expressionStatement();
  }

  auto forStatement() -> Stmt {
    using enum TokenType;
    using namespace std;

    consume(LEFT_PAREN, "Expect '(' after 'for'.");

    auto&& initializer = Stmt{};
    if (match<SEMICOLON>()) {
      initializer = monostate{};
    } else if (match<VAR>()) {
      initializer = varDeclaration();
    } else {
      initializer = expressionStatement();
    }

    auto&& condition = Expr{};
    if (!check(SEMICOLON)) {
      condition = expression();
    }
    consume(SEMICOLON, "Expect ';' after loop condition.");

    auto&& increment = Expr{};
    if (!check(RIGHT_PAREN)) {
      increment = expression();
    }
    consume(RIGHT_PAREN, "Expect ';' after for clauses.");
    
    auto&& body = statement();
    if (increment != Expr{monostate{}}) {
      auto&& tmp = vector<Stmt>{};
      tmp.emplace_back(std::move(body));
      tmp.emplace_back(make_unique<Expression>(std::move(increment)));
      body = make_unique<Block>(std::move(tmp));
    }

    if (condition == Expr{monostate{}}) condition = make_unique<Literal>(true);
    body = make_unique<While>(std::move(condition), std::move(body));

    if (initializer != Stmt{monostate{}}) {
      auto&& tmp = vector<Stmt>{};
      tmp.emplace_back(std::move(initializer));
      tmp.emplace_back(std::move(body));
      body = make_unique<Block>(std::move(tmp));
    }

    return body;
  }

  auto ifStatement() -> Stmt {
    using enum TokenType;
    using namespace std;

    consume(LEFT_PAREN, "Expect '( after 'if'.");
    auto&& condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after if condition.");

    auto&& thenBranch = statement();
    auto&& elseBranch = Stmt{};
    if (match<ELSE>()) {
      elseBranch = statement();
    }

    return make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
  }

  auto printStatement() -> Stmt {
    using enum TokenType;
    using namespace std;

    auto&& value = expression();
    consume(SEMICOLON, "Expect ';' after value.");
    return make_unique<Print>(std::move(value));
  }

  auto varDeclaration() -> Stmt {
    using enum TokenType;

    auto&& name = consume(IDENTIFIER, "Expect variable name.");

    auto&& initializer = Expr{std::monostate{}};
    if (match<EQUAL>()) {
      initializer = expression();
    }

    consume(SEMICOLON, "Expect ';' after variable declaration.");
    return make_unique<Var>(std::move(name), std::move(initializer));
  }

  auto whileStatement() -> Stmt {
    using enum TokenType;
    using namespace std;

    consume(LEFT_PAREN, "Expect '(' afer 'while'.");
    auto&& condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after condition.");
    auto&& body = statement();

    return make_unique<While>(std::move(condition), std::move(body));
  }

  auto expressionStatement() -> Stmt {
    using enum TokenType;
    using namespace std;
    
    auto&& expr = expression();
    consume(SEMICOLON, "Expect ';' after expression.");
    return make_unique<Expression>(std::move(expr));
  }

  auto block() -> std::vector<Stmt> {
    using enum TokenType;
    using namespace std;

    auto statements = vector<Stmt>{};
    while (!check(RIGHT_BRACE) && !isAtEnd()) {
      statements.emplace_back(declaration());
    }

    consume(RIGHT_BRACE, "Expect '}' after block.");
    return statements;
  }

  auto assignment() -> Expr {
    using enum TokenType;
    using namespace std;

    auto&& expr = disjunction();

    if (match<EQUAL>()) {
      auto&& equals = previous();
      auto&& value = assignment();

      if (auto&& var = get_if<std::unique_ptr<Variable>>(&expr)) {
        return make_unique<Assign>(std::move((*var)->name), std::move(value));
      }

      error(equals, "Invalid assignment target.");
    }

    return expr;
  }

  auto disjunction() -> Expr {
    using enum TokenType;
    using namespace std;

    auto&& expr = conjuction();

    while (match<OR>()) {
      auto&& op = previous();
      auto&& right = conjuction();
      expr = make_unique<Logical>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto conjuction() -> Expr {
    using enum TokenType;
    using namespace std;

    auto&& expr = equality();

    while (match<AND>()) {
      auto&& op = previous();
      auto&& right = equality();
      expr = make_unique<Logical>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto equality() -> Expr {
    using enum TokenType;

    auto&& expr = comparison();
    while (match<BANG_EQUAL, EQUAL_EQUAL>()) {
      auto&& op = previous();
      auto&& right = comparison();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto comparison() -> Expr {
    using enum TokenType;

    auto&& expr = term();
    while (match<GREATER, GREATER_EQUAL, LESS, LESS_EQUAL>()) {
      auto&& op = previous();
      auto&& right = term();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto term() -> Expr {
    using enum TokenType;

    auto&& expr = factor();
    while (match<MINUS, PLUS>()) {
      auto&& op = previous();
      auto&& right = factor();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto factor() -> Expr {
    using enum TokenType;

    auto&& expr = unary();
    while (match<SLASH, STAR>()) {
      auto&& op = previous();
      auto&& right = unary();
      expr = make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }

    return expr;
  }

  auto unary() -> Expr {
    using enum TokenType;

    if (match<BANG, MINUS>()) {
      auto&& op = previous();
      auto&& right = unary();
      return make_unique<Unary>(std::move(op), std::move(right));
    }

    return primary();
  }

  auto primary() -> Expr {
    using enum TokenType;
    using namespace std;

    if (match<FALSE>()) return make_unique<Literal>(false);
    if (match<TRUE>()) return make_unique<Literal>(true);
    if (match<NIL>()) return make_unique<Literal>(monostate{});

    if (match<NUMBER, STRING>()) {
      return make_unique<Literal>(std::move(previous().literal));
    }

    if (match<IDENTIFIER>()) {
      return make_unique<Variable>(previous());
    }

    if (match<LEFT_PAREN>()) {
      auto&& expr = expression();
      consume(RIGHT_PAREN, "Expect ')' after expression.");
      return make_unique<Grouping>(std::move(expr));
    }

    throw error(peek(), "Expect expression.");
  }

  template<TokenType... types>
  auto match() -> bool {
    if ((check(types) || ...)) {
      advance();
      return true;
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

  auto synchronize() -> void {
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
        default:
          break;
      }
    }
  
    advance();
  }
};
}
