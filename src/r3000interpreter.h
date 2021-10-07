#pragma once
#include "common.h"
#include "fmt/core.h"
#include "state.h"
#include <stdint.h>

namespace Meeps {

#define TEMPLATE_LOGICAL(instr, op)                                            \
  if constexpr (T == instr) {                                                  \
    value op operand;                                                          \
  }

enum class Comparison { SLT, SLTI, SLTU, SLTIU };

enum class Logical { AND, ANDI, OR, ORI, XOR, XORI, NOR };

enum class Shift { SLL, SRL, SRA, SLLV, SRLV, SRAV, LUI };
union Instruction {
  uint32_t value;
  Instruction(uint32_t value) : value(value) {}

  // Immediate type opcodes
  struct {
    unsigned imm : 16;
    unsigned rt : 5;
    unsigned rs : 5;
    unsigned op : 6;
  } i;

  // Jump type opcodes
  struct {
    unsigned target : 26;
    unsigned op : 6;
  } j;

  // Register type opcodes
  struct {
    unsigned func : 6;
    unsigned shamt : 5;
    unsigned rd : 5;
    unsigned rt : 5;
    unsigned rs : 5;
    unsigned op : 6;
  } r;
};

using interpreterfp = void (*)(State &, Instruction);

class R3000Interpreter {
public:
  static void Execute(State &state) {
    fmt::print("Hello World! {}\n", state.GetGPR(10));
  }

  template <Comparison T>
  static void ComparisonInstruction(State &state, Instruction instr) {
    uint32_t dest;
    uint32_t operand;
    uint32_t value = state.GetGPR(instr.i.rs);

    if constexpr (ValueIsIn(T, Comparison::SLT, Comparison::SLTU)) {
      dest = instr.r.rd;
      operand = state.GetGPR(instr.i.rt);
    } else {
      dest = instr.i.rt;
      operand = (uint32_t)(uint16_t)instr.i.imm;
    }

    if constexpr (ValueIsIn(T, Comparison::SLT, Comparison::SLTI)) {
      // signed
      value = (int32_t)value < (int32_t)operand;
    } else {
      //unsigned
      value = value < operand;
    }

    state.SetGPR(dest, value);
  }

  template <Logical T>
  static void LogicalInstruction(State &state, Instruction instr) {
    uint32_t dest;
    uint32_t operand;
    uint32_t value = state.GetGPR(instr.i.rs);

    if constexpr (ValueIsIn(T, Logical::ANDI, Logical::ORI, Logical::XORI)) {
      dest = instr.i.rt;
      operand = instr.i.imm;
    } else {
      dest = instr.r.rd;
      operand = state.GetGPR(instr.i.rt);
    }

    TEMPLATE_LOGICAL(Logical::AND, &=);
    TEMPLATE_LOGICAL(Logical::ANDI, &=);
    TEMPLATE_LOGICAL(Logical::OR, |=);
    TEMPLATE_LOGICAL(Logical::ORI, |=);
    TEMPLATE_LOGICAL(Logical::XOR, ^=);
    TEMPLATE_LOGICAL(Logical::XORI, ^=);
    if constexpr (T == Logical::NOR) {
      value = ~(value | operand);
    }

    state.SetGPR(dest, value);
  }

  template <Shift T>
  static void ShiftInstruction(State &state, Instruction instr) {
    if constexpr (T == Shift::LUI) {
      state.SetGPR(instr.i.rt, instr.i.imm << 16);
      return;
    }

    uint32_t operand;
    uint32_t dest = instr.r.rd;
    uint32_t value = state.GetGPR(instr.i.rt);

    if constexpr (ValueIsIn(T, Shift::SLL, Shift::SRL, Shift::SRA)) {
      operand = instr.i.rs; // rs is shift amount for shift instructions
    } else {
      operand = state.GetGPR(instr.i.rs) & 0x1f;
    }

    if constexpr (ValueIsIn(T, Shift::SRA, Shift::SRAV)) {
      value = (int32_t)value >> operand;
    }

    if constexpr (ValueIsIn(T, Shift::SLL, Shift::SLLV)) {
      value <<= operand;
    }

    if constexpr (ValueIsIn(T, Shift::SRL, Shift::SRLV)) {
      value >>= operand;
    }

    state.SetGPR(dest, value);
  }

  static void SecondaryTableLookup(State &state, Instruction instr) {
    secondaryTable[instr.r.func](state, instr);
  }

  static void InvalidInstruction(State &state, Instruction instr) {
    fmt::print(
        "[INVALID INSTRUCTION] PC: {:08X} NEXTPC: {:08X} OPCODE: {:08X}\n",
        state.pc, state.nextPC, instr.value);
  }

private:
  constexpr static std::array<interpreterfp, 64> primaryTable = {

  };
  constexpr static std::array<interpreterfp, 32> secondaryTable = {

  };
};

} // namespace Meeps