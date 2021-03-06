#pragma once
#include <cstdint>
#include <stdio.h> //TODO: why is this redundatn include required for size_t on linux? (travis)

namespace Meeps
{
template <class T>
using readPointer = T (*)(void*, size_t);

template <class T>
using writePointer = void (*)(void*, size_t, T);
}  // namespace Meeps