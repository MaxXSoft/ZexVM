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

    explicit GCObject(MemSizeT position, MemSizeT length)
            : position_(position), length_(length) {}
    // move constructor
    GCObject(GCObject &&gco) noexcept
            : position_(gco.position_), length_(gco.length_),
              reachable_(gco.reachable_), elem_list_(std::move(gco.elem_list_)) {}
    GCObject(const GCObject &gco) = delete;
    ~GCObject() {}

    // move assignment operator
    GCObject &operator=(GCObject &&gco) noexcept {
        if (this != &gco) {
            position_ = gco.position_;
            length_ = gco.length_;
            reachable_ = gco.reachable_;
            elem_list_ = std::move(gco.elem_list_);
        }
        return *this;
    }
    GCObject &operator=(const GCObject &gco) = delete;

    MemSizeT position() const { return position_; }
    MemSizeT length() const { return length_; }
    bool reachable() const { return reachable_; }
    const ElemList &elem_list() const { return elem_list_; }

    void set_position(MemSizeT position) { position_ = position; }
    void set_reachable(bool reachable) { reachable_ = reachable; }

    // will not check if id is repeated
    void AddElem(unsigned int id) { elem_list_.push_back(id); }

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
    using ObjSet = std::map<unsigned int, gc::GCObject>;

    GarbageCollector(MemSizeT pool_size) : pool_size_(pool_size) { ResetGC(); }
    ~GarbageCollector() {}

    void ResetGC();

    unsigned int AddObj(MemSizeT length);
    unsigned int AddObjFromMemory(const char *position, MemSizeT length);
    bool ExpandObj(unsigned int id, const char *data_pos, MemSizeT data_len, MemSizeT overlay = 0);
    bool DeleteObj(unsigned int id);

    void SetRootObj(unsigned int id) { root_id_ = id; }
    void AddElem(unsigned int obj_id, unsigned int elem_id);
    void DelElem(unsigned int obj_id, unsigned int elem_id);

    char *AccessObj(unsigned int id);
    MemSizeT GetObjLength(unsigned int id);

    bool gc_error() const { return gc_error_; }
    MemSizeT pool_size() const { return pool_size_; }

private:
    bool Reallocate(MemSizeT need_size);
    void Trace(unsigned int id);

    // garbage collector must ensure that when you add an object after
    // you deleted another object, GetId will return the id of the object
    // you just deleted
    unsigned int GetId();

    bool gc_error_;
    MemSizeT pool_size_, gc_stack_ptr_;
    unsigned int obj_id_, root_id_;
    std::unique_ptr<char[]> gc_pool_, temp_pool_;
    // map: <id, GCObject>
    ObjSet obj_set_;
    std::deque<unsigned int> free_id_;
};

} // namespace zvm

#endif // ZVM_GC_H_
