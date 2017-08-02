#include "memman.h"

namespace zvm {

void MemoryManager::ResetMemory() {
    mem_ = std::make_unique<char[]>(mem_size_);
    stack_ = std::make_unique<char[]>(stack_size_);
    stack_ptr_ = 0;
    mem_error_ = false;
}

} // namespace zvm

