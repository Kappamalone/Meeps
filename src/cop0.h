#pragma once
#include "types.h"

namespace Meeps {
class COP0 {
public:
    virtual uint32_t GetReg(size_t reg) = 0;
    virtual void SetReg(size_t reg, uint32_t value) = 0;
};
} // namespace Meeps