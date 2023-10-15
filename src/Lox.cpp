#include "Interpreter.hpp"
#include "Lox.hpp"
#include "Parser.hpp"
#include "RuntimeError.hpp"
#include "Scanner.hpp"
#include "TokenType.hpp"

#include <fmt/core.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace lox {
auto report(std::size_t line, const std::string& where, const std::string& message) -> void {
  using namespace fmt;

  print("[line {}] Error {}: {}\n", line, where, message);
  hadError = true;
}

auto error(const Token& token, const std::string& message) -> void {
  using enum TokenType;

  if (token.type == LOX_EOF) {
    report(token.line, " at end", message);
  } else {
    report(token.line, " at '" + token.lexeme + "'", message);
  }
}

auto error(std::size_t line, const std::string& message) -> void {
  report(line, "", message);
}

auto runtimeError(const RuntimeError& error) -> void {
  using namespace fmt;

  print("{} \n[line {} ]", error.what(), error.token.line);
  hadRuntimeError = true;
}

auto run(const std::string& source) -> void {
  using namespace fmt;

  auto&& scanner = Scanner{source};
  auto&& tokens = scanner.scanTokens();
  auto&& parser = Parser{tokens};
  auto&& statements = parser.parse();

  if (hadError) return;

  print("{}\n", statements);

  auto&& interpreter = Interpreter{};
  interpreter.interpret(statements);
}

auto runPrompt() -> void {
  using namespace fmt;
  using namespace std;

  for (;;) {
    print("> ");

    auto&& line = string{};
    getline(cin, line);
    if (empty(line)) break;

    run(line);
    if (hadError) {
      exit(65);
    }
  }
}

auto runFile(char* path) -> void {
  using namespace std;

  auto&& fs = ifstream{};
  fs.open(path, ios::in);
  if (!fs.is_open()) {
    return;
  }

  auto&& strStream = stringstream{};
  strStream << fs.rdbuf();
  auto&& source = strStream.str();

  run(source);
  if (hadError) exit(65);
  if (hadRuntimeError) exit(70);
}
}
