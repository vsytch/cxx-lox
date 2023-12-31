#pragma once

#include <boost/hana/functional/overload.hpp>
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

    // Cannot use overload_linearly() due to implicit double <-> bool conversion.
    return format_to(ctx.out(), "{}", visit(overload(
      [](std::monostate) { return "nil"s; },
      [](const double val) { return fmt::format("{}", val); },
      [](const string& val) { return val; },
      [](const bool val) { return fmt::format("{}", val); }
    ), obj));
  }
};
}
