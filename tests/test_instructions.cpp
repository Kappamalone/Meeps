#include "test_cop0.h"
#include "test_memory.h"
#include "unicorn_emu.h"
#include <chrono>
#include <doctest.h>
#include <fmt/core.h>
#include <r3000.h>
#include <r3000interpreter.h>
#include <random>
#include <vector>

using namespace Meeps;

static TestCOP0 cop0{};
static CPU r3000{CPUMode::Interpreter, &cop0};
static TestMemory memory{};
static auto &state = r3000.GetState();
static auto uemu = UnicornMIPS();
static constexpr auto instrCount = 100000;

// Helper functions
static auto rnum = [](int lower, int upper) { // both inclusive
  static std::mt19937 rng(
      std::chrono::steady_clock::now().time_since_epoch().count());
  return std::uniform_int_distribution<uint32_t>(lower, upper)(rng);
};

static auto CompareRegisters = [](uint32_t *meepsRegs, uint32_t *unicornRegs) {
  auto success = true;
  for (auto i = 0; i < 32; i++) {
    if (!(meepsRegs[i] == unicornRegs[i])) {
      success = false;
      fmt::print("DIFF ");
    }

    if (!success)
      fmt::print("Meeps r{}: 0x{:08X}, Unicorn r{}: 0x{:08X}\n", i,
                 meepsRegs[i], i, unicornRegs[i]);
  }
  return success;
};

static auto InitRegisters = []() {
  for (auto i = 0; i < 32; i++) {
    uint32_t value = rnum(0, 0xffffffff);
    state.SetGPR(i, value);
    uemu.SetGPR(i, value);
  }
};

TEST_CASE("Unicorn Comparison") {
  r3000.SetMemoryPointer(&memory); // TODO: why doesn't implicit template
                                   // instantiation work on linux? (travis)
  r3000.SetReadPointer<uint8_t>(&TestMemory::read<uint8_t>);
  r3000.SetWritePointer<uint8_t>(&TestMemory::write<uint8_t>);
  r3000.SetReadPointer<uint16_t>(&TestMemory::read<uint16_t>);
  r3000.SetWritePointer<uint16_t>(&TestMemory::write<uint16_t>);
  r3000.SetReadPointer<uint32_t>(&TestMemory::read<uint32_t>);
  r3000.SetWritePointer<uint32_t>(&TestMemory::write<uint32_t>);

  SUBCASE("Sanity Test") {
    r3000.Reset();
    uemu.Reset();
    memory.Reset();
    InitRegisters();
    REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR()));

    memory.write<uint32_t>(&memory, 0, 0x3c080013); // lui $8 , 0x13
    r3000.Run(1);
    uint32_t *regs = uemu.ExecuteInstructions(memory.mem.data(), 4);

    REQUIRE(CompareRegisters(state.gpr.data(), regs));
  }

  SUBCASE("Shift-Imm Instruction Tests") {
    fmt::print("Executing Shift-Imm Instruction Tests\n");
    for (auto i = 0; i < 10; i++) {
      r3000.Reset();
      uemu.Reset();
      memory.Reset();
      InitRegisters();

      std::vector<size_t> funcs = {0x0, 0x2, 0x3};
      Instruction instr = 0;
      instr.r.op = 0;
      instr.r.rs = 0;

      for (auto i = 0; i < instrCount; i++) {
        instr.r.rt = rnum(0, 0x1f);                      // random 5 bits
        instr.r.rd = rnum(0, 0x1f);                      // random 5 bits
        instr.r.shamt = rnum(0, 0x1f);                   // random 5 bits
        instr.r.func = funcs[rnum(0, funcs.size() - 1)]; // random lower 2 bits
        memory.write<uint32_t>(&memory, i * 4, instr.value);
      }

      r3000.Run(instrCount);
      uint32_t *regs =
          uemu.ExecuteInstructions(memory.mem.data(), instrCount * 4);
      REQUIRE(CompareRegisters(state.gpr.data(), regs));
    }
  }

  SUBCASE("Shift-Reg Instruction Tests") {
    fmt::print("Executing Shift-Reg Instruction Tests\n");
    for (auto i = 0; i < 10; i++) {
      r3000.Reset();
      uemu.Reset();
      memory.Reset();
      InitRegisters();

      std::vector<size_t> funcs = {0x4, 0x6, 0x7};
      Instruction instr = 0;
      instr.r.op = 0;

      for (auto i = 0; i < instrCount; i++) {
        instr.r.rs = rnum(0, 0x1f); // random 5 bits
        instr.r.rt = rnum(0, 0x1f); // random 5 bits
        instr.r.rd = rnum(0, 0x1f); // random 5 bits
        instr.r.func =
            0b100 | funcs[rnum(0, funcs.size() - 1)]; // random lower 2 bits
        memory.write<uint32_t>(&memory, i * 4, instr.value);
      }

      r3000.Run(instrCount);
      uint32_t *regs =
          uemu.ExecuteInstructions(memory.mem.data(), instrCount * 4);
      REQUIRE(CompareRegisters(state.gpr.data(), regs));
    }
  }

  // not sure how to test this against unicorn
  // SUBCASE("MulDiv Instruction Tests") {
  // }

  SUBCASE("ALU-Reg Instruction Tests") {
    fmt::print("Executing ALU-Reg Instruction Tests\n");
    for (auto i = 0; i < 10; i++) {
      r3000.Reset();
      uemu.Reset();
      memory.Reset();
      InitRegisters();

      // Can't test 0 (ADD) and 2 (SUB) due to overflow traps terminating
      // unicorn execution
      std::vector<size_t> funcs = {0x1, 0x3, 0x4, 0x5, 0x6, 0x7, 0xA, 0xB};
      Instruction instr = 0;
      instr.r.op = 0;

      for (auto i = 0; i < instrCount; i++) {
        instr.r.rs = rnum(0, 0x1f); // random 5 bits
        instr.r.rt = rnum(0, 0x1f); // random 5 bits
        instr.r.rd = rnum(0, 0x1f); // random 5 bits
        instr.r.func =
            0b10'0000 | funcs[rnum(0, funcs.size() - 1)]; // random lower 4 bits
        memory.write<uint32_t>(&memory, i * 4, instr.value);
      }

      r3000.Run(instrCount);
      uint32_t *regs =
          uemu.ExecuteInstructions(memory.mem.data(), instrCount * 4);
      REQUIRE(CompareRegisters(state.gpr.data(), regs));
    }
  }

  SUBCASE("ALU-imm Instruction Tests") {
    fmt::print("Executing ALU-Imm Instruction Tests\n");
    for (auto i = 0; i < 100; i++) {
      r3000.Reset();
      uemu.Reset();
      memory.Reset();
      InitRegisters();

      // Can't test 0 (ADDI) due to overflow traps terminating unicorn execution
      std::vector<size_t> funcs = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};
      Instruction instr = 0;

      for (auto i = 0; i < instrCount; i++) {
        instr.r.op = 0b00'1000 | funcs[rnum(0, funcs.size() - 1)];
        instr.i.rs = rnum(0, 0x1f);
        instr.i.rt = rnum(0, 0x1f);
        instr.i.imm = rnum(0, 0xffff);
        memory.write<uint32_t>(&memory, i * 4, instr.value);
      }

      r3000.Run(instrCount);
      uint32_t *regs =
          uemu.ExecuteInstructions(memory.mem.data(), instrCount * 4);
      REQUIRE(CompareRegisters(state.gpr.data(), regs));
    }
  }

  SUBCASE("LUI-imm Instruction Tests") {
    fmt::print("Executing LUI-Imm Instruction Tests\n");
    for (auto i = 0; i < 10; i++) {
      r3000.Reset();
      uemu.Reset();
      memory.Reset();
      InitRegisters();

      Instruction instr = 0;
      instr.r.op = 0b001111;

      for (auto i = 0; i < instrCount; i++) {
        instr.i.rt = rnum(0, 0x1f);
        instr.i.imm = rnum(0, 0xffff);
        memory.write<uint32_t>(&memory, i * 4, instr.value);
      }

      r3000.Run(instrCount);
      uint32_t *regs =
          uemu.ExecuteInstructions(memory.mem.data(), instrCount * 4);
      REQUIRE(CompareRegisters(state.gpr.data(), regs));
    }
  }
}

// I am not bothered enough to do this, I'll just write a ps1 emulator
// to iron the rest of the bugs out
TEST_CASE("Manual Testing") {
  r3000.SetMemoryPointer(&memory);
  r3000.SetReadPointer<uint8_t>(&TestMemory::read<uint8_t>);
  r3000.SetWritePointer<uint8_t>(&TestMemory::write<uint8_t>);
  r3000.SetReadPointer<uint16_t>(&TestMemory::read<uint16_t>);
  r3000.SetWritePointer<uint16_t>(&TestMemory::write<uint16_t>);
  r3000.SetReadPointer<uint32_t>(&TestMemory::read<uint32_t>);
  r3000.SetWritePointer<uint32_t>(&TestMemory::write<uint32_t>);
  
  SUBCASE("Aligned Load/Stores") {
    fmt::print("Manually Testing Aligned Load/Stores\n");
    r3000.Reset();
    uemu.Reset();
    memory.Reset();

    state.SetGPR(1, 0x11223344);
    state.SetGPR(2, 10);
    memory.WriteInstrSequential(0xac41fff6); // sw $1, -10($2)
    memory.WriteInstrSequential(0x8c010000); // lw $1, 0($0)
    r3000.Run(memory.instrCounter / 4);
    REQUIRE(state.GetGPR(1) == 0x11223344);
  }
}