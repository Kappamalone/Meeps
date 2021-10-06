#pragma once
#include "types.h"

namespace Meeps
{
union Instruction
{
  uint32_t value;
  Instruction(uint32_t value) : value(value) {}

  // Immediate type opcodes
  struct
  {
    unsigned imm : 16;
    unsigned rt : 5;
    unsigned rs : 5;
    unsigned op : 6;
  } i;

  // Jump type opcodes
  struct
  {
    unsigned target : 26;
    unsigned op : 6;
  } j;

  // Register type opcodes
  struct
  {
    unsigned func : 6;
    unsigned shamt : 5;
    unsigned rd : 5;
    unsigned rt : 5;
    unsigned rs : 5;
    unsigned op : 6;
  } r;
};

class R3000Interpreter
{
public:
};

}  // namespace Meeps