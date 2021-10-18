#pragma once
#include <cop0.h>
#include <array>

class TestCOP0 : public Meeps::COP0 {
public:
    TestCOP0() {
        gpr.fill(0);
    }

    uint32_t GetReg(size_t reg) override {
        return gpr[reg];
    }

    void SetReg(size_t reg, uint32_t value) override {
        gpr[reg] = value;
    }

    std::array<uint32_t, 64> gpr;
};
