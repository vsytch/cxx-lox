#pragma once

#include <fmt/format.h>

#include <optional>
#include <string>
#include <variant>

namespace cxx {
template<typename... Ts>
struct match : Ts... { using Ts::operator()...; };
template<typename... Ts>
match(Ts...) -> match<Ts...>;
}

namespace lox {
using Object = std::variant<
  std::monostate,
  double,
  std::string
>;
}

namespace fmt {
template<>
struct formatter<lox::Object> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const lox::Object& obj, FormatContext& ctx) const {
    using namespace std;

    return format_to(ctx.out(), "{}", visit(cxx::match{
      [](std::monostate) { return "nil"s; },
      [](const double val) { return fmt::format("{}", val); },
      [](const std::string& val) { return val; }
    }, obj));
  }
};
}
