import Interpreter;
import Lox;
import Parser;
import Scanner;

#include <fmt/core.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace lox {
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

auto main(int argc, char** argv) -> int {
  using namespace fmt;

  if (argc > 2) {
    print("Usage: cxx-lox [script]\n");
  } else if (argc == 2) {
    lox::runFile(argv[1]);
  } else {
    lox::runPrompt();
  }

  return 0;
}