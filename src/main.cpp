#include "Lox.hpp"

#include <fmt/core.h>

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