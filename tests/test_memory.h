#include "types.h"
#include <array>


static constexpr int KILOBYTE = 1024;

// Assumed to be in little endian format
class TestMemory {
public:
  std::array<uint8_t, KILOBYTE> mem;

  TestMemory() { mem.fill(0); }

  template <typename T> static auto read(void *m, size_t addr) {
    return *(T *)&((TestMemory *)m)->mem[addr];
  }

  template <typename T> static void write(void *m, size_t addr, T value) {
    *(T *)&((TestMemory *)m)->mem[addr] = value;
  }
};