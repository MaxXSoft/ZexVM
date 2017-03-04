#ifndef ZVM_INTERRUPT_H_
#define ZVM_INTERRUPT_H_

#include <array>
#include <vector>
#include <functional>
#include <cstddef>

#include "type.h"

namespace zvm {

using IntFuncArg = std::array<Register, kArgRegisterCount>;
using IntFuncMem = std::array<char, kMemorySize>;
using IntFunc = std::function<ZValue(const IntFuncArg &, IntFuncMem &)>;

class InterruptManager {
public:
    InterruptManager();
    ~InterruptManager() {}

    int RegisterInterrupt(IntFunc func);
    void TriggerInterrupt(unsigned int index, std::array<Register, kRegisterCount> &reg, std::array<char, kMemorySize> &mem);

private:
    std::vector<IntFunc> func_vector_;
};

}

#endif // ZVM_INTERRUPT_H_
