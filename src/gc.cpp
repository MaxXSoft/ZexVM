#include "gc.h"

namespace zvm {

bool GarbageCollector::Reallocate(MemSizeT need_size) {
    temp_pool_ = std::make_unique<char[]>(pool_size_);   // TODO
    gc_stack_ptr_ = 0;
    auto total_size = need_size;

    for (auto &&i : obj_set_) {
        auto &value = i.second;
        total_size += value.second;
        // completely full
        if (total_size > pool_size_) return false;

        // copy to new pool
        for (MemSizeT j = 0; j < value.second; ++j) {
            temp_pool_[gc_stack_ptr_ + j] = gc_pool_[value.first + j];
        }
        value.first = gc_stack_ptr_;
        gc_stack_ptr_ += value.second;
    }
    
    gc_pool_ = std::move(temp_pool_);
    return true;
}

unsigned int GarbageCollector::GetId()  {
    if (!free_id_.empty()) {   // reuse id that has already beed deleted
        auto id = free_id_.front();
        free_id_.pop_front();
        return id;
    }
    else {
        return obj_id_++;
    }
}

void GarbageCollector::ResetGC()  {
    gc_pool_ = std::make_unique<char[]>(pool_size_);
    gc_stack_ptr_ = 0;
    obj_id_ = 0;
    free_id_.clear();
    gc_error_ = false;
}

unsigned int GarbageCollector::AddObjFromMemory(char *position, MemSizeT length) {
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
    for (MemSizeT i = 0; i < length; ++i) {
        gc_pool_[gc_stack_ptr_ + i] = position[i];
    }

    obj_set_.insert({new_id, {gc_stack_ptr_, length}});
    gc_stack_ptr_ += length;
    return new_id;
}

bool GarbageCollector::DeleteObj(unsigned int id) {
    auto it = obj_set_.find(id);
    if (it != obj_set_.end()) {
        free_id_.push_back(it->first);
        obj_set_.erase(it);
        return true;
    }
    else {
        return false;
    }
}

std::unique_ptr<gc::GCObject> GarbageCollector::AccessObj(unsigned int id) {
    auto it = obj_set_.find(id);
    if (it == obj_set_.end()) return nullptr;
    const auto &value = it->second;
    return std::make_unique<gc::GCObject>(gc_pool_.get() + value.first, value.second);
}

} // namespace zvm
