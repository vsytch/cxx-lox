#pragma once

#include "Object.hpp"
#include "TokenType.hpp"

#include <boost/hana/functional/overload.hpp>
#include <fmt/format.h>

#include <memory>
#include <variant>

namespace lox {
struct Binary;
struct Grouping;
struct Literal;
struct Unary;
using Expr = std::variant<
  std::monostate,
  std::unique_ptr<Binary>,
  std::unique_ptr<Grouping>,
  std::unique_ptr<Literal>,
  std::unique_ptr<Unary>
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
    using namespace boost::hana;
    using namespace lox;
    using namespace std;

    return format_to(ctx.out(), "{}", visit(overload(
      [](std::monostate) { return "nil"s; },
      [](const unique_ptr<Binary>& expr) { return fmt::format("({} {} {})", expr->op.lexeme, expr->left, expr->right); },
      [](const unique_ptr<Grouping>& expr) { return fmt::format("(group {})", expr->expression); },
      [](const unique_ptr<Literal>& expr) { return fmt::format("{}", expr->value); },
      [](const unique_ptr<Unary>& expr) { return fmt::format("({} {})", expr->op.lexeme, expr->right); }
    ), expression));
  }
};
}
