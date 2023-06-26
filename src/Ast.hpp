#pragma once

#include "TokenType.hpp"
#include "Object.hpp"

#include <fmt/format.h>

#include <memory>
#include <variant>

namespace lox {
struct Binary;   using BinaryRef   = std::unique_ptr<Binary>;
struct Grouping; using GroupingRef = std::unique_ptr<Grouping>;
struct Literal;  using LiteralRef  = std::unique_ptr<Literal>;
struct Unary;    using UnaryRef    = std::unique_ptr<Unary>;
using Expr = std::variant<
  BinaryRef,
  GroupingRef,
  LiteralRef,
  UnaryRef
>;

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
    return format_to(ctx.out(), "{}", std::visit(cxx::match{
      [](const lox::BinaryRef& expr) { return fmt::format("({} {} {})", expr->op.lexeme, expr->left, expr->right); },
      [](const lox::GroupingRef& expr) { return fmt::format("(group {})", expr->expression); },
      [](const lox::LiteralRef& expr) { return fmt::format("{}", expr->value); },
      [](const lox::UnaryRef& expr) { return fmt::format("({} {})", expr->op.lexeme, expr->right); }
    }, expression));
  }
};
}
