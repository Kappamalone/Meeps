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

static CPU r3000{CPUMode::Interpreter};
static TestMemory memory{};
static auto &state = r3000.GetState();
static auto uemu = UnicornMIPS();

// Helper functions
static auto rnum = [](int lower, int upper) { // both inclusive
  static std::mt19937 rng(
      std::chrono::steady_clock::now().time_since_epoch().count());
  return std::uniform_int_distribution<uint32_t>(lower, upper)(rng);
};

static auto CompareRegisters = [](uint32_t *meepsRegs, uint32_t *unicornRegs,
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
};

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

      REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR(), false));

      std::vector<size_t> funcs = {0, 2, 3};
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

      REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR(), false));

      std::vector<size_t> funcs = {4, 6, 7};
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
      REQUIRE(CompareRegisters(state.gpr.data(), regs, true));
    }
  }

  SUBCASE("ALU-Reg Instruction Tests") {
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

      REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR(), false));

      // TODO: Unicorn fails on 0 (ADD), 2 (SUB),
      // std::array<size_t,10> funcs = {0,1,2,3,4,5,6,7,0xA,0xB};
      std::vector<size_t> funcs = {1, 3, 4, 5, 6, 7, 0xA, 0xB};
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
      REQUIRE(CompareRegisters(state.gpr.data(), regs, true));
    }
  }

  SUBCASE("ALU-imm Instruction Tests") {
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

      REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR(), false));

      Instruction instr = 0;
      std::vector<size_t> funcs = {0, 1, 2, 3, 4, 5, 6, 7};

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
      REQUIRE(CompareRegisters(state.gpr.data(), regs, true));
    }
  }

  SUBCASE("LUI-imm Instruction Tests") {
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

      REQUIRE(CompareRegisters(state.gpr.data(), uemu.GetAllGPR(), false));

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
      REQUIRE(CompareRegisters(state.gpr.data(), regs, true));
    }
  }
}