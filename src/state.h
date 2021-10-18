#pragma once
#include "types.h"
#include "cop0.h"
#include <array>

namespace Meeps {
struct State {
public:
  State(COP0* cop0) : cop0(cop0) {
    Reset();
  }

  void Reset() {
    gpr.fill(0);
    pc = 0;
    nextPC = pc + 4;
    hi = 0;
    lo = 0;
  }

  uint32_t GetGPR(size_t reg) { return gpr[reg]; }

  void SetGPR(size_t reg, uint32_t value) {
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

  
  uint32_t pc;     // Two PC's are used to deal with branch delays
  uint32_t nextPC;
  uint32_t hi;
  uint32_t lo;
  std::array<uint32_t, 32> gpr;
  COP0* cop0;

  // Interface
  void *mp;
  readPointer<uint8_t> rp8;
  readPointer<uint16_t> rp16;
  readPointer<uint32_t> rp32;
  writePointer<uint8_t> wp8;
  writePointer<uint16_t> wp16;
  writePointer<uint32_t> wp32;
};
} // namespace Meeps