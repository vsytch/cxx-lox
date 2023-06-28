#pragma once

#include <charconv>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Lox.hpp"
#include "Object.hpp"
#include "TokenType.hpp"

namespace lox {
struct Scanner {
  inline static std::unordered_map<std::string_view, TokenType> keywords = {
    {"and", TokenType::AND},
    {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},
    {"false", TokenType::FALSE},
    {"for", TokenType::FOR},
    {"fun", TokenType::FUN},
    {"if", TokenType::IF},
    {"nil", TokenType::NIL},
    {"or", TokenType::OR},
    {"print", TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"true", TokenType::TRUE},
    {"var", TokenType::VAR},
    {"while", TokenType::WHILE},
  };

  std::string source = {};
  std::vector<Token> tokens = {};
  std::size_t start = {};
  std::size_t current = {};
  std::size_t line = {1};

  auto advance() -> char {
    return source[current++];
  }

  auto addToken(TokenType type, Object literal) -> void {
    auto&& text = source.substr(start, current - start);
    tokens.push_back(Token{type, text, literal, line});
  }

  auto addToken(TokenType type) -> void {
    addToken(type, std::monostate{});
  }

  auto scanToken() -> void {
    using enum TokenType;

    auto&& c = advance();
    switch (c) {
      case '(': addToken(LEFT_PAREN); break;
      case ')': addToken(RIGHT_PAREN); break;
      case '{': addToken(LEFT_BRACE); break;
      case '}': addToken(RIGHT_BRACE); break;
      case ',': addToken(COMMA); break;
      case '.': addToken(DOT); break;
      case '-': addToken(MINUS); break;
      case '+': addToken(PLUS); break;
      case ';': addToken(SEMICOLON); break;
      case '*': addToken(STAR); break;

      case '!':
        addToken(match('=') ? BANG_EQUAL : BANG);
        break;
      case '=':
        addToken(match('=') ? EQUAL_EQUAL : EQUAL);
        break;
      case '<':
        addToken(match('=') ? LESS_EQUAL : LESS);
        break;
      case '>':
        addToken(match('=') ? GREATER_EQUAL : GREATER);
        break;

      case '/':
        if (match('/')) {
          // A comment goes until the end of the line.
          while (peek() != '\n' && !isAtEnd()) advance();
        } else {
          addToken(SLASH);
        }
        break;

      case ' ':
      case '\r':
      case '\t':
        // Ignore whitespace.
        break;

      case '\n':
        line++;
        break;

      case '"': string(); break;

      default:
        if (isDigit(c)) {
          number();
        } else if (isAlpha(c)) {
          identifier();
        } else {
          lox::error(line, "Unexpected character.");
        }
        break;
    }
  }

  auto identifier() -> void {
    using enum TokenType;

    while (isAlphaNumeric(peek())) advance();

    auto&& text = source.substr(start, current - start);
    auto&& type = keywords.contains(text) ? keywords[text] : IDENTIFIER;

    addToken(type);
  }

  auto match(char expected) -> bool {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;

    current++;
    return true;
  }

  auto peek() -> char {
    if (isAtEnd()) return '\0';
    return source[current];
  }

  auto peekNext() -> char {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
  }

  auto isAlpha(char c) -> bool {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c == '_');
  }

  auto isAlphaNumeric(char c) -> bool {
    return isAlpha(c) || isDigit(c);
  }

  auto isDigit(char c) -> bool {
    return c >= '0' && c <= '9';
  }

  auto isAtEnd() -> bool {
    return current >= source.length();
  }

  auto scanTokens() -> std::vector<Token> {
    using enum TokenType;

    while (!isAtEnd()) {
      // We are at the beginning of the next lexeme.
      start = current;
      scanToken();
    }

    tokens.push_back(Token{LOX_EOF, "clrf", std::monostate{}, line});
    return tokens;
  }

  auto number() -> void {
    using enum TokenType;

    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
      // Consume the ".".
      advance();

      while (isDigit(peek())) advance();
    }

    auto&& value = double{};
    std::from_chars(source.c_str() + start, source.c_str() + current, value);
    addToken(NUMBER, value);
  }

  auto string() -> void {
    using enum TokenType;

    while (peek() != '"' && !isAtEnd()) {
      if (peek() == '\n') line++;
      advance();
    }

    if (isAtEnd()) {
      lox::error(line, "Unterminated string.");
      return;
    }

    advance(); // The closing ".

    // Trim the surrounding quotes.
    auto&& value = source.substr(start + 1, (current - 1) - (start + 1));
    addToken(STRING, value);
  }
};
}
