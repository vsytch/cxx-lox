#pragma once

#include "Object.hpp"
#include "TokenType.hpp"

#include <boost/hana/functional/overload_linearly.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <memory>
#include <variant>

namespace lox {
struct Assign;
struct Binary;
struct Grouping;
struct Literal;
struct Unary;
struct Variable;
using Expr = std::variant<
  std::monostate,
  std::unique_ptr<Assign>,
  std::unique_ptr<Binary>,
  std::unique_ptr<Grouping>,
  std::unique_ptr<Literal>,
  std::unique_ptr<Unary>,
  std::unique_ptr<Variable>
>;

struct Assign {
  Token name;
  Expr value;
};

struct Binary {
  Expr left;
  Token op;
  Expr right;
};

struct Grouping {
  Expr expression;
};

struct Literal {
  Object value;
};

struct Unary {
  Token op;
  Expr right;
};

struct Variable {
  Token name;
};

struct Block;
struct Expression;
struct Print;
struct Var;
using Stmt = std::variant<
  std::monostate,
  std::unique_ptr<Block>,
  std::unique_ptr<Expression>,
  std::unique_ptr<Print>,
  std::unique_ptr<Var>
>;

struct Block {
  std::vector<Stmt> statements;
};

struct Expression {
  Expr expression;
};

struct Print {
  Expr expression;
};

struct Var {
  Token name;
  Expr initializer;
};
}

namespace fmt {
template<>
struct formatter<lox::Expr> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const lox::Expr& expression, FormatContext& ctx) const {
    using namespace boost::hana;
    using namespace lox;
    using namespace std;

    return format_to(ctx.out(), "{}", visit(overload_linearly(
      [](std::monostate) { return "nil"s; },
      [](const unique_ptr<Assign>& expr) { return fmt::format("(assign {} {})", expr->name, expr->value); },
      [](const unique_ptr<Binary>& expr) { return fmt::format("({} {} {})", expr->op.lexeme, expr->left, expr->right); },
      [](const unique_ptr<Grouping>& expr) { return fmt::format("(group {})", expr->expression); },
      [](const unique_ptr<Literal>& expr) { return fmt::format("{}", expr->value); },
      [](const unique_ptr<Unary>& expr) { return fmt::format("({} {})", expr->op.lexeme, expr->right); },
      [](const unique_ptr<Variable>& expr) { return fmt::format("(var {})", expr->name); }
    ), expression));
  }
};

template<>
struct formatter<lox::Stmt> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const lox::Stmt& statement, FormatContext& ctx) const {
    using namespace boost::hana;
    using namespace lox;
    using namespace std;

    return format_to(ctx.out(), "{}", visit(overload_linearly(
      [](std::monostate) { return "nil"s; },
      [](const unique_ptr<Block>& stmt) { return fmt::format("(eval {})", stmt->statements); },
      [](const unique_ptr<Expression>& stmt) { return fmt::format("(eval {})", stmt->expression); },
      [](const unique_ptr<Print>& stmt) { return fmt::format("(print {})", stmt->expression); },
      [](const unique_ptr<Var>& stmt) { return fmt::format("(declare {} {})", stmt->name, stmt->initializer); }
    ), statement));
  }
};
}
