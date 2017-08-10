#ifndef ZVM_INTERRUPT_H_
#define ZVM_INTERRUPT_H_

#include <array>
#include <map>
#include <functional>
#include <cstddef>

#include "type.h"
#include "memman.h"

namespace zvm {

using IntFuncArg = const std::array<Register, kArgRegisterCount> &;
using IntFuncMem = MemoryManager &;
using IntFunc = std::function<ZValue(IntFuncArg, IntFuncMem)>;

class InterruptManager {
public:
    InterruptManager();
    ~InterruptManager() {}


    bool RegisterInterrupt(const char *name, IntFunc func);
    bool TriggerInterrupt(unsigned int id, std::array<Register, kRegisterCount> &reg, IntFuncMem mem);

private:
    std::map<unsigned int, IntFunc> func_set_;
};

} // namespace zvm

#endif // ZVM_INTERRUPT_H_
