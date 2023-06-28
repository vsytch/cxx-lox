#pragma once

#include "TokenType.hpp"
#include "RuntimeError.hpp"

#include <cstddef>
#include <string>

namespace lox {
static bool hadError = false;
static bool hadRuntimeError = false;

auto report(std::size_t line, const std::string& where, const std::string& message) -> void;

auto error(const Token& token, const std::string& message) -> void;

auto error(std::size_t line, const std::string& message) -> void;

auto runtimeError(const RuntimeError& error) -> void;

auto run(const std::string& source) -> void;

auto runPrompt() -> void;

auto runFile(char* path) -> void;
}
