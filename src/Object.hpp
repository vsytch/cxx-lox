#pragma once

#include <boost/hana/functional/overload_linearly.hpp>
#include <fmt/format.h>

#include <string>
#include <variant>

namespace lox {
using Object = std::variant<
  std::monostate,
  double,
  std::string,
  bool
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
    using namespace boost::hana;
    using namespace std;

    return format_to(ctx.out(), "{}", visit(overload_linearly(
      [](std::monostate) { return "nil"s; },
      [](const double val) { return fmt::format("{}", val); },
      [](const string& val) { return val; },
      [](const bool val) { return fmt::format("{}", val); }
    ), obj));
  }
};
}
