#ifndef ZVM_ZVM_H_
#define ZVM_ZVM_H_

#include <fstream>
#include <array>
#include <vector>

#include "type.h"
#include "memman.h"
#include "interrupt.h"

namespace zvm {

class ZexVM {
public:
    ZexVM(MemSizeT gc_pool_size, InterruptManager &int_manager)
            : mem_(gc_pool_size), int_manager_(int_manager) { Initialize(); }
    ~ZexVM() {}

    bool LoadProgram(std::ifstream &file);
    bool SetStartupArguments(const std::vector<std::string> &arg_list);
    int Run();

    bool program_error() const { return program_error_; }

private:
    void Initialize();

    bool program_error_;
    std::array<Register, kRegisterCount> reg_;
    std::array<char, kCacheSize> cache_;
    MemoryManager mem_;
    InterruptManager &int_manager_;
};

} // namespace zvm

#endif // ZVM_ZVM_H_
