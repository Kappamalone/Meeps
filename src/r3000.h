#pragma once

#include <array>
#include <stdint.h>
#include <type_traits>
#include "r3000interpreter.h"
#include "types.h"

#ifndef NDEBUG
#define private public
#endif

namespace Meeps
{
enum class CPUMode
{
  Interpreter,
};

class CPU
{
public:
  CPU(CPUMode mode) { this->mode = mode; }

  void SetPC(uint32_t pc)
  {
    state.pc = pc;
    state.nextPC = state.pc + 4;
  }

  void SetMemoryPointer(void* mp) { state.mp = mp; }

  template <class T>
  void SetReadPointer(readPointer<T> rp)
  {
    if constexpr (std::is_same_v<readPointer<uint8_t>, readPointer<T>>)
    {
      state.rp8 = rp;
    }

    if constexpr (std::is_same_v<readPointer<uint16_t>, readPointer<T>>)
    {
      state.rp16 = rp;
    }

    if constexpr (std::is_same_v<readPointer<uint32_t>, readPointer<T>>)
    {
      state.rp32 = rp;
    }
  }

  template <class T>
  void SetWritePointer(writePointer<T> wp)
  {
    if constexpr (std::is_same_v<writePointer<uint8_t>, writePointer<T>>)
    {
      state.wp8 = wp;
    }

    if constexpr (std::is_same_v<writePointer<uint16_t>, writePointer<T>>)
    {
      state.wp16 = wp;
    }

    if constexpr (std::is_same_v<writePointer<uint32_t>, writePointer<T>>)
    {
      state.wp32 = wp;
    }
  }

private:
  // Represents the internal state of the cpu
  struct State
  {
  public:
    State()
    {
      gpr.fill(0);
      pc = 0;
      nextPC = 0;
    }

    // Two PC's are used to deal with branch delays
    uint32_t pc;
    uint32_t nextPC;

    std::array<uint32_t, 32> gpr;

    // TODO: asserts
    uint32_t GetGPR(size_t reg) { return gpr[reg]; }

    void SetGPR(size_t reg, uint32_t value)
    {
      if (reg)
        gpr[reg] = value;
    }

    // TODO: Test on godbolt to see if faster/slower than virtual functions
    inline uint8_t read8(size_t addr) { return rp8(mp, addr); }
    inline void write8(size_t addr, uint8_t value) { wp8(mp, addr, value); }

    inline uint16_t read16(size_t addr) { return rp16(mp, addr); }
    inline void write16(size_t addr, uint16_t value) { wp16(mp, addr, value); }

    inline uint32_t read32(size_t addr) { return rp32(mp, addr); }
    inline void write32(size_t addr, uint32_t value) { wp32(mp, addr, value); }

    // Interface
    void* mp;
    readPointer<uint8_t> rp8;
    readPointer<uint16_t> rp16;
    readPointer<uint32_t> rp32;
    writePointer<uint8_t> wp8;
    writePointer<uint16_t> wp16;
    writePointer<uint32_t> wp32;

  } state;

  CPUMode mode;
};

}  // namespace Meeps