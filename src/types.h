#pragma once
#include <cstdint>

namespace Meeps
{
template <class T>
using readPointer = T (*)(void*, size_t);

template <class T>
using writePointer = void (*)(void*, size_t, T);
}  // namespace Meeps