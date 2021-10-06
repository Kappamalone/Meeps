#pragma once
#include "types.h"

namespace Meeps {
// https://stackoverflow.com/questions/15181579/c-most-efficient-way-to-compare-a-variable-to-multiple-values
template <typename First, typename... T>
constexpr bool ValueIsIn(First &&first, T &&...t) {
  return ((first == t) || ...);
}
} // namespace Meeps