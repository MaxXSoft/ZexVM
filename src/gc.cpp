#include "gc.h"

namespace zvm {

bool GarbageCollector::Reallocate(MemSizeT need_size) {
    // mark unreachable object recursively
    Trace(root_id_);

    temp_pool_ = std::make_unique<char[]>(pool_size_);   // TODO
    gc_stack_ptr_ = 0;
    auto total_size = need_size;

    for (auto i = obj_set_.begin(); i != obj_set_.end(); ) {
        auto &gco = i->second;
        // sweep unreachable object
        if (!gco.reachable()) {
            free_id_.push_front(i->first);
            i = obj_set_.erase(i);
        }
        else {
            // reset reachable status
            gco.set_reachable(false);
            total_size += gco.length();
            // completely full
            //     notice that if program were not forced to stop now
            //     the reachable status would be wrong
            if (total_size > pool_size_) return false;
            // copy to new pool
            for (MemSizeT j = 0; j < gco.length(); ++j) {
                temp_pool_[gc_stack_ptr_ + j] = gc_pool_[gco.position() + j];
            }
            gco.set_position(gc_stack_ptr_);
            gc_stack_ptr_ += gco.length();
            ++i;   // increase iterator
        }
    }
    
    gc_pool_ = std::move(temp_pool_);
    return true;
}

void GarbageCollector::Trace(unsigned int id) {
    auto it = obj_set_.find(id);
    if (it == obj_set_.end()) return;
    auto &gco = it->second;
    gco.set_reachable(true);
    if (gco.elem_list().empty()) return;
    for (const auto &i : gco.elem_list()) {
        auto it = obj_set_.find(i);
        if (it != obj_set_.end() && !it->second.reachable()) Trace(i);
    }
}

unsigned int GarbageCollector::GetId()  {
    if (!free_id_.empty()) {   // reuse id that has already beed deleted
        auto id = free_id_.back();
        free_id_.pop_back();
        return id;
    }
    else {
        return obj_id_++;
    }
}

void GarbageCollector::ResetGC()  {
    gc_pool_ = std::make_unique<char[]>(pool_size_);
    gc_stack_ptr_ = 0;
    obj_id_ = root_id_ = 0;
    free_id_.clear();
    gc_error_ = false;
}

unsigned int GarbageCollector::AddObj(MemSizeT length) {
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

    obj_set_.insert(ObjSet::value_type(new_id, gc::GCObject(gc_stack_ptr_, length)));
    gc_stack_ptr_ += length;
    return new_id;
}

unsigned int GarbageCollector::AddObjFromMemory(const char *position, MemSizeT length) {
    auto new_id = AddObj(length);
    if (gc_error_) return new_id;

    // copy to GC pool
    for (MemSizeT i = 0; i < length; ++i) {
        gc_pool_[gc_stack_ptr_ - length + i] = position[i];
    }

    return new_id;
}

bool GarbageCollector::ExpandObj(unsigned int id, const char *data_pos, MemSizeT data_len, MemSizeT overlay) {
    auto it = obj_set_.find(id);
    if (it == obj_set_.end()) return !(gc_error_ = true);
    auto &obj = it->second;
    // calculate the length of the original object
    // after excluding the overlay
    auto obj_len = obj.length() - overlay;
    if (obj_len <= 0) return !(gc_error_ = true);

    MemSizeT start_pos;
    // object is not on the top of GC pool
    if (obj.position() + obj.length() != gc_stack_ptr_) {
        // allocate a new object
        auto new_id = AddObj(obj_len + data_len);
        if (gc_error_) return false;
        // copy the data whose length is obj_len
        // from the original object to the new object
        auto new_it = obj_set_.find(new_id);
        auto &new_obj = new_it->second;
        auto obj_pos = obj.position();
        start_pos = new_obj.position();
        for (MemSizeT i = 0; i < obj_len; ++i) {
            gc_pool_[start_pos + i] = gc_pool_[obj_pos + i];
        }
        // move the content of new object and the original one
        // and delete the new object
        obj = std::move(new_obj);   // TODO: test
        obj_set_.erase(new_it);
    }
    else {
        // just change stack pointer directly
        auto size = data_len - overlay;
        // full GC
        if (gc_stack_ptr_ + size >= pool_size_) {
            if (!Reallocate(size)) return !(gc_error_ = true);
        }
        gc_stack_ptr_ += size;
        start_pos = obj.position();
    }

    // copy the remaining data
    for (MemSizeT i = 0; i < data_len; ++i) {
        gc_pool_[start_pos + obj_len + i] = data_pos[i];
    }
    return true;
}

bool GarbageCollector::DeleteObj(unsigned int id) {
    auto it = obj_set_.find(id);
    if (it != obj_set_.end()) {
        free_id_.push_back(it->first);
        obj_set_.erase(it);
        auto &gco = it->second;
        // object is on the top of the GC pool
        if (gco.position() + gco.length() == gc_stack_ptr_) {
            // restore stack pointer
            gc_stack_ptr_ -= gco.length();
        }
        return true;
    }
    else {
        return !(gc_error_ = true);   // false
    }
}

void GarbageCollector::AddElem(unsigned int obj_id, unsigned int elem_id) {
    auto it = obj_set_.find(obj_id);
    if (it != obj_set_.end() && obj_set_.find(elem_id) != obj_set_.end()) {
        it->second.AddElem(elem_id);
    }
    else {
        gc_error_ = true;
    }
}

void GarbageCollector::DelElem(unsigned int obj_id, unsigned int elem_id) {
    auto it = obj_set_.find(obj_id);
    if (it != obj_set_.end()) {
        it->second.DelElem(elem_id);
    }
    else {
        gc_error_ = true;
    }
}


char *GarbageCollector::AccessObj(unsigned int id) {
    auto it = obj_set_.find(id);
    if (it == obj_set_.end()) {
        gc_error_ = true;
        return nullptr;
    }
    const auto &gco = it->second;
    return gc_pool_.get() + gco.position();
}

MemSizeT GarbageCollector::GetObjLength(unsigned int id) {
    auto it = obj_set_.find(id);
    if (it == obj_set_.end()) {
        gc_error_ = true;
        return 0;
    }
    return it->second.length();
}

} // namespace zvm
