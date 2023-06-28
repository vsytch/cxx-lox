#pragma once

#include "Ast.hpp"
#include "Lox.hpp"
#include "RuntimeError.hpp"

#include <boost/hana/functional/overload_linearly.hpp>
#include <fmt/format.h>

#include <memory>
#include <variant>

namespace lox {
inline auto isTruthy(const Object& object) -> bool {
  using namespace boost::hana;
  using namespace std;

  return visit(overload_linearly(
    [](std::monostate) { return false; },
    [](bool val) { return val; },
    [](auto&&) { return true; }
  ), object);
}

inline auto isEqual(const Object& left, const Object& right) -> bool {
  using namespace boost::hana;
  using namespace std;

  return visit(overload_linearly(
    [](std::monostate, std::monostate) { return true; },
    [](std::monostate, auto&&) { return false; },
    [](auto&& left, auto&& right) 
      requires std::equality_comparable_with<decltype(left), decltype(right)>
      { return std::equal_to{}(left, right); },
    [](auto&&, auto&&) { return false; }
  ), left, right);
}

inline auto checkNumberOperand(const Token& op, const Object& operand) -> void {
  using namespace boost::hana;
  using namespace std;

  return visit(overload_linearly(
    [](double) { return; },
    [op](auto&&) { throw RuntimeError{op, "Operands must be numbers."}; }
  ), operand);  
}

inline auto checkNumberOperands(const Token& op, const Object& left, const Object& right) -> void {
  using namespace boost::hana;
  using namespace std;

  return visit(overload_linearly(
    [](double, double) { return; },
    [op](auto&&, auto&&) { throw RuntimeError{op, "Operands must be numbers."}; }
  ), left, right);
}

inline auto evaluate(const Expr& expression) -> Object {
  using enum TokenType;
  using namespace boost::hana;
  using namespace std;

  return visit(overload_linearly(
    [](std::monostate) -> Object { return std::monostate{}; },
    [](const unique_ptr<Literal>& expr) -> Object { return expr->value; },
    [](const unique_ptr<Grouping>& expr) -> Object { return evaluate(expr->expression); },
    [](const unique_ptr<Unary>& expr) -> Object {
      auto&& right = evaluate(expr->right);

      switch (expr->op.type) {
        case BANG:
          return !isTruthy(right);
        case MINUS:
          return -get<double>(right);
      }

      return monostate{};
    },
    [](const unique_ptr<Binary>& expr) -> Object {
      auto&& left = evaluate(expr->left);
      auto&& right = evaluate(expr->right);

      switch (expr->op.type) {
        case GREATER:
          checkNumberOperands(expr->op, left, right);
          return get<double>(left) > get<double>(right);
        case GREATER_EQUAL:
          checkNumberOperands(expr->op, left, right);
          return get<double>(left) >= get<double>(right);
        case LESS:
          checkNumberOperands(expr->op, left, right);
          return get<double>(left) < get<double>(right);
        case LESS_EQUAL:
          checkNumberOperands(expr->op, left, right);
          return get<double>(left) <= get<double>(right);
        case BANG_EQUAL:
          return !isEqual(left, right);
        case EQUAL_EQUAL:
          return isEqual(left, right);
        case MINUS:
          checkNumberOperand(expr->op, right);
          return get<double>(left) - get<double>(right);
        case PLUS:
          return visit(overload_linearly(
            [](double left, double right) -> Object {
              return left + right;
            },
            [](const std::string& left, const std::string& right) -> Object {
              return left + right;              
            },
            [&op=expr->op](auto&&, auto&&) -> Object {
              throw RuntimeError{op, "Operands must be two numbers or two strings"};
              return std::monostate{};
            }
          ), left, right);
        case SLASH:
          checkNumberOperands(expr->op, left, right);
          return get<double>(left) / get<double>(right);
        case STAR:
          checkNumberOperands(expr->op, left, right);
          return get<double>(left) * get<double>(right);
      }

      return monostate{};
    }
  ), expression);
}

inline auto interpret(const Expr& expression) -> void {
  using namespace fmt;

  try {
    auto&& value = evaluate(expression);
    print("{}\n", value);
  } catch (const RuntimeError& err) {
    runtimeError(err);
  } 
}
}
