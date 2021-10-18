#pragma once

#ifndef NDEBUG
#define DPRINT(f_, ...) fmt::print((f_), __VA_ARGS__)
#else
#define DPRINT(f_, ...)
#endif

namespace Meeps {
// https://stackoverflow.com/questions/15181579/c-most-efficient-way-to-compare-a-variable-to-multiple-values
template <typename First, typename... T>
constexpr bool ValueIsIn(First &&first, T &&...t) {
  return ((first == t) || ...);
}
} // namespace Meeps