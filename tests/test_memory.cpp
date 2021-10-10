//#pragma once

#include "test_memory.h"
#include <array>
#include <doctest.h>
#include <fmt/core.h>
#include <r3000.h>
#include <r3000interpreter.h>

using namespace Meeps;

static CPU r3000{CPUMode::Interpreter};
static TestMemory mem{};
static auto &state = r3000.GetState();

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