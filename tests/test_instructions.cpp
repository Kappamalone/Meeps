#include "test_memory.h"
#include <doctest.h>
#include <fmt/core.h>
#include <r3000.h>
#include <r3000interpreter.h>
#include "unicorn_emu.h"

using namespace Meeps;

static CPU r3000{CPUMode::Interpreter};
static TestMemory memory{};
static auto &state = r3000.GetState();
static auto uemu = UnicornMIPS();

static bool CompareRegisters(uint32_t* meepsRegs, uint32_t* unicornRegs) {
  for (auto i = 0; i < 32; i++) {
    if (!(meepsRegs[i] == unicornRegs[i])) {
      fmt::print("Aborting on mismatched reg [0x{:02}] !\n", i);
      return false;
    }
  }
  return true;
}

TEST_CASE("Unicorn Comparison") {
  r3000.SetMemoryPointer(&memory);
  r3000.SetReadPointer(&TestMemory::read<uint8_t>);
  r3000.SetWritePointer(&TestMemory::write<uint8_t>);
  r3000.SetReadPointer(&TestMemory::read<uint16_t>);
  r3000.SetWritePointer(&TestMemory::write<uint16_t>);
  r3000.SetReadPointer(&TestMemory::read<uint32_t>);
  r3000.SetWritePointer(&TestMemory::write<uint32_t>);

  SUBCASE("Sanity Test") {
    r3000.Reset();
    memory.Reset();

    memory.write<uint32_t>(&memory, 0, 0x3c080013); // lui $8 , 0x13
    r3000.Run(1);
    uint32_t* regs = uemu.ExecuteInstructions(memory.mem.data(), 4);

    REQUIRE(CompareRegisters(state.gpr.data(), regs));
  }

  SUBCASE("Logical Instruction Tests") {
    r3000.Reset();
    memory.Reset();

    Instruction shiftImm = 0;
    shiftImm.r.op = 0;
    shiftImm.r.rs = 0;
    shiftImm.r.rt = 0; //TODO: random
    shiftImm.r.rd = 0; //TODO: random
    shiftImm.r.shamt = 0; // TODO: random
    shiftImm.r.func = 0; //TODO: random lower 2 bits

    // TODO: fuzz like, 10,000 instructions per group and test (except maybe jumps?)

    //memory.write<uint32_t>(&memory, 0, 0x3c080013); // lui $8 , 0x13
    //r3000.Run(1);
    //uint32_t* regs = uemu.ExecuteInstructions(memory.mem.data(), 4);

    //REQUIRE(CompareRegisters(state.gpr.data(), regs));
  }
}

TEST_CASE("Basic CPU Instructions") {
  // and $4 , $2 , $1
  state.SetGPR(0x1, 0xff0f);
  state.SetGPR(0x2, 0x30f);
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

  // slt $1 , $25 , $24
  state.SetGPR(25, 10);
  state.SetGPR(24, 12);
  R3000Interpreter::ComparisonInstruction<Comparison::SLT>(state, 0x0338082a);
  REQUIRE(state.GetGPR(1) == 1);

  state.SetGPR(25, -12);
  state.SetGPR(24, -9);
  R3000Interpreter::ComparisonInstruction<Comparison::SLT>(state, 0x0338082a);
  REQUIRE(state.GetGPR(1) == 1);
}