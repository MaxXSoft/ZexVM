#include "memman.h"

void MemoryManager::ResetMemory() {
    mem_ = std::make_unique<char[]>(mem_size_);
    stack_ = std::make_unique<char[]>(stack_size_);
    stack_ptr_ = 0;
}
