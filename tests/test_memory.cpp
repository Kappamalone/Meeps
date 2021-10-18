//#pragma once

#include "test_memory.h"
#include "test_cop0.h"
#include <array>
#include <doctest.h>
#include <fmt/core.h>
#include <r3000.h>
#include <r3000interpreter.h>


using namespace Meeps;

static TestCOP0 cop0{};
static CPU r3000{CPUMode::Interpreter, &cop0};
static TestMemory memory{};
static auto &state = r3000.GetState();

TEST_CASE("Memory Interface") {
  r3000.SetMemoryPointer(&memory);
  r3000.SetReadPointer<uint8_t>(&TestMemory::read<uint8_t>);
  r3000.SetWritePointer<uint8_t>(&TestMemory::write<uint8_t>);
  r3000.SetReadPointer<uint16_t>(&TestMemory::read<uint16_t>);
  r3000.SetWritePointer<uint16_t>(&TestMemory::write<uint16_t>);
  r3000.SetReadPointer<uint32_t>(&TestMemory::read<uint32_t>);
  r3000.SetWritePointer<uint32_t>(&TestMemory::write<uint32_t>);

  state.write8(0x100, 0xBE);
  state.write8(0x101, 0xEF);
  REQUIRE(state.read16(0x100) == 0xEFBE);
  state.write16(0x200, 0xBEEF);
  REQUIRE(state.read16(0x200) == 0xBEEF);
  state.write32(0x300, 0x11223344);
  REQUIRE(state.read32(0x300) == 0x11223344);
}