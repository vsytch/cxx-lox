#pragma once

#include "Ast.hpp"
#include "Environment.hpp"
#include "Lox.hpp"
#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include "Object.hpp"
#include "Return.hpp"
#include "RuntimeError.hpp"

#include <boost/hana/functional/overload_linearly.hpp>
#include <fmt/format.h>

#include <memory>
#include <variant>

namespace lox {
struct Interpreter {
  std::shared_ptr<Environment> globals = std::make_shared<Environment>();
  std::shared_ptr<Environment> environment = globals;

  auto isTruthy(const Object& object) -> bool {
    using namespace boost::hana;
    using namespace std;

    return visit(overload_linearly(
      [](std::monostate) { return false; },
      [](bool val) { return val; },
      [](auto&&) { return true; }
    ), object);
  }

  auto isEqual(const Object& left, const Object& right) -> bool {
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

  auto checkNumberOperand(const Token& op, const Object& operand) -> void {
    using namespace boost::hana;
    using namespace std;

    return visit(overload_linearly(
      [](double) { return; },
      [op](auto&&) { throw RuntimeError{op, "Operands must be numbers."}; }
    ), operand);  
  }

  auto checkNumberOperands(const Token& op, const Object& left, const Object& right) -> void {
    using namespace boost::hana;
    using namespace std;

    return visit(overload_linearly(
      [](double, double) { return; },
      [op](auto&&, auto&&) { throw RuntimeError{op, "Operands must be numbers."}; }
    ), left, right);
  }

  auto call(LoxCallable* callable, std::vector<Object>& arguments) -> Object {
    using namespace std;    
  }

  auto evaluate(const Expr& expression) -> Object {
    using enum TokenType;
    using namespace boost::hana;
    using namespace std;

    return visit(overload_linearly(
      [](std::monostate) -> Object { return std::monostate{}; },
      [this](const unique_ptr<Assign>& expr) -> Object {
        auto&& value = evaluate(expr->value);
        environment->assign(expr->name, value);
        return value;
      },
      [this](const unique_ptr<Binary>& expr) -> Object {
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
          default:
            break;
        }

        return monostate{};
      },
      [this](const unique_ptr<Call>& expr) -> Object {
        using namespace fmt;
        using namespace std;
        
        auto&& callee = evaluate(expr->callee);

        auto&& arguments = vector<Object>{};
        for (auto&& argument: expr->arguments) {
          arguments.emplace_back(evaluate(argument));
        }

        // auto&& function = get<unique_ptr<LoxCallable>>(callee);
        // if (!function) {
        //   throw RuntimeError(expr->paren, "Can only call functions and classes.");
        // }
        // if (arguments.size() != function->arity()) {
        //   throw RuntimeError(expr->paren, format("Expected {} arguments but got {}.", function->arity(), arguments.size()));
        // }

        // return function->call(*this, std::move(arguments));
      },
      [this](const unique_ptr<Grouping>& expr) -> Object { return evaluate(expr->expression); },
      [](const unique_ptr<Literal>& expr) -> Object { return expr->value; },
      [this](const unique_ptr<Logical>& expr) -> Object {
        auto&& left = evaluate(expr->left);

        if (expr->op.type == OR) {
          if (isTruthy(left)) return left;
        } else {
          if (!isTruthy(left)) return left;
        }

        return evaluate(expr->right);
      },
      [this](const unique_ptr<Unary>& expr) -> Object {
        auto&& right = evaluate(expr->right);

        switch (expr->op.type) {
          case BANG:
            return !isTruthy(right);
          case MINUS:
            return -get<double>(right);
          default:
            break;
        }

        return monostate{};
      },
      [this](const std::unique_ptr<Variable>& expr) -> Object {
        return environment->get(expr->name);
      }
    ), expression);
  }

  auto execute(const Stmt& statement) -> void {
    using enum TokenType;
    using namespace boost::hana;
    using namespace std;

    return visit(overload_linearly(
      [](std::monostate) { return; },
      [this](const unique_ptr<Block>& stmt) {
        executeBlock(stmt->statements, make_shared<Environment>(environment));
      },
      [this](const unique_ptr<Expression>& stmt) {
        evaluate(stmt->expression);
      },
      [this](const unique_ptr<Function>& stmt) {
        using namespace std;

        auto&& function = make_unique<LoxFunction>(stmt.get(), environment);
        environment->define(stmt->name.lexeme, std::move(function));
      },
      [this](const unique_ptr<IfStmt>& stmt) {
        if (isTruthy(evaluate(stmt->condition))) {
          execute(stmt->thenBranch);
        } else if (stmt->elseBranch != Stmt{monostate{}}) {
          execute(stmt->elseBranch);
        }
      },
      [this](const unique_ptr<Print>& stmt) {
        auto&& value = evaluate(stmt->expression);
        fmt::print("{}\n", value);
      },
      [this](const unique_ptr<Return>& stmt) {
        using namespace std;

        auto&& value = Object{};
        if (stmt->value != Expr{monostate{}}) value = evaluate(stmt->value);

        throw ReturnException{std::move(value)};
      },
      [this](const unique_ptr<Var>& stmt) {
        auto&& value = Object{};
        if (stmt->initializer != Expr{monostate{}}) {
          value = evaluate(stmt->initializer);
        }

        environment->define(stmt->name.lexeme, std::move(value));
      },
      [this](const unique_ptr<While>& stmt) {
        while (isTruthy(evaluate(stmt->condition))) {
          execute(stmt->body);
        }
      }
    ), statement);
  }

  auto executeBlock(const std::vector<Stmt>& statements, std::shared_ptr<Environment> next) -> void {
    using namespace std;

    auto&& previous = make_shared<Environment>(environment);
    try {
      environment = next;

      for (auto&& statement: statements) {
        execute(statement);
      }
    } catch (...) {}

    environment = previous;
  }

  auto interpret(const std::vector<Stmt>& statements) -> void {
    using namespace fmt;

    try {
      for (auto&& statement: statements) {
        execute(statement);
      }
    } catch (const RuntimeError& err) {
      runtimeError(err);
    }
  }
};
}
