#ifndef ZVM_TYPE_H_
#define ZVM_TYPE_H_

namespace zvm {

union Number {
    long long long_long;
    double doub;
};

// that means String and List have the same structure
struct String {
    unsigned int reserved;
    unsigned int position;
};

struct List {
    unsigned int reserved;
    unsigned int position;
};

// that means Function structure can be read as a List directly
struct Function {
    unsigned int position;
    unsigned int env_pointer;
};

union ZValue {
    Number num;
    String str;
    List list;
    Function func;
};

const unsigned int kMemorySize = 1024 * 32;   // 32k
const unsigned int kStackSize = 1024 * 16;    // 16k
const unsigned int kCacheSize = 1024 * 512;   // 512k
const unsigned int kGCPoolSize = 1024 * 128;  // 128k

const char kRegisterCount = 16;
const char kArgRegisterCount = 6;
const char kArgRegisterOffset = 8;

const char kBytecodeHeaderLength = sizeof(unsigned char) * 5 + sizeof(unsigned int) * 4;
const unsigned char kCurrentVersion[2] = {0, 7};
const unsigned char kMinimumVersion[2] = {0, 7};

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
