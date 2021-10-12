#include "test_memory.h"
#include "unicorn_emu.h"
#include <chrono>
#include <doctest.h>
#include <fmt/core.h>
#include <r3000.h>
#include <r3000interpreter.h>
#include <random>

using namespace Meeps;

static CPU r3000{CPUMode::Interpreter};
static TestMemory memory{};
static auto &state = r3000.GetState();
static auto uemu = UnicornMIPS();
static auto rnum = [](int lower, int upper) {
  static std::mt19937 rng(
      std::chrono::steady_clock::now().time_since_epoch().count());
  return std::uniform_int_distribution<uint32_t>(lower, upper)(rng);
};

static bool CompareRegisters(uint32_t *meepsRegs, uint32_t *unicornRegs,
                             bool printRegs) {
  auto success = true;
  for (auto i = 0; i < 32; i++) {
    if (!(meepsRegs[i] == unicornRegs[i])) {
      success = false;
      fmt::print("DIFF ");
    }

    if (printRegs)
      fmt::print("Meeps r{}: 0x{:08X}, Unicorn r{}: 0x{:08X}\n", i,
                 meepsRegs[i], i, unicornRegs[i]);
  }
  return success;
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
    uint32_t *regs = uemu.ExecuteInstructions(memory.mem.data(), 4);

    REQUIRE(CompareRegisters(state.gpr.data(), regs, false));
  }

  SUBCASE("Shift-Imm Instruction Tests") {
    for (auto i = 0; i < 10; i++) {
      r3000.Reset();
      uemu.Reset();
      memory.Reset();
      auto instrCount = 100;

      for (auto i = 0; i < 32; i++) {
        uint32_t value = rnum(0, 0xffffffff);
        state.SetGPR(i, value);
        uemu.SetGPR(i, value);
      }

      REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR(), true));

      size_t funcs[] = {0, 2, 3};
      Instruction shiftImm = 0;
      shiftImm.r.op = 0;
      shiftImm.r.rs = 0;

      for (auto i = 0; i < instrCount; i++) {
        shiftImm.r.rt = rnum(0, 0x1f);       // random 5 bits
        shiftImm.r.rd = rnum(0, 0x1f);       // random 5 bits
        shiftImm.r.shamt = rnum(0, 0x1f);    // random 5 bits
        shiftImm.r.func = funcs[rnum(0, 2)]; // random lower 2 bits
        memory.write<uint32_t>(&memory, i * 4, shiftImm.value);
        fmt::print("Writing Instruction: 0x{:08X}\n", shiftImm.value);
      }

      r3000.Run(instrCount);
      uint32_t *regs =
          uemu.ExecuteInstructions(memory.mem.data(), instrCount * 4);
      REQUIRE(CompareRegisters(state.gpr.data(), regs, true));
    }
  }

  SUBCASE("Shift-Reg Instruction Tests") {
    for (auto i = 0; i < 10; i++) {
      r3000.Reset();
      uemu.Reset();
      memory.Reset();
      auto instrCount = 100;

      for (auto i = 0; i < 32; i++) {
        uint32_t value = rnum(0, 0xffffffff);
        state.SetGPR(i, value);
        uemu.SetGPR(i, value);
      }
      
      REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR(), true));

      size_t funcs[] = {4, 6, 7};
      Instruction shiftReg = 0;
      shiftReg.r.op = 0;

      for (auto i = 0; i < instrCount; i++) {
        shiftReg.r.rs = rnum(0, 0x1f);       // random 5 bits
        shiftReg.r.rt = rnum(0, 0x1f);       // random 5 bits
        shiftReg.r.rd = rnum(0, 0x1f);       // random 5 bits
        shiftReg.r.func = 0b100 | funcs[rnum(0, 2)]; // random lower 2 bits
        memory.write<uint32_t>(&memory, i * 4, shiftReg.value);
        fmt::print("Writing Instruction: 0x{:08X}\n", shiftReg.value);
      }

      r3000.Run(instrCount);
      uint32_t *regs =
          uemu.ExecuteInstructions(memory.mem.data(), instrCount * 4);
      REQUIRE(CompareRegisters(state.gpr.data(), regs, true));
    }
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