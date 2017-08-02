#ifndef ZVM_GC_H_
#define ZVM_GC_H_

#include <memory>
#include <utility>
#include <map>

namespace zvm {

class GarbageCollector {
public:
    GarbageCollector(unsigned int pool_size) : pool_size_(pool_size) { ResetGC(); }
    ~GarbageCollector() {}

    void ResetGC();

    unsigned int AddObjFromMemory(char *position, unsigned int length);
    bool DeleteObj(unsigned int id);

    bool gc_error() const { return gc_error_; }
    unsigned int pool_size() const { return pool_size_; }

private:
    bool Reallocate(unsigned int need_size);
    unsigned int GetId() { return obj_id_++; }

    bool gc_error_;
    unsigned int pool_size_, gc_stack_ptr_, obj_id_;
    std::unique_ptr<char[]> gc_pool_, temp_pool_;
    // map: <id, <position, length>>
    std::map<unsigned int, std::pair<unsigned int, unsigned int>> obj_set_;
};

} // namespace zvm

#endif // ZVM_GC_H_
