#ifndef ZVM_MEMMAN_H_
#define ZVM_MEMMAN_H_

#include <memory>

#include "type.h"
#include "gc.h"

namespace zvm {

class MemoryManager {
public:
    MemoryManager(MemSizeT memory_size, MemSizeT stack_size,
                  MemSizeT gc_pool_size)
            : mem_size_(memory_size), stack_size_(stack_size),
              gc_(gc_pool_size) { ResetMemory(); }
    MemoryManager(MemSizeT gc_pool_size) : gc_(gc_pool_size) {}
    ~MemoryManager() {}

    void ResetMemory();

    bool Push(Register value) {
        if (stack_ptr_ >= stack_size_ - sizeof(Register)) return !(mem_error_ = true);
        *(Register *)(stack_.get() + stack_ptr_) = value;
        stack_ptr_ += sizeof(Register);
        return true;
    }

    Register Pop() {
        if (stack_ptr_ < sizeof(Register)) {
            mem_error_ = true;
            return {0};
        }
        stack_ptr_ -= sizeof(Register);
        return *(Register *)(stack_.get() + stack_ptr_);
    }

    Register Peek(MemSizeT offset = 0) {
        if (stack_ptr_ - offset < sizeof(Register)) {
            mem_error_ = true;
            return {0};
        }
        return *(Register *)(stack_.get() + stack_ptr_ - offset);
    }

    char &operator[](MemSizeT index) {   // exposes mem_ to the outside
        if (index >= mem_size_) {
            mem_error_ = true;
            return undefined_[0];
        }
        return mem_[index];
    }

    Register &operator()(MemSizeT index) {
        if (index + sizeof(Register) >= mem_size_) {
            mem_error_ = true;
            return *(Register *)undefined_;
        }
        return *(Register *)(mem_.get() + index);
    }

    String AddStringObj(MemSizeT position);
    List AddListObj(MemSizeT position, MemSizeT length);
    bool DelStringObj(String str);
    bool DelListObj(List list);

    const char *GetStringObj(String str);
    Register GetListItem(List list, MemSizeT index);
    bool SetListItem(List list, MemSizeT index, Register value);
    void SetRootEnv(List env) { gc_.SetRootObj(env.position); }
    void AddListRef(List list, List ref);
    void DelListRef(List list, List ref);

    bool StringCompare(String str1, String str2);
    String StringCatenate(String str1, String str2);
    Register StringLength(String str);
    String StringCopy(String str);

    bool ListCompare(List list1, List list2);
    List ListCatenate(List list1, List list2);
    Register ListLength(List list);
    List ListCopy(List list);

    bool mem_error() const { return mem_error_; }
    MemSizeT memory_size() const { return mem_size_; }
    MemSizeT stack_size() const { return stack_size_; }

    void set_memory_size(unsigned int memory_size) { mem_size_ = memory_size; }
    void set_stack_size(unsigned int stack_size) { stack_size_ = stack_size; }

private:
    GarbageCollector gc_;

    bool mem_error_;
    char undefined_[sizeof(Register)];
    std::unique_ptr<char[]> mem_, stack_;
    MemSizeT stack_ptr_;
    MemSizeT mem_size_, stack_size_;
};

} // namespace zvm

#endif // ZVM_MEMMAN_H_
