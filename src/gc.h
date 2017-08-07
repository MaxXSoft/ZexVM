#ifndef ZVM_GC_H_
#define ZVM_GC_H_

#include <memory>
#include <utility>
#include <map>
#include <deque>

#include "type.h"

namespace zvm {

namespace gc {

class GCObject {
public:
    using ElemList = std::deque<unsigned int>;

    GCObject(MemSizeT position, MemSizeT length)
            : position_(position), length_(length), reachable_(false) {}
    ~GCObject() {}

    MemSizeT position() const { return position_; }
    MemSizeT length() const { return length_; }
    bool reachable() const { return reachable_; }
    const ElemList &elem_list() const { return elem_list_; }

    void set_position(MemSizeT position) { position_ = position; }
    void set_reachable(bool reachable) { reachable_ = reachable; }

    void AddElem(unsigned int id) {
        if (elem_list_.empty() || elem_list_.back() != id) {
            elem_list_.push_back(id);
        }
    }

    void DelElem(unsigned int id) {
        for (auto &&i : elem_list_) {
            if (i == id) {
                i = elem_list_.back();
                elem_list_.pop_back();
                break;
            }
        }
    }

private:
    MemSizeT position_, length_;
    bool reachable_;
    ElemList elem_list_;
};

} // namespace gc

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
