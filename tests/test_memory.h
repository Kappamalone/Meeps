#include "types.h"
#include <array>

// Assumed to be in little endian format
class TestMemory {
public:
  std::array<uint8_t, 100 * 1024 * 1024> mem;

  TestMemory() { Reset(); }

  void Reset() { mem.fill(0); }

  template <typename T> static auto read(void *m, size_t addr) {
    return *(T *)&((TestMemory *)m)->mem[addr];
  }

  template <typename T> static void write(void *m, size_t addr, T value) {
    *(T *)&((TestMemory *)m)->mem[addr] = value;
  }
};