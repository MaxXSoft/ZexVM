#include "interrupt.h"

#include <cstdio>
#include <ctime>
#include <string>
#include <iostream>

#include "xstl/str_hash.h"

namespace {

zvm::ZValue null_value, temp;

zvm::ZValue PutChar(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    fputc((int)(arg[0].long_long & 0xFF), stderr);
    return null_value;
}

zvm::ZValue GetChar(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num.long_long = (long long)getchar();
    return temp;
}

zvm::ZValue PutInteger(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    fprintf(stderr, "%lld", arg[0].long_long);
    return null_value;
}

zvm::ZValue PutFloat(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    fprintf(stderr, "%lf", arg[0].doub);
    return null_value;
}

zvm::ZValue PutString(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num = arg[0];
    fprintf(stderr, "%s", mem.GetRawString(temp.str));
    return null_value;
}

// TODO
zvm::ZValue PutRawString(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    fprintf(stderr, "%s", &mem[arg[0].long_long]);
    return null_value;
}

zvm::ZValue GetInteger(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num.long_long = 0;
    scanf("%lld", &temp.num.long_long);
    return temp;
}

zvm::ZValue GetFloat(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num.doub = 0;
    scanf("%lf", &temp.num.doub);
    return temp;
}

zvm::ZValue GetString(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num = arg[0];
    std::string temp_str;
    std::cin >> temp_str;
    mem.SetRawString(temp.str, temp_str.c_str());
    return temp;
}

// unsafe
// zvm::ZValue GetRawString(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
//     scanf("%s", &mem[arg[0].long_long]);
//     return null_value;
// }

zvm::ZValue AddChar(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    putchar((int)(arg[0].long_long & 0xFF));
    return null_value;
}

zvm::ZValue AddString(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num = arg[0];
    printf("%s", mem.GetRawString(temp.str));
    return null_value;
}

// TODO
zvm::ZValue AddRawString(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    printf("%s", &mem[arg[0].long_long]);
    return null_value;
}

zvm::ZValue Flush(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    switch (arg[0].long_long) {
        case 0: {
            fflush(stdin);
            break;
        }
        case 1: {
            fflush(stdout);
            break;
        }
        default: {
            fflush((FILE *)arg[0].long_long);
        }
    }
    return null_value;
}

zvm::ZValue GetMillisecond(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num.long_long = (long long)clock() * 1000 / CLOCKS_PER_SEC;
    return temp;
}

zvm::ZValue Sleep(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    auto now = clock();
    auto duration = arg[0].long_long * CLOCKS_PER_SEC / 1000;
    while (clock() - now < duration);
    return null_value;
}

zvm::ZValue OpenFile(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    // arg[0] : file path
    // arg[1] : 0 -> read, 1 -> write, 2 -> read & write
    temp.num = arg[0];
    const char *file_mode[] = {"rb", "wb", "wb+"};
    FILE *file_ptr = fopen(mem.GetRawString(temp.str), file_mode[arg[1].long_long]);
    temp.num.long_long = (long long)file_ptr;
    return temp;
}

zvm::ZValue CloseFile(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num.long_long = fclose((FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue ReadByte(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    char buffer;
    fread(&buffer, sizeof(char), 1, (FILE *)arg[0].long_long);
    temp.num.long_long = (long long)buffer;
    return temp;
}

zvm::ZValue ReadReg(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    fread(&temp.num.long_long, sizeof(long long), 1, (FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue WriteByte(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    char buffer = arg[1].long_long & 0xFF;
    temp.num.long_long = (long long)fwrite(&buffer, sizeof(char), 1, (FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue WriteReg(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num.long_long = (long long)fwrite(&arg[1].long_long, sizeof(long long), 1, (FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue Tell(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    temp.num.long_long = (long long)ftell((FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue Seek(zvm::IntFuncArg arg, zvm::IntFuncMem mem) {
    // arg[0] : file pointer
    // arg[1] : offset
    // arg[2] : 0 -> set, 1 -> cur, 2 -> end
    auto seek_mode = arg[2].long_long == 0 ? SEEK_SET : arg[2].long_long == 1 ? SEEK_CUR : SEEK_END;
    temp.num.long_long = (long long)fseek((FILE *)arg[0].long_long, (long int)arg[1].long_long, seek_mode);
    return temp;
}

} // namespace

namespace zvm {

InterruptManager::InterruptManager() {
    null_value.num.long_long = 0;
    RegisterInterrupt("PutChar", PutChar);
    RegisterInterrupt("GetChar", GetChar);
    RegisterInterrupt("PutInteger", PutInteger);
    RegisterInterrupt("PutFloat", PutFloat);
    RegisterInterrupt("PutString", PutString);
    RegisterInterrupt("GetInteger", GetInteger);
    RegisterInterrupt("GetFloat", GetFloat);
    RegisterInterrupt("GetString", GetString);
    RegisterInterrupt("AddChar", AddChar);
    RegisterInterrupt("AddString", AddString);
    RegisterInterrupt("Flush", Flush);
    RegisterInterrupt("GetMillisecond", GetMillisecond);
    RegisterInterrupt("Sleep", Sleep);
    RegisterInterrupt("OpenFile", OpenFile);
    RegisterInterrupt("CloseFile", CloseFile);
    RegisterInterrupt("ReadByte", ReadByte);
    RegisterInterrupt("ReadReg", ReadReg);
    RegisterInterrupt("WriteByte", WriteByte);
    RegisterInterrupt("WriteReg", WriteReg);
    RegisterInterrupt("Tell", Tell);
    RegisterInterrupt("Seek", Seek);
}

bool InterruptManager::RegisterInterrupt(const char *name, IntFunc func) {
    auto hash = (unsigned int)(xstl::StringHashRT(name) & 0xFFFFFFFF);
    if (func_set_.find(hash) == func_set_.end()) return false;
    func_set_.insert({hash, func});
    return true;
}

bool InterruptManager::TriggerInterrupt(unsigned int id, std::array<Register, kRegisterCount> &reg, IntFuncMem mem) {
    auto it = func_set_.find(id);
    if (it == func_set_.end()) return false;

    std::array<Register, kArgRegisterCount> arg;
    for (int i = 0; i < kArgRegisterCount; ++i) {
        arg[i] = reg[i + kArgRegisterOffset];
    }
    ZValue ret = it->second(arg, mem);
    reg[kArgRegisterOffset + kArgRegisterCount] = ret.num;   // RV = ret
    return true;
}

} // namespace zvm
