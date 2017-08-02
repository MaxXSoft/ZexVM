#include "gc.h"

namespace zvm {

bool GarbageCollector::Reallocate(unsigned int need_size) {
    temp_pool_ = std::make_unique<char[]>(pool_size_);
    gc_stack_ptr_ = 0;
    auto total_size = need_size;

    for (auto &&i : obj_set_) {
        auto &value = i.second;
        total_size += value.second;
        // completely full
        if (total_size > pool_size_) return false;

        // copy to new pool
        for (unsigned int j = 0; j < value.second; ++j) {
            temp_pool_[gc_stack_ptr_ + j] = gc_pool_[value.first + j];
        }
        value.first = gc_stack_ptr_;
        gc_stack_ptr_ += value.second;
    }
    
    gc_pool_ = std::move(temp_pool_);
    return true;
}

void GarbageCollector::ResetGC()  {
    gc_pool_ = std::make_unique<char[]>(pool_size_);
    gc_stack_ptr_ = 0;
    obj_id_ = 0;
    gc_error_ = false;
}

unsigned int GarbageCollector::AddObjFromMemory(char *position, unsigned int length) {
    // full GC
    if (gc_stack_ptr_ + length >= pool_size_) {
        if (!Reallocate(length)) {   // completely full
            gc_error_ = true;
            return 0xFFFFFFFF;
        }
    }

    auto new_id = GetId();
    // run out of obj id
    if (obj_id_ == 0xFFFFFFFF) {
        gc_error_ = true;
        return obj_id_;
    }

    // copy to GC pool
    for (unsigned int i = 0; i < length; ++i) {
        gc_pool_[gc_stack_ptr_ + i] = position[i];
    }

    obj_set_.insert({new_id, {gc_stack_ptr_, length}});
    gc_stack_ptr_ += length;
    return new_id;
}

bool GarbageCollector::DeleteObj(unsigned int id) {
    auto it = obj_set_.find(id);
    if (it != obj_set_.end()) {
        obj_set_.erase(it);
        return true;
    }
    else {
        return false;
    }
}

} // namespace zvm
