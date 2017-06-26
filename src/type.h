#ifndef ZVM_TYPE_H_
#define ZVM_TYPE_H_

namespace zvm {

union Number {
    long long long_long;
    double doub;
};

struct String {
    unsigned long long position;
};

struct List {
    unsigned int len;
    unsigned int position;
};

struct Function {
    unsigned char reserved;
    unsigned char arg_count;
    unsigned short arg_stack_pointer;
    unsigned int position;
};

union ZValue {
    Number num;
    String str;
    List list;
    Function func;
};

enum ZValueType {
    kNumber, 
    kString, 
    kList, 
    kFunction
};

struct Value {
    ZValue value;
    char type;
};

const unsigned int kMemorySize = 1024 * 128;
const unsigned int kCacheSize = 1024 * 512;

const char kRegisterCount = 16;
const char kArgRegisterCount = 6;
const char kArgRegisterOffset = 8;

const char kBytecodeHeaderLength = sizeof(unsigned char) * 5 + sizeof(unsigned int) * 4;
const unsigned char kCurrentVersion[2] = {0, 3};

enum VMReturnCode {
    kFinished, 
    kProgramError, 
    kStackError, 
    kMemoryError, 
    kCacheError
};

using Register = Number;

}

#endif // ZVM_TYPE_H_
