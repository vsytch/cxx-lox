#include "Ast.hpp"
#include "Lox.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"
#include "TokenType.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <fstream>
#include <iostream>

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

auto run(const std::string& source) -> void {
  using namespace fmt;
  using namespace lox;

  auto&& scanner = Scanner{source};
  auto&& tokens = scanner.scanTokens();
  auto&& parser = Parser{tokens};
  auto&& expression = parser.parse();

  if (hadError) return;

  print("{}\n", expression);
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

  auto&& source = string{};
  fs >> source;

  run(source);
  if (hadError) {
    exit(65);
  }
}
}
