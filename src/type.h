#ifndef ZVM_TYPE_H_
#define ZVM_TYPE_H_

namespace zvm {

union Number {
    long long long_long;
    double doub;
};

struct String {
    unsigned int len;   // without '\0'
    unsigned int position;
};

struct List {
    unsigned int len;   // count of items in a list
    unsigned int position;
};

struct Function {
    unsigned char reserved;
    unsigned char arg_count;
    unsigned short arg_stack_pointer;
    // unsigned int env_pointer;
    unsigned int position;
};

union ZValue {
    Number num;
    String str;
    List list;
    Function func;
};

enum ZValueType : char {
    kNumber, 
    kString, 
    kList, 
    kFunction
};

struct Value {
    ZValue value;
    char type;
};

const unsigned int kMemorySize = 1024 * 32;   // 32k
const unsigned int kStackSize = 1024 * 16;    // 16k
const unsigned int kCacheSize = 1024 * 512;   // 512k
const unsigned int kGCPoolSize = 1024 * 128;  // 128k

const char kRegisterCount = 16;
const char kArgRegisterCount = 6;
const char kArgRegisterOffset = 8;

const char kBytecodeHeaderLength = sizeof(unsigned char) * 5 + sizeof(unsigned int) * 4;
const unsigned char kCurrentVersion[2] = {0, 5};
const unsigned char kMinimumVersion[2] = {0, 4};

enum VMReturnCode {
    kFinished, 
    kProgramError, 
    kStackError, 
    kMemoryError, 
    kCacheError
};

using Register = Number;
using MemSizeT = unsigned int;   // type that can storage memory size

} // namespace zvm

#endif // ZVM_TYPE_H_
