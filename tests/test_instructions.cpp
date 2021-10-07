#include <array>
#include <doctest.h>
#include <fmt/core.h>
#include <r3000.h>
#include <r3000interpreter.h>
#include "test_memory.h"

using namespace Meeps;

CPU r3000{CPUMode::Interpreter};
TestMemory mem{};
auto &state = r3000.state;

TEST_CASE("CPU Instructions") {
  state.SetGPR(0x1, 0xff0f);
  state.SetGPR(0x2, 0x30f);

  // and $4 , $2 , $1
  R3000Interpreter::LogicalInstruction<Logical::AND>(state, 0x00412024);
  REQUIRE(state.GetGPR(4) == 0x30f);

  // andi $4 , $4 , 0xff
  R3000Interpreter::LogicalInstruction<Logical::ANDI>(state, 0x308400ff);
  REQUIRE(state.GetGPR(4) == 0xf);

  // lui $8 , 0x13
  R3000Interpreter::ShiftInstruction<Shift::LUI>(state, 0x3c080013);
  REQUIRE(state.GetGPR(8) == 0x00130000);

  // sllv $25, $24, $3
  state.SetGPR(3, 0x2); // Set shift amount to 2
  state.SetGPR(24, 0x100);
  R3000Interpreter::ShiftInstruction<Shift::SLLV>(state, 0x0078c804);
  REQUIRE(state.GetGPR(25) == 0x400);

  // srav $8 , $8 , $7
  state.SetGPR(7, 0x3);
  state.SetGPR(8, 0x80000000);
  R3000Interpreter::ShiftInstruction<Shift::SRAV>(state, 0x00e84007);
  REQUIRE(state.GetGPR(8) == 0xf0000000);

  //TODO: test comparison
}