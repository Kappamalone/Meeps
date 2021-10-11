#pragma once
#include <array>
#include <unicorn/unicorn.h>
#include "fmt/core.h"

#define MIPS_CODE_EL "\x56\x34\x21\x34" // ori $at, $at, 0x3456;

static void hook_block(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
    printf(">>> Tracing basic block at 0x%" PRIx64 ", block size = 0x%x\n", address, size);
}

static void hook_code(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
    printf(">>> Tracing instruction at 0x%" PRIx64 ", instruction size = 0x%x\n", address, size);
}

class UnicornMIPS {
public:
  UnicornMIPS() {
    // Initialize emulator in MIPS mode
    uc_open(UC_ARCH_MIPS, (uc_mode)(UC_MODE_MIPS32 + UC_MODE_LITTLE_ENDIAN), &uc);

    // map memory for emulation
    uc_mem_map(uc, START_ADDRESS, MEM_SIZE, UC_PROT_ALL);
  }

  ~UnicornMIPS() {
    uc_close(uc);
  }

  uint32_t* ExecuteInstructions(uint8_t* instrMem, size_t size) {
    ResetState();

    // write machine code to be emulated to memory
    uc_mem_write(uc, START_ADDRESS, instrMem, size);
    
    // emulate machine code in infinite time (last param = 0), or when
    // finishing all the code.
    uc_err err = uc_emu_start(uc, START_ADDRESS, START_ADDRESS + sizeof(instrMem), 0, 0);
    if (err) {
        printf("Failed on uc_emu_start() with error returned: %u (%s)\n", err, uc_strerror(err));
    }

    ReadIntoGPR();
    return gpr.data();
  }

private:
  void ResetState() {
    int resetPC = START_ADDRESS;
    uc_reg_write(uc, UC_MIPS_REG_PC, &resetPC); // reset pc

    int resetValue = 0;
    for (auto i = 0; i < 32; i++) {
      uc_reg_write(uc, UC_MIPS_REG_0 + i, &resetValue); // reset regs
    }

    static uint8_t resetArray[MEM_SIZE];
    uc_mem_write(uc, START_ADDRESS, resetArray, MEM_SIZE); // reset memory
  }

  // Read Unicorn reg values
  void ReadIntoGPR() {
    for (auto i = 0; i < 32; i++) {
      uc_reg_read(uc, UC_MIPS_REG_0 + i, &gpr[i]);
    }
  }

  uc_engine* uc;
  std::array<uint32_t, 32> gpr;
  static constexpr uint64_t START_ADDRESS = 0;
  static constexpr uint64_t MEM_SIZE = 2 * 1024 * 1024;
};
