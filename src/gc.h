#ifndef ZVM_GC_H_
#define ZVM_GC_H_

#include "type.h"

#include <memory>
#include <utility>
#include <map>
#include <deque>

namespace zvm {

// namespace gc {

// class GCObject {
// public:
//     GCObject(char *mem, MemSizeT length)
//             : mem_(mem), length_(length) {}
//     ~GCObject() {}

//     char &operator[](MemSizeT index) {
//         if (index >= length_) return undefined_[0];
//         return mem_[index];
//     }

//     const char &operator[](MemSizeT index) const {
//         if (index >= length_) return undefined_[0];
//         return mem_[index];
//     }

//     Register &operator()(MemSizeT index) {
//         if (index * sizeof(Register) >= length_) return *(Register *)undefined_;
//         return *(Register *)(mem_ + index * sizeof(Register));
//     }

//     const Register &operator()(MemSizeT index) const {
//         if (index * sizeof(Register) >= length_) return *(Register *)undefined_;
//         return *(Register *)(mem_ + index * sizeof(Register));
//     }

//     MemSizeT length() const { return length_; }

// private:
//     char *mem_;
//     char undefined_[sizeof(Register)];
//     MemSizeT length_;
// };

// } // namespace gc

class GarbageCollector {
public:
    GarbageCollector(MemSizeT pool_size) : pool_size_(pool_size) { ResetGC(); }
    ~GarbageCollector() {}

    void ResetGC();

    unsigned int AddObj(MemSizeT length);
    unsigned int AddObjFromMemory(char *position, MemSizeT length);
    bool DeleteObj(unsigned int id);

    char *AccessObj(unsigned int id);
    MemSizeT GetObjLength(unsigned int id);

    // bool FindId(unsigned int id) { return obj_set_.find(id) != obj_set_.end(); }

    bool gc_error() const { return gc_error_; }
    MemSizeT pool_size() const { return pool_size_; }

private:
    bool Reallocate(MemSizeT need_size);
    unsigned int GetId();

    bool gc_error_;
    MemSizeT pool_size_, gc_stack_ptr_;
    unsigned int obj_id_;
    std::unique_ptr<char[]> gc_pool_, temp_pool_;
    // map: <id, <position, length>>
    std::map<unsigned int, std::pair<MemSizeT, MemSizeT>> obj_set_;
    std::deque<unsigned int> free_id_;
};

} // namespace zvm

#endif // ZVM_GC_H_
