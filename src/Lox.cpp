#include "Ast.hpp"
#include "Lox.hpp"
#include "Scanner.hpp"
#include "TokenType.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <fstream>
#include <iostream>

namespace lox {
auto report(std::size_t line, std::string where, std::string message) -> void {
  using namespace fmt;

  print("[line {}] Error {}: {}", line, where, message);
  hadError = true;
}

auto error(std::size_t line, std::string message) -> void {
  report(line, "", message);
}

auto run(std::string source) -> void {
  using namespace fmt;
  using namespace lox;

  auto&& scanner = Scanner{source};
  auto&& tokens = scanner.scanTokens();

  print("{}\n", join(tokens, ", "));
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
