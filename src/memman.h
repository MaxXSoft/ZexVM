#ifndef ZVM_MEMMAN_H_
#define ZVM_MEMMAN_H_

#include "gc.h"

#include <memory>

namespace zvm {

class MemoryManager {
public:
    MemoryManager(unsigned int memory_size, unsigned int stack_size)
            : mem_size_(memory_size), stack_size_(stack_size) { ResetMemory(); }
    ~MemoryManager() {}

    Register ReadMemory(unsigned int position) {
        if (position >= mem_size_) return {0};
        return *(Register *)(mem_.get() + position);
    }

    bool WriteMemory(unsigned int position, Register value) {
        if (position >= mem_size_) return false;
        *(Register *)(mem_.get() + position) = value;
        return true;
    }

    bool Push(Register value) {
        if (stack_ptr_ >= stack_size_ - sizeof(Register)) return false;
        *(Register *)(stack_.get() + stack_ptr_) = value;
        stack_ptr_ += sizeof(Register);
        return true;
    }

    Register Pop() {
        if (stack_ptr_ < sizeof(Register)) return {0};
        stack_ptr_ -= sizeof(Register);
        return *(Register *)(stack_.get() + stack_ptr_);
    }

    Register Peek(unsigned int offset = 0) {
        if (stack_ptr_ - offset < sizeof(Register)) return {0};
        return *(Register *)(stack_.get() + stack_ptr_ - offset);
    }

    char &operator[](unsigned int index) {   // exposes mem_ to the outside
        if (index >= mem_size_) index = 0;   // TODO: dangerous
        return mem_[index];
    }

    /*
    GC:
        AddStringObj
        AddListObj
        DelStringObj
        DelListObj

        GetStringObj
        GetListObj
        SetStringObj
        SetListObj

        StringCompare
        StringCatenate
        StringLength
        StringCopy

        ListCompare
        ListCatenate
        ListLength
        ListCopy
    */

    unsigned int memory_size() const { return mem_size_; }
    unsigned int stack_size() const { return stack_size_; }

    // void set_memory_size(unsigned int memory_size) { mem_size_ = memory_size; }
    // void set_stack_size(unsigned int stack_size) { stack_size_ = stack_size; }

private:
    void ResetMemory();

    std::unique_ptr<char[]> mem_, stack_;
    unsigned int stack_ptr_;
    unsigned int mem_size_, stack_size_;
};

} // namespace zvm

#endif // ZVM_MEMMAN_H_
