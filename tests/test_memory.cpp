//#pragma once

#include <array>
#include <fmt/core.h>
#include <r3000.h>
#include "doctest.h"


using namespace Meeps;

static constexpr int KILOBYTE = 1024;

// Assumed to be in little endian format
class TestMemory
{
public:
  std::array<uint8_t, KILOBYTE> mem;

  TestMemory() { mem.fill(0); }

  template <typename T>
  static auto read(void* m, size_t addr)
  {
    return *(T*)&((TestMemory*)m)->mem[addr];
  }

  template <typename T>
  static void write(void* m, size_t addr, T value)
  {
    *(T*)&((TestMemory*)m)->mem[addr] = value;
  }
};

CPU r3000{CPUMode::Interpreter};
TestMemory mem{};


TEST_CASE("Memory Interface")
{
  r3000.SetMemoryPointer(&mem);
  r3000.SetReadPointer(&TestMemory::read<uint8_t>);
  r3000.SetWritePointer(&TestMemory::write<uint8_t>);
  r3000.SetReadPointer(&TestMemory::read<uint16_t>);
  r3000.SetWritePointer(&TestMemory::write<uint16_t>);
  r3000.SetReadPointer(&TestMemory::read<uint32_t>);
  r3000.SetWritePointer(&TestMemory::write<uint32_t>);

  auto& state = r3000.state;
  state.write8(0x100, 0xBE);
  state.write8(0x101, 0xEF);
  REQUIRE(state.read16(0x100) == 0xEFBE);
  state.write16(0x200, 0xBEEF);
  REQUIRE(state.read16(0x200) == 0xBEEF);
  state.write32(0x300, 0x11223344);
  REQUIRE(state.read32(0x300) == 0x11223344);
}