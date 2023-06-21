#pragma once

#include <cstddef>
#include <string>

namespace lox {
static bool hadError = false;

auto report(std::size_t line, std::string where, std::string message) -> void;

auto error(std::size_t line, std::string message) -> void;

auto run(std::string source) -> void;

auto runPrompt() -> void;

auto runFile(char* path) -> void;
}
