#pragma once
#include "common.h"
#include "fmt/core.h"
#include "state.h"
#include <stdint.h>

namespace Meeps {

enum class Invalid { NA };

enum class Special { SYSCALL, BREAK };

enum class ALoad { LB, LBU, LH, LHU, LW };

enum class AStore { SB, SH, SW };

enum class ULoadStore { LWL, LWR, SWL, SWR };

enum class Arithmetic { ADD, ADDU, ADDI, ADDIU, SUB, SUBU };

enum class Comparison { SLT, SLTI, SLTU, SLTIU };

enum class Logical { AND, ANDI, OR, ORI, XOR, XORI, NOR };

enum class Shift { SLL, SRL, SRA, SLLV, SRLV, SRAV, LUI };

enum class MulDiv { MULT, MULTU, DIV, DIVU, MFHI, MFLO, MTHI, MTLO };

enum class Jump { J, JAL, JR, JALR };

enum class Branch { BEQ, BNE, BLTZ, BGEZ, BGTZ, BLEZ, BLTZAL, BGEZAL };

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
  static void ExecuteInstruction(State &state) {
#ifndef NDEBUG
    fmt::print("PC: {:08X}\n", state.pc);
#endif
    Instruction instr = state.read32(state.pc);
    state.pc = state.nextPC;
    state.nextPC += 4;

    primaryTable[instr.i.op](state, instr);
  }

  // TODO: load delays maybe?
  template <ALoad T>
  static void ALoadInstruction(State &state, Instruction instr) {
    uint32_t value;
    uint32_t dest = instr.i.rt;
    uint32_t addr = state.GetGPR(instr.i.rs) + instr.i.imm;

    if constexpr (T == ALoad::LB) {
      value = (int32_t)(int8_t)state.read8(addr);
    } else if constexpr (T == ALoad::LBU) {
      value = state.read8(addr);
    } else if constexpr (T == ALoad::LH) {
      value = (int32_t)(int16_t)state.read16(addr);
    } else if constexpr (T == ALoad::LHU) {
      value = state.read16(addr);
    } else if constexpr (T == ALoad::LW) {
      value = state.read32(addr);
    }

    state.SetGPR(dest, value);
  }

  template <AStore T>
  static void AStoreInstruction(State &state, Instruction instr) {
    uint32_t value = state.GetGPR(instr.i.rt);
    uint32_t addr = state.GetGPR(instr.i.rs) + instr.i.imm;
    if constexpr (T == AStore::SB) {
      state.write8(addr, value & 0xff);
    } else if constexpr (T == AStore::SH) {
      state.write8(addr, value & 0xffff);
    } else if constexpr (T == AStore::SW) {
      state.write8(addr, value);
    }
  }

  template <ULoadStore T>
  static void ULoadStoreInstruction(State &state, Instruction instr) {
    return;
  }

  template <Arithmetic T>
  static void ArithmeticInstruction(State &state, Instruction instr) {
    uint32_t dest;
    uint32_t operand;
    uint32_t value = state.GetGPR(instr.i.rs);

    if constexpr (ValueIsIn(T, Arithmetic::ADDI, Arithmetic::ADDIU)) {
      dest = instr.i.rt;
    } else {
      dest = instr.r.rd;
    }

    if constexpr (ValueIsIn(T, Arithmetic::ADD, Arithmetic::ADDI,
                            Arithmetic::SUB)) {
      // signed
      // TODO: how do you detect overflow again?
      operand = T == Arithmetic::ADDI ? (int32_t)(int16_t)instr.i.imm
                                      : state.GetGPR(instr.i.rt);
    } else {
      // unsigned
      // TODO: check ternary constexpr stuff on godbolt
      operand = T == Arithmetic::ADDIU ? instr.i.imm : state.GetGPR(instr.i.rt);
    }

    if constexpr (ValueIsIn(T, Arithmetic::SUB, Arithmetic::SUBU)) {
      value -= operand;
    } else {
      value += operand;
    }

    state.SetGPR(dest, value);
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
      // unsigned
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

    if constexpr (ValueIsIn(T, Logical::AND, Logical::ANDI)) {
      value &= operand;
    } else if constexpr (ValueIsIn(T, Logical::OR, Logical::ORI)) {
      value |= operand;
    } else if constexpr (ValueIsIn(T, Logical::XOR, Logical::XORI)) {
      value ^= operand;
    } else if constexpr (T == Logical::NOR) {
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

  template <MulDiv T>
  static void MulDivInstruction(State &state, Instruction instr) {
    if constexpr (ValueIsIn(T, MulDiv::MFHI, MulDiv::MFLO)) {
      uint32_t dest = instr.r.rd;
      uint32_t value;
      if constexpr (T == MulDiv::MFHI) {
        value = state.hi;
      } else {
        value = state.lo;
      }
      state.SetGPR(dest, value);
      return;
    }

    if constexpr (ValueIsIn(T, MulDiv::MTHI, MulDiv::MTLO)) {
      uint32_t value = state.GetGPR(instr.i.rs);
      if constexpr (T == MulDiv::MFHI) {
        state.hi = value;
      } else {
        state.lo = value;
      }
      return;
    }

    uint32_t op1 = state.GetGPR(instr.i.rs);
    uint32_t op2 = state.GetGPR(instr.i.rt);

    if constexpr (T == MulDiv::MULT) {
      uint64_t result = (int64_t)(int32_t)op1 * (int64_t)(int32_t)op2;
      state.hi = result & 0xffff'ffff'0000'0000 >> 32;
      state.lo = result & 0xffff'ffff;
      return;
    }

    if constexpr (T == MulDiv::MULTU) {
      uint64_t result = (uint64_t)op1 * (uint64_t)op2;
      state.hi = result & 0xffff'ffff'0000'0000 >> 32;
      state.lo = result & 0xffff'ffff;
      return;
    }

    if constexpr (T == MulDiv::DIV) {
      uint32_t quotient = (int32_t)op1 / (int32_t)op2;
      uint32_t remainder = (int32_t)op1 % (int32_t)op2;

      state.lo = quotient;
      state.hi = remainder;
      return;
    }

    if constexpr (T == MulDiv::DIVU) {
      uint32_t quotient = op1 / op2;
      uint32_t remainder = op1 % op2;

      state.lo = quotient;
      state.hi = remainder;
      return;
    }
  }

  template <Jump T>
  static void JumpInstruction(State &state, Instruction instr) {
    // We set nextPC to emulate branch delays, with the real PC having a delay
    // of 1 instr
    uint32_t value;

    if constexpr (ValueIsIn(T, Jump::J, Jump::JAL)) {
      // TODO: are we using the current pc or nextPC? (should be the same
      // regardless I think)
      value = (state.pc & 0xf000'0000) + (instr.j.target << 2);
    } else {
      value = state.GetGPR(instr.i.rs);
    }

    // Example:
    // PC          -> 0xbfc0'0000 : J instr
    // NextPC      -> 0xbfc0'0004 : Branch Delay instr
    // Return Addr -> 0xbfc0'0008 : Return instr
    // In main loop:
    // PC = NextPC, then NextPC = value, therefore Return Addr = PC + 4
    // note: PC = ($ + 4)
    if constexpr (T == Jump::JAL) {
      state.SetGPR(31, state.pc + 4); // 31 = $ra
    } else if constexpr (T == Jump::JALR) {
      uint32_t dest = instr.r.rd;
      state.SetGPR(dest, state.pc + 4);
    }

    state.nextPC = value;
  }

  template <Branch T>
  static void BranchInstruction(State &state, Instruction instr) {
    uint32_t rsReg = state.GetGPR(instr.i.rs);
    uint32_t value = state.pc + (instr.i.imm * 4);

    if constexpr (T == Branch::BEQ) {
      if (rsReg == state.GetGPR(instr.i.rt)) {
        state.nextPC = value;
      } else if constexpr (T == Branch::BNE) {
        if (rsReg != state.GetGPR(instr.i.rt)) {
          state.nextPC = value;
        }
      } else if constexpr (T == Branch::BLTZ) {
        if (rsReg < 0) {
          state.nextPC = value;
        }
      } else if constexpr (T == Branch::BGEZ) {
        if (rsReg >= 0) {
          state.nextPC = value;
        }
      } else if constexpr (T == Branch::BGTZ) {
        if (rsReg > 0) {
          state.nextPC = value;
        }
      } else if constexpr (T == Branch::BLEZ) {
        if (rsReg <= 0) {
          state.nextPC = value;
        }
      } else if constexpr (T == Branch::BLTZAL) {
        if (rsReg < 0) {
          state.nextPC = value;
          state.SetGPR(31, state.pc + 4);
        }
      } else if constexpr (T == Branch::BGEZAL) {
        if (rsReg >= 0) {
          state.nextPC = value;
          state.SetGPR(31, state.pc + 4);
        }
      }
    }
  }

  template <Special T>
  static void SpecialInstruction(State &state, Instruction instr) {
    fmt::print("[SYSCALL / BREAK] PC: {:08X} NEXTPC: {:08X} OPCODE: {:08X}\n",
               state.pc, state.nextPC, instr.value);
    exit(1);
  }

  template <Invalid T>
  static void InvalidInstruction(State &state, Instruction instr) {
    fmt::print(
        "[INVALID INSTRUCTION] PC: {:08X} NEXTPC: {:08X} OPCODE: {:08X}\n",
        state.pc, state.nextPC, instr.value);
    exit(1);
  }

  static void SecondaryTableLookup(State &state, Instruction instr) {
    secondaryTable[instr.r.func](state, instr);
  }

private:
#define instr(type, op) type##Instruction<type::op>
  // clang-format off
  constexpr static std::array<interpreterfp, 64> primaryTable = {
    SecondaryTableLookup,
  };
  constexpr static std::array<interpreterfp, 64> secondaryTable = {
    instr(Shift, SLL),       instr(Invalid, NA),      instr(Shift, SRL),      instr(Shift, SRA),       // first column
    instr(Shift, SLLV),      instr(Invalid, NA),      instr(Shift, SRLV),     instr(Shift, SRAV),  
    instr(Jump, JR),         instr(Jump, JALR),       instr(Invalid, NA),     instr(Invalid, NA),    // second column
    instr(Special, SYSCALL), instr(Special, BREAK),   instr(Invalid, NA),     instr(Invalid, NA), 
    instr(MulDiv, MFHI),     instr(MulDiv, MTHI),     instr(MulDiv, MFLO),    instr(MulDiv, MTLO),     // third column
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),
    instr(MulDiv, MULT),     instr(MulDiv, MULTU),    instr(MulDiv, DIV),     instr(MulDiv, DIVU),     // fourth column
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),
    instr(Arithmetic, ADD),  instr(Arithmetic, ADDU), instr(Arithmetic, SUB), instr(Arithmetic, SUBU), // fifth column
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Comparison, SLT), instr(Comparison, SLTU), // sixth column
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),    // seventh column
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),    // eigth column
    instr(Invalid, NA),      instr(Invalid, NA),      instr(Invalid, NA),     instr(Invalid, NA),
  };
// clang-format on
#undef instr
};

} // namespace Meeps