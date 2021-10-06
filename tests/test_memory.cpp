//#pragma once

#include "doctest.h"
#include "r3000interpreter.h"
#include <array>
#include <fmt/core.h>
#include <r3000.h>

using namespace Meeps;

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

CPU r3000{CPUMode::Interpreter};
TestMemory mem{};
auto &state = r3000.state;

// TODO: figure out how to use unicorn

TEST_CASE("Memory Interface") {
  r3000.SetMemoryPointer(&mem);
  r3000.SetReadPointer(&TestMemory::read<uint8_t>);
  r3000.SetWritePointer(&TestMemory::write<uint8_t>);
  r3000.SetReadPointer(&TestMemory::read<uint16_t>);
  r3000.SetWritePointer(&TestMemory::write<uint16_t>);
  r3000.SetReadPointer(&TestMemory::read<uint32_t>);
  r3000.SetWritePointer(&TestMemory::write<uint32_t>);

  state.write8(0x100, 0xBE);
  state.write8(0x101, 0xEF);
  REQUIRE(state.read16(0x100) == 0xEFBE);
  state.write16(0x200, 0xBEEF);
  REQUIRE(state.read16(0x200) == 0xBEEF);
  state.write32(0x300, 0x11223344);
  REQUIRE(state.read32(0x300) == 0x11223344);
}

TEST_CASE("CPU Instructions") {

  state.SetGPR(0x1, 0xff0f);
  state.SetGPR(0x2, 0x30f);
  
  // and $4 , $2 , $1
  R3000Interpreter::LogicalInstruction<Logical::AND>(state, 0x00412024);
  REQUIRE(state.GetGPR(0x4) == 0x30f);

  // andi $4 , $4 , 0xff
  R3000:R3000Interpreter::LogicalInstruction<Logical::ANDI>(state, 0x308400ff);
  REQUIRE(state.GetGPR(0x4) == 0xf);
}