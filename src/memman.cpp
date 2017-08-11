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
    return {0, id};
}

String MemoryManager::AddStringObj(const std::string &str) {
    auto id = gc_.AddObjFromMemory(str.c_str(), str.length() + 1);
    if (gc_.gc_error()) {
        mem_error_ = true;
        return {0, 0};
    }
    return {0, id};
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
    return {0, id};
}

List MemoryManager::AddListObj(const ZValue *data, MemSizeT length) {
    auto id = gc_.AddObjFromMemory((char *)data, length * sizeof(Register));
    if (gc_.gc_error()) {
        mem_error_ = true;
        return {0, 0};
    }
    return {0, id};
}

bool MemoryManager::DelStringObj(String str) {
    return gc_.DeleteObj(str.position);
}

bool MemoryManager::DelListObj(List list) {
    return gc_.DeleteObj(list.position);
}

const char *MemoryManager::GetRawString(String str) {
    auto obj = gc_.AccessObj(str.position);
    if (gc_.gc_error()) mem_error_ = true;
    return obj;
}

bool MemoryManager::SetRawString(String &str, const char *data) {
    gc_.DeleteObj(str.position);
    gc_.AddObjFromMemory(data, strlen(data) + 1);
    if (gc_.gc_error()) return !(mem_error_ = true);
    return true;
}

bool MemoryManager::GetStringObj(String str, MemSizeT position) {
    auto len = StringLength(str);
    if (mem_error_) return false;
    if (position + len + 1 >= mem_size_) return !(mem_error_ = true);
    auto obj = gc_.AccessObj(str.position);
    MemSizeT i;
    for (i = 0; i < len; ++i) {
        mem_[i + position] = obj[i];
    }
    mem_[i + position] = '\0';
    return true;
}

bool MemoryManager::SetStringObj(String &str, MemSizeT position) {
    if (position >= mem_size_) return !(mem_error_ = true);
    return SetRawString(str, &mem_[position]);
}

Register MemoryManager::GetListItem(List list, MemSizeT index) {
    auto obj = gc_.AccessObj(list.position);
    if (!obj || index >= ListLength(list)) {
        mem_error_ = true;
        return {0};
    }
    return *((Register *)obj + index);
}

bool MemoryManager::SetListItem(List list, MemSizeT index, Register value) {
    auto obj = gc_.AccessObj(list.position);
    if (!obj || index >= ListLength(list)) return !(mem_error_ = true);
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
    auto obj1 = gc_.AccessObj(str1.position);
    auto obj2 = gc_.AccessObj(str2.position);
    return !strcmp(obj1, obj2);
}

bool MemoryManager::StringCatenate(String str1, String str2) {
    auto obj2 = gc_.AccessObj(str2.position);
    if (!obj2) return !(mem_error_ = true);
    auto data_len = StringLength(str2) + 1;
    if (mem_error_) return false;
    if (!gc_.ExpandObj(str1.position, obj2, data_len, 1)) return !(mem_error_ = true);
    return true;
}

MemSizeT MemoryManager::StringLength(String str) {
    auto temp = gc_.GetObjLength(str.position) - 1;
    if (gc_.gc_error()) mem_error_ = true;
    return temp;
}

String MemoryManager::StringCopy(String str) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        String str = {0, 0};
        return str;
    };
    auto obj = gc_.AccessObj(str.position);
    if (!obj) return ReturnError();
    auto len = StringLength(str);
    if (mem_error_) return ReturnError();
    auto id = gc_.AddObj(len + 1);
    if (gc_.gc_error()) return ReturnError();
    auto new_obj = gc_.AccessObj(id);
    for (MemSizeT i = 0; i < len + 1; ++i) {
        new_obj[i] = obj[i];
    }
    return {0, id};
}

bool MemoryManager::ListCompare(List list1, List list2) {
    auto len1 = ListLength(list1);
    if (mem_error_ || len1 != ListLength(list2)) return false;
    auto obj1 = (Register *)gc_.AccessObj(list1.position);
    auto obj2 = (Register *)gc_.AccessObj(list2.position);
    if (!obj1 || !obj2) return false;
    for (MemSizeT i = 0; i < len1; ++i) {
        if (obj1[i].long_long != obj2[i].long_long) return false;
    }
    return true;
}

bool MemoryManager::ListCatenate(List list1, List list2) {
    auto obj2 = gc_.AccessObj(list2.position);
    if (!obj2) return !(mem_error_ = true);
    auto data_len = ListLength(list2) * sizeof(Register);
    if (mem_error_) return false;
    if (!gc_.ExpandObj(list1.position, obj2, data_len)) return !(mem_error_ = true);
    return true;
}

MemSizeT MemoryManager::ListLength(List list) {
    auto temp = gc_.GetObjLength(list.position) / sizeof(Register);
    if (gc_.gc_error()) mem_error_ = true;
    return temp;
}

List MemoryManager::ListCopy(List list) {
    auto ReturnError = [this]() {
        mem_error_ = true;
        List list = {0, 0};
        return list;
    };
    auto obj = gc_.AccessObj(list.position);
    if (!obj) return ReturnError();
    auto len = ListLength(list);
    if (mem_error_) return ReturnError();
    auto id = gc_.AddObj(len * sizeof(Register));
    if (gc_.gc_error()) return ReturnError();
    auto new_obj = gc_.AccessObj(id);
    for (MemSizeT i = 0; i < len * sizeof(Register); ++i) {
        new_obj[i] = obj[i];
    }
    return {0, id};
}

} // namespace zvm

