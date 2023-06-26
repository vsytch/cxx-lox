#pragma once

#include "TokenType.hpp"

#include <cstddef>
#include <string>

namespace lox {
static bool hadError = false;

auto report(std::size_t line, const std::string& where, const std::string& message) -> void;

auto error(const Token& token, const std::string& message) -> void;

auto error(std::size_t line, const std::string& message) -> void;

auto run(const std::string& source) -> void;

auto runPrompt() -> void;

auto runFile(char* path) -> void;
}
