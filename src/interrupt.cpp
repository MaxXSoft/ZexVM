#include "interrupt.h"

#include <cstdio>
#include <ctime>

namespace {

zvm::ZValue null_value, temp;

zvm::ZValue PutChar(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    fputc((int)(arg[0].long_long & 0xFF), stderr);
    return null_value;
}

zvm::ZValue GetChar(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num.long_long = (long long)getchar();
    return temp;
}

zvm::ZValue PutInteger(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    fprintf(stderr, "%lld", arg[0].long_long);
    return null_value;
}

zvm::ZValue PutFloat(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    fprintf(stderr, "%lf", arg[0].doub);
    return null_value;
}

zvm::ZValue PutString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num = arg[0];
    fprintf(stderr, "%s", mem.data() + temp.str.position);
    return null_value;
}

// zvm::ZValue PutRawString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
//     fprintf(stderr, "%s", mem.data() + arg[0].long_long);
//     return null_value;
// }

zvm::ZValue GetInteger(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num.long_long = 0;
    scanf("%lld", &temp.num.long_long);
    return temp;
}

zvm::ZValue GetFloat(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num.doub = 0;
    scanf("%lf", &temp.num.doub);
    return temp;
}

zvm::ZValue GetString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num = arg[0];
    scanf("%s", mem.data() + temp.str.position);
    return null_value;
}

// zvm::ZValue GetRawString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
//     scanf("%s", mem.data() + arg[0].long_long);
//     return null_value;
// }

zvm::ZValue AddChar(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    putchar((int)(arg[0].long_long & 0xFF));
    return null_value;
}

zvm::ZValue AddString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num = arg[0];
    printf("%s", mem.data() + temp.str.position);
    return null_value;
}

// zvm::ZValue AddRawString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
//     printf("%s", mem.data() + arg[0].long_long);
//     return null_value;
// }

zvm::ZValue Flush(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    if (arg[0].long_long == 0) {
        fflush(stdout);
    }
    else {
        fflush(stdin);
    }
    return null_value;
}

zvm::ZValue GetMillisecond(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num.long_long = (long long)clock() * 1000 / CLOCKS_PER_SEC;
    return temp;
}

zvm::ZValue Sleep(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    auto now = clock();
    auto duration = arg[0].long_long * CLOCKS_PER_SEC / 1000;
    while (clock() - now < duration);
    return null_value;
}

zvm::ZValue OpenFile(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    // arg[0] : file path
    // arg[1] : 0 -> read, 1 -> write, 2 -> read & write
    temp.num = arg[0];
    const char *file_mode[] = {"rb", "wb", "wb+"};
    FILE *file_ptr = fopen(mem.data() + temp.str.position, file_mode[arg[1].long_long]);
    temp.num.long_long = (long long)file_ptr;
    return temp;
}

zvm::ZValue CloseFile(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num.long_long = fclose((FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue ReadByte(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    char buffer;
    fread(&buffer, sizeof(char), 1, (FILE *)arg[0].long_long);
    temp.num.long_long = (long long)buffer;
    return temp;
}

zvm::ZValue ReadReg(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    fread(&temp.num.long_long, sizeof(long long), 1, (FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue WriteByte(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    char buffer = arg[1].long_long & 0xFF;
    temp.num.long_long = (long long)fwrite(&buffer, sizeof(char), 1, (FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue WriteReg(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num.long_long = (long long)fwrite(&arg[1].long_long, sizeof(long long), 1, (FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue Tell(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    temp.num.long_long = (long long)ftell((FILE *)arg[0].long_long);
    return temp;
}

zvm::ZValue Seek(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
    // arg[0] : file pointer
    // arg[1] : offset
    // arg[2] : 0 -> set, 1 -> cur, 2 -> end
    auto seek_mode = arg[2].long_long == 0 ? SEEK_SET : arg[2].long_long == 1 ? SEEK_CUR : SEEK_END;
    temp.num.long_long = (long long)fseek((FILE *)arg[0].long_long, (long int)arg[1].long_long, seek_mode);
    return temp;
}

}

namespace zvm {

InterruptManager::InterruptManager() {
    null_value.num.long_long = 0;
    RegisterInterrupt(PutChar);
    RegisterInterrupt(GetChar);
    RegisterInterrupt(PutInteger);
    RegisterInterrupt(PutFloat);
    RegisterInterrupt(PutString);
    RegisterInterrupt(GetInteger);
    RegisterInterrupt(GetFloat);
    RegisterInterrupt(GetString);
    RegisterInterrupt(AddChar);
    RegisterInterrupt(AddString);
    RegisterInterrupt(Flush);
    RegisterInterrupt(GetMillisecond);
    RegisterInterrupt(Sleep);
    RegisterInterrupt(OpenFile);
    RegisterInterrupt(CloseFile);
    RegisterInterrupt(ReadByte);
    RegisterInterrupt(ReadReg);
    RegisterInterrupt(WriteByte);
    RegisterInterrupt(WriteReg);
    RegisterInterrupt(Tell);
    RegisterInterrupt(Seek);
}

int InterruptManager::RegisterInterrupt(IntFunc func) {
    func_vector_.push_back(func);
    return (int)func_vector_.size() - 1;
}

void InterruptManager::TriggerInterrupt(unsigned int index, std::array<Register, kRegisterCount> &reg, std::array<char, kMemorySize> &mem) {
    if (index > func_vector_.size() - 1) return;

    IntFuncArg arg;
    for (int i = 0; i < kArgRegisterCount; ++i) {
        arg[i] = reg[i + kArgRegisterOffset];
    }
    ZValue ret = func_vector_[index](arg, mem);
    reg[kArgRegisterOffset + kArgRegisterCount] = ret.num;
}

}
