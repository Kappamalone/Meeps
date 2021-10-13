#pragma once
#include "fmt/core.h"
#include <array>
#include <unicorn/mips.h>
#include <unicorn/unicorn.h>

class UnicornMIPS {
public:
  UnicornMIPS() {
    // Initialize emulator in MIPS mode
    uc_open(UC_ARCH_MIPS, (uc_mode)(UC_MODE_MIPS32 + UC_MODE_LITTLE_ENDIAN),
            &uc);

    // map memory for emulation
    uc_mem_map(uc, START_ADDRESS, MEM_SIZE, UC_PROT_ALL);
  }

  ~UnicornMIPS() { uc_close(uc); }

  void Reset() {
    int resetPC = START_ADDRESS;
    uc_reg_write(uc, UC_MIPS_REG_PC, &resetPC); // reset pc

    int resetValue = 0;
    for (auto i = 0; i < 32; i++) {
      uc_reg_write(uc, UC_MIPS_REG_0 + i, &resetValue); // reset regs
    }

    static uint8_t resetArray[MEM_SIZE];
    uc_mem_write(uc, START_ADDRESS, resetArray, MEM_SIZE); // reset memory
  }

  uint32_t *ExecuteInstructions(uint8_t *instrMem, size_t size) {
    // write machine code to be emulated to memory
    uc_mem_write(uc, START_ADDRESS, instrMem, size);

    // emulate machine code in infinite time (last param = 0), or when
    // finishing all the code.
    uc_err err =
        uc_emu_start(uc, START_ADDRESS, START_ADDRESS + size, 0, size);
    if (err) {
      printf("Failed on uc_emu_start() with error returned: %u (%s)\n", err,
             uc_strerror(err));
      uint32_t val;
      uc_reg_read(uc, UC_MIPS_REG_PC, &val);
      fmt::print("Unicorn PC: {:08X}\n", val);
      //exit(1);
    }

    return GetAllGPR();
  }

  void SetGPR(size_t reg, int value) {
    if (reg) {
      uc_reg_write(uc, UC_MIPS_REG_0 + reg, &value);
    }
  }

  uint32_t* GetAllGPR() {
    ReadIntoGPR();
    return gpr.data();
  }

private:
  // Read Unicorn reg values
  void ReadIntoGPR() {
    for (auto i = 0; i < 32; i++) {
      uc_reg_read(uc, UC_MIPS_REG_0 + i, &gpr[i]);
    }
  }

  uc_engine *uc;
  std::array<uint32_t, 32> gpr;
  static constexpr uint64_t START_ADDRESS = 0;
  static constexpr uint64_t MEM_SIZE = 2 * 1024 * 1024;
};
