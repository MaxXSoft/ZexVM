#include "memman.h"

#include <cstring>

namespace zvm {

void MemoryManager::ResetMemory() {
    mem_ = std::make_unique<char[]>(mem_size_);
    stack_ = std::make_unique<char[]>(stack_size_);
    stack_ptr_ = 0;
    mem_error_ = false;
    if (gc_.gc_error()) gc_.ResetGC();   // TODO: ???
}

String MemoryManager::AddStringObj(MemSizeT position) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        String str = {0, 0};
        return str;
    };
    if (position >= mem_size_) return ReturnError();
    auto len = strlen(mem_.get() + position) + 1;   // with '\0'
    auto id = gc_.AddObjFromMemory(mem_.get() + position, len);
    if (gc_.gc_error()) return ReturnError();
    return {(unsigned int)len - 1, id};
}

List MemoryManager::AddListObj(MemSizeT position, MemSizeT length) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        List list = {0, 0};
        return list;
    };
    if (position + length * sizeof(Register) >= mem_size_) return ReturnError();
    auto id = gc_.AddObjFromMemory(mem_.get() + position, length * sizeof(Register));
    if (gc_.gc_error()) return ReturnError();
    return {length, id};
}

bool MemoryManager::DelStringObj(String str) {
    return gc_.DeleteObj(str.position);
}

bool MemoryManager::DelListObj(List list) {
    return gc_.DeleteObj(list.position);
}

const char *MemoryManager::GetStringObj(String str) {
    auto obj = gc_.AccessObj(str.position);
    return obj;
}

Register MemoryManager::GetListItem(List list, MemSizeT index) {
    auto obj = gc_.AccessObj(list.position);
    if (index >= list.len || !obj) {
        mem_error_ = true;
        return {0};
    }
    return *((Register *)obj + index);
}

bool MemoryManager::SetListItem(List list, MemSizeT index, Register value) {
    auto obj = gc_.AccessObj(list.position);
    if (index >= list.len || !obj) return !(mem_error_ = true);
    *((Register *)obj + index) = value;
    return true;
}

void MemoryManager::AddListRef(List list, List ref) {
    gc_.AddElem(list.position, ref.position);
    if (gc_.gc_error()) mem_error_ = true;
}

void MemoryManager::DelListRef(List list, List ref) {
    gc_.DelElem(list.position, ref.position);
    if (gc_.gc_error()) mem_error_ = true;
}

bool MemoryManager::StringCompare(String str1, String str2) {
    if (str1.len != str2.len) return false;
    auto obj1 = gc_.AccessObj(str1.position);
    auto obj2 = gc_.AccessObj(str2.position);
    return !strcmp(obj1, obj2);
}

String MemoryManager::StringCatenate(String str1, String str2) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        String str = {0, 0};
        return str;
    };

    auto obj1 = gc_.AccessObj(str1.position);
    auto obj2 = gc_.AccessObj(str2.position);
    if (!obj1 || !obj2) ReturnError();

    auto len = str1.len + str2.len + 1;
    auto id = gc_.AddObj(len);
    if (gc_.gc_error()) ReturnError();
    auto obj = gc_.AccessObj(id);

    while (obj1) *(obj++) = *(obj1++);
    while (obj2) *(obj++) = *(obj2++);
    *obj = '\0';

    gc_.DeleteObj(str1.position);
    gc_.DeleteObj(str2.position);
    return {len - 1, id};
}

Register MemoryManager::StringLength(String str) {
    return {str.len};
}

String MemoryManager::StringCopy(String str) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        String str = {0, 0};
        return str;
    };
    auto obj = gc_.AccessObj(str.position);
    if (!obj) return ReturnError();
    auto id = gc_.AddObj(str.len + 1);
    if (gc_.gc_error()) return ReturnError();
    auto new_obj = gc_.AccessObj(id);
    for (MemSizeT i = 0; i < str.len + 1; ++i) {
        new_obj[i] = obj[i];
    }
    return {str.len, id};
}

bool MemoryManager::ListCompare(List list1, List list2) {
    if (list1.len != list2.len) return false;
    auto obj1 = (Register *)gc_.AccessObj(list1.position);
    auto obj2 = (Register *)gc_.AccessObj(list2.position);
    if (!obj1 || !obj2) return false;
    for (MemSizeT i = 0; i < list1.len; ++i) {
        if (obj1[i].long_long != obj2[i].long_long) return false;
    }
    return true;
}

List MemoryManager::ListCatenate(List list1, List list2) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        List list = {0, 0};
        return list;
    };

    auto obj1 = (Register *)gc_.AccessObj(list1.position);
    auto obj2 = (Register *)gc_.AccessObj(list2.position);
    if (!obj1 || !obj2) ReturnError();

    auto len = list1.len + list2.len;
    auto id = gc_.AddObj(len * sizeof(Register));
    if (gc_.gc_error()) ReturnError();
    auto obj = (Register *)gc_.AccessObj(id);

    for (MemSizeT i = 0; i < list1.len; ++i) {
        obj[i] = obj1[i];
    }
    for (MemSizeT i = 0; i < list2.len; ++i) {
        obj[list1.len + i] = obj1[i];
    }

    gc_.DeleteObj(list1.position);
    gc_.DeleteObj(list2.position);
    return {len, id};
}

Register MemoryManager::ListLength(List list) {
    return {list.len};
}

List MemoryManager::ListCopy(List list) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        List list = {0, 0};
        return list;
    };
    auto obj = gc_.AccessObj(list.position);
    if (!obj) return ReturnError();
    auto id = gc_.AddObj(list.len * sizeof(Register));
    if (gc_.gc_error()) return ReturnError();
    auto new_obj = gc_.AccessObj(id);
    for (MemSizeT i = 0; i < list.len * sizeof(Register); ++i) {
        new_obj[i] = obj[i];
    }
    return {list.len, id};
}

} // namespace zvm

