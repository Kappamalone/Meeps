#pragma once

#include "r3000interpreter.h"
#include "state.h"
#include <type_traits>

namespace Meeps {

enum class CPUMode {
  Interpreter,
  // soontm
};

class CPU {
public:
  CPU(CPUMode mode, COP0* cop0) : state(cop0) { this->mode = mode; }

  void Run(int cycles) {
    while (cycles--) {
      R3000Interpreter::ExecuteInstruction(state);
    }
  }

  void Reset() { state.Reset(); }

  State &GetState() { return state; }

  void SetPC(uint32_t pc) {
    state.pc = pc;
    state.nextPC = state.pc + 4;
  }

  void SetMemoryPointer(void *mp) { state.mp = mp; }

  template <class T> void SetReadPointer(readPointer<T> rp) {
    if constexpr (std::is_same_v<readPointer<uint8_t>, readPointer<T>>) {
      state.rp8 = rp;
    }

    if constexpr (std::is_same_v<readPointer<uint16_t>, readPointer<T>>) {
      state.rp16 = rp;
    }

    if constexpr (std::is_same_v<readPointer<uint32_t>, readPointer<T>>) {
      state.rp32 = rp;
    }
  }

  template <class T> void SetWritePointer(writePointer<T> wp) {
    if constexpr (std::is_same_v<writePointer<uint8_t>, writePointer<T>>) {
      state.wp8 = wp;
    }

    if constexpr (std::is_same_v<writePointer<uint16_t>, writePointer<T>>) {
      state.wp16 = wp;
    }

    if constexpr (std::is_same_v<writePointer<uint32_t>, writePointer<T>>) {
      state.wp32 = wp;
    }
  }

private:
  State state;
  CPUMode mode;
};

} // namespace Meeps