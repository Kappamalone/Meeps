#include "types.h"
#include <array>
#include <stdint.h>

// Assumed to be in little endian format
class TestMemory {
public:
  std::array<uint8_t, 100 * 1024 * 1024> mem;
  size_t instrCounter = 0;

  TestMemory() { Reset(); }

  void Reset() {
    mem.fill(0);
    instrCounter = 0;
  }

  template <typename T> static auto read(void *m, size_t addr) {
    return *(T *)&((TestMemory *)m)->mem[addr];
  }

  template <typename T> static void write(void *m, size_t addr, T value) {
    *(T *)&((TestMemory *)m)->mem[addr] = value;
  }

  // For manually writing instructions to memory
  void WriteInstrSequential(uint32_t value) {
    *(uint32_t *)&mem[instrCounter] = value;
    instrCounter += 4;
  }
};