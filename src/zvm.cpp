#include "zvm.h"

#include <cmath>
#include <cctype>
#include <cstdlib>
#include <string>
#include <cstring>

namespace {

enum InstOp {
    END,   // VM
    AND, XOR, OR, NOT, SHL, SHR,   // Bit
    ADD, ADDF, SUB, SUBF, MUL, MULF, DIV, DIVF, NEG, NEGF, MOD, POW,   // Math
    LT, LTF, GT, GTF, LE, LEF, GE, GEF, EQ, NEQ,   // Logic
    JMP, JZ, JNZ, CALL, RET,   // Jump
    MOV, MOVL, POP, PUSH, PEEK, LD, ST, STR, STC, INT,   // Basic
    NEWS, NEWL, DELS, DELL, SETR, ADR, RMR,   // GC
    // SETR (set root), ADR (add ref), RMR (remove ref)
    ITF, FTI, ITS, STI, FTS, STF,   // Convert
    ADDS, CPS, LENS, EQS, GETS, SETS,   // String
    ADDL, CPL, LENL, EQL, GETL, SETL   // List
// TODO: add: GETS, SETS, SETL
};

enum InstReg {
    IMM,   // marked as an immediate number
    R1, R2, R3, R4, R5, R6, R7,   // general registers
    A1, A2, A3, A4, A5, A6, RV,   // function registers
    // (A1-A6: args, RV: environment pointer and ret value)
    PC   // program counter
};

enum InstLen : zvm::MemSizeT {
    itVOID = sizeof(unsigned char), 
    itR = sizeof(unsigned char) * 2, 
    itI = sizeof(unsigned char) + sizeof(unsigned int), 
    itRR = sizeof(unsigned char) * 2, 
    itRI = sizeof(unsigned char) * 2 + sizeof(unsigned int), 
    itRIF = sizeof(unsigned char) * 2 + sizeof(double), 
    itII = sizeof(unsigned char) * 2 + sizeof(unsigned int) * 2, 
    itRRR = sizeof(unsigned char) * 3
};

union InstImm {
    unsigned int int_val;
    double fp_val;
};

#pragma pack(1)

struct VMInst {
    unsigned char op;
    unsigned char reg;
    InstImm imm;
};

#pragma pack()

template <typename T>
inline T CalcExpression(const T &opr1, const T &opr2, int Op) {
    switch (Op) {
        case AND: return (long long)opr1 & (long long)opr2;
        case XOR: return (long long)opr1 ^ (long long)opr2;
        case OR: return (long long)opr1 | (long long)opr2;
        case NOT: return ~(long long)opr1;
        case SHL: return (long long)opr1 << (long long)opr2;
        case SHR: return (long long)opr1 >> (long long)opr2;
        case ADD: case ADDF: return opr1 + opr2;
        case SUB: case SUBF: return opr1 - opr2;
        case MUL: case MULF: return opr1 * opr2;
        case DIV: case DIVF: return opr1 / opr2;
        case NEG: return -opr1;
        case MOD: return (long long)opr1 % (long long)opr2;
        case POW: return pow(opr1, opr2);
        case LT: case LTF: return opr1 < opr2;
        case GT: case GTF: return opr1 > opr2;
        case LE: case LEF: return opr1 <= opr2;
        case GE: case GEF: return opr1 >= opr2;
        case EQ: return opr1 == opr2;
        case NEQ: return opr1 != opr2;
        default: return 0;
    }
}

} // namespace

namespace zvm {

void ZexVM::Initialize() {
    program_error_ = true;
    reg_.fill({0});
    cache_.fill(0);
    if (mem_.mem_error()) mem_.ResetMemory();
}

bool ZexVM::LoadProgram(std::ifstream &file) {
    Initialize();

    if (!file.is_open()) return false;

    auto current = file.tellg();   // save current position
    file.seekg(0, std::ios::end);
    auto len = file.tellg();
    file.seekg(current);   // restore saved position
    if (len < kBytecodeHeaderLength) return false;
    
    file >> std::noskipws;

    char header[3], version[2];
    file >> header[0] >> header[1] >> header[2];
    file >> version[0] >> version[1];
    if (header[0] != '\x93' || header[1] != '\x94' || header[2] != '\x86') return false;
    if (version[0] > kCurrentVersion[0] || version[0] < kMinimumVersion[0]) {
        return false;
    }
    else if(version[1] > kCurrentVersion[1] || version[0] < kMinimumVersion[0]) {
        return false;
    }

    MemSizeT mem_size = 0, stack_size = 0, const_pool_size = 0, temp = 0;
    file.read((char *)&mem_size, sizeof(MemSizeT));
    file.read((char *)&stack_size, sizeof(MemSizeT));
    file.read((char *)&const_pool_size, sizeof(MemSizeT));
    file.read((char *)&temp, sizeof(MemSizeT));
    const_pool_size = temp - const_pool_size;
    if (const_pool_size >= mem_size) return false;
    mem_.set_memory_size(mem_size);
    mem_.set_stack_size(stack_size);
    mem_.ResetMemory();

    for (int i = 0; i < const_pool_size; ++i) {
        file >> mem_[i];
    }

    if ((MemSizeT)len - temp >= kCacheSize) return false;
    for (int i = 0; !file.eof(); ++i) {
        file >> cache_[i];
    }

    return !(program_error_ = false);
}

int ZexVM::Run() {
#define reg_x reg_[rx_index]
#define reg_y reg_[ry_index]
#define NEXT(inst_len) SwitchInst(inst_len); \
        if (reg_pc >= kCacheSize) goto _PERR; \
        goto *inst_list[inst->op]

    if (program_error_) return kProgramError;

    VMInst *inst = nullptr;
    ZValue temp;
    auto &reg_pc = reg_[PC].long_long;
    auto rx_index = 0, ry_index = 0;
    auto imm_mode = false;

    void *inst_list[] = {
        &&_END,
        &&_AND, &&_XOR, &&_OR, &&_NOT, &&_SHL, &&_SHR,
        &&_ADD, &&_ADDF, &&_SUB, &&_SUBF, &&_MUL, &&_MULF, &&_DIV, &&_DIVF, &&_NEG, &&_NEGF, &&_MOD, &&_POW,
        &&_LT, &&_LTF, &&_GT, &&_GTF, &&_LE, &&_LEF, &&_GE, &&_GEF, &&_EQ, &&_NEQ,
        &&_JMP, &&_JZ, &&_JNZ, &&_CALL, &&_RET,
        &&_MOV, &&_MOVL, &&_POP, &&_PUSH, &&_PEEK, &&_LD, &&_ST, &&_STR, &&_STC, &&_INT,
        &&_NEWS, &&_NEWL, &&_DELS, &&_DELL, &&_SETR, &&_ADR, &&_RMR,
        &&_ITF, &&_FTI, &&_ITS, &&_STI, &&_FTS, &&_STF,
        &&_ADDS, &&_CPS, &&_LENS, &&_EQS, &&_GETS, &&_SETS,
        &&_ADDL, &&_CPL, &&_LENL, &&_EQL, &&_GETL, &&_SETL
    };

    auto SwitchInst = [&](MemSizeT inst_len) {
        reg_pc += inst_len;
        inst = (VMInst *)(cache_.data() + reg_pc);
        rx_index = inst->reg >> 4;
        ry_index = inst->reg & 0x0F;
        imm_mode = !(inst->reg & 0x0F);
    };

    NEXT(0);   // start running

    _PERR: program_error_ = true; return kProgramError;
    _SERR: program_error_ = true; return kStackError;
    _MERR: program_error_ = true; return kMemoryError;
    _CERR: program_error_ = true; return kCacheError;
    _END: {
        return kFinished;
    }
    _AND: _XOR: _OR: _SHL: _SHR: _ADD: _SUB: _MUL: _DIV: _MOD: _LT:
    _GT: _LE: _GE: _EQ: _NEQ: {
        reg_x.long_long = CalcExpression(reg_x.long_long, (imm_mode ? (long long)inst->imm.int_val : reg_y.long_long), inst->op);
        NEXT(imm_mode ? itRI : itRR);
    }
    _ADDF: _SUBF: _MULF: _DIVF: _POW: {
        reg_x.doub = CalcExpression(reg_x.doub, (imm_mode ? inst->imm.fp_val : reg_y.doub), inst->op);
        NEXT(imm_mode ? itRIF : itRR);
    }
    _LTF: _GTF: _LEF: _GEF: {
        reg_x.long_long = (long long)CalcExpression(reg_x.doub, (imm_mode ? inst->imm.fp_val : reg_y.doub), inst->op);
        NEXT(imm_mode ? itRIF : itRR);
    }
    _NOT: _NEG: {
        reg_x.long_long = CalcExpression(reg_x.long_long, 0LL, inst->op);
        NEXT(itR);
    }
    _NEGF: {
        reg_x.doub = -reg_x.doub;
        NEXT(itR);
    }
    _JMP: {
        reg_pc = imm_mode ? inst->imm.int_val : reg_x.long_long;
        NEXT(0);
    }
    _JZ: {
        if (reg_x.long_long == 0) {
            reg_pc = imm_mode ? inst->imm.int_val : reg_x.long_long;
            NEXT(0);
        }
        else {
            NEXT(imm_mode ? itRI : itRR);
        }
    }
    _JNZ: {
        if (reg_x.long_long != 0) {
            reg_pc = imm_mode ? inst->imm.int_val : reg_x.long_long;
            NEXT(0);
        }
        else {
            NEXT(imm_mode ? itRI : itRR);
        }
    }
    _CALL: {
        temp.num.doub = imm_mode ? inst->imm.fp_val : reg_x.doub;
        reg_[RV].long_long = temp.func.env_pointer;
        reg_pc += imm_mode ? itRI : itR;
        if (!mem_.Push(reg_[PC])) goto _SERR;
        reg_pc = temp.func.position;
        NEXT(0);
    }
    _RET: {
        reg_pc = mem_.Pop().long_long;
        if (mem_.mem_error()) goto _SERR;
        NEXT(0);
    }
    _MOV: {
        reg_x.long_long = imm_mode ? inst->imm.int_val : reg_y.long_long;
        NEXT(imm_mode ? itRI : itRR);
    }
    _MOVL: {
        if (!imm_mode) goto _PERR;   // imm_mode ONLY!
        reg_x.doub = inst->imm.fp_val;
        NEXT(itRIF);
    }
    _POP: {
        reg_x = mem_.Pop();
        if (mem_.mem_error()) goto _SERR;
        NEXT(itR);
    }
    _PUSH: {
        temp.num.long_long = inst->imm.int_val;
        if (!mem_.Push(imm_mode ? temp.num : reg_x)) goto _SERR;
        NEXT(imm_mode ? itRI : itR);
    }
    _PEEK: {
        reg_x = mem_.Peek(reg_x.long_long);
        if (mem_.mem_error()) goto _MERR;
        NEXT(itR);
    }
    _LD: {
        temp.num.long_long = imm_mode ? inst->imm.int_val : reg_y.long_long;
        reg_x = mem_(temp.num.long_long);
        if (mem_.mem_error()) goto _MERR;
        NEXT(imm_mode ? itRI : itRR);
    }
    _ST: {
        // ST: I, R/I;   inst->imm -> I, reg_x/temp.num -> R/I
        if (imm_mode) {
            temp.num.long_long = *(unsigned int *)(cache_.data() + reg_pc + itRI);
            mem_(inst->imm.int_val) = temp.num;
            if (mem_.mem_error()) goto _MERR;
            NEXT(itII);
        }
        else {
            mem_(inst->imm.int_val) = reg_x;
            if (mem_.mem_error()) goto _MERR;
            NEXT(itRI);
        }
    }
    _STR: {
        temp.num.long_long = inst->imm.int_val;
        mem_(reg_x.long_long) = imm_mode ? temp.num : reg_y;
        if (mem_.mem_error()) goto _MERR;
        NEXT(imm_mode ? itRI : itRR);
    }
    _STC: {
        mem_[reg_x.long_long] = (char)(imm_mode ? inst->imm.int_val : reg_y.long_long);
        if (mem_.mem_error()) goto _MERR;
        NEXT(imm_mode ? itRI : itRR);
    }
    _INT: {
        auto opr = *(unsigned int *)(cache_.data() + reg_pc + itVOID);
        if (!int_manager_.TriggerInterrupt(opr, reg_, mem_)) goto _PERR;
        if (mem_.mem_error()) goto _MERR;
        NEXT(itI);
    }
    _NEWS: {
        temp.str = mem_.AddStringObj(reg_x.long_long);
        if (mem_.mem_error()) goto _MERR;
        reg_x = temp.num;
        NEXT(itR);
    }
    _NEWL: {
        temp.list = mem_.AddListObj(reg_x.long_long, reg_y.long_long);
        if (mem_.mem_error()) goto _MERR;
        reg_x = temp.num;
        NEXT(itRR);
    }
    _DELS: _DELL: {
        temp.num = reg_x;
        if (inst->op == DELS) {
            if (!mem_.DelStringObj(temp.str)) goto _MERR;
        }
        else {
            if (!mem_.DelListObj(temp.list)) goto _MERR;
        }
        NEXT(itR);
    }
    _SETR: {
        temp.num = reg_x;
        mem_.SetRootEnv(temp.list);
        NEXT(itR);
    }
    _ADR: _RMR: {
        temp.num = reg_x;
        ZValue opr = {reg_y};
        if (inst->op == ADR) {
            mem_.AddListRef(temp.list, opr.list);
        }
        else {
            mem_.DelListRef(temp.list, opr.list);
        }
        if (mem_.mem_error()) goto _MERR;
    }
    NEXT(itRR);
    _ITF: {
        reg_x.doub = (double)reg_x.long_long;
        NEXT(itR);
    }
    _FTI: {
        reg_x.long_long = (long long)reg_x.doub;
        NEXT(itR);
    }
    _ITS: _FTS: {
        std::string str;
        if (inst->op == ITS) {
            str = std::to_string(reg_y.long_long);
        }
        else {
            str = std::to_string(reg_y.doub);
        }
        temp.num = reg_x;
        if (!mem_.SetRawString(temp.str, str.c_str())) goto _MERR;
        reg_x = temp.num;
    }
    NEXT(itRR);
    _STI: _STF: {
        temp.num = reg_y;
        auto ptr = mem_.GetRawString(temp.str);
        if (mem_.mem_error()) goto _MERR;
        if (inst->op == STI) {
            reg_x.long_long = strtoll(ptr, nullptr, 10);
        }
        else {
            reg_x.doub = strtod(ptr, nullptr);
        }
        NEXT(itRR);
    }
    _ADDS: _EQS: {
        temp.num = reg_x;
        ZValue opr = {reg_y};
        if (inst->op == ADDS) {
            if (!mem_.StringCatenate(temp.str, opr.str)) goto _MERR;
            reg_x = temp.num;
        }
        else {
            reg_x.long_long = mem_.StringCompare(temp.str, opr.str);
            if (mem_.mem_error()) goto _MERR;
        }
    }
    NEXT(itRR);
    _CPS: {
        ZValue opr = {reg_y};
        temp.str = mem_.StringCopy(opr.str);
        if (mem_.mem_error()) goto _MERR;
        reg_x = temp.num;
    }
    NEXT(itRR);
    _LENS: {
        temp.num = reg_y;
        reg_x.long_long = mem_.StringLength(temp.str);
        NEXT(itRR);
    }
    _GETS: {
        temp.num = reg_x;
        if (!mem_.GetStringObj(temp.str, reg_y.long_long)) goto _MERR;
        NEXT(itRR);
    }
    _SETS: {
        temp.num = reg_x;
        if (!mem_.SetStringObj(temp.str, reg_y.long_long)) goto _MERR;
        reg_x = temp.num;
        NEXT(itRR);
    }
    _ADDL: {
        temp.num = reg_x;
        ZValue opr = {reg_y};
        if (!mem_.ListCatenate(temp.list, opr.list)) goto _MERR;
        reg_x = temp.num;
    }
    NEXT(itRR);
    _CPL: {
        ZValue opr = {reg_y};
        temp.list = mem_.ListCopy(opr.list);
        if (mem_.mem_error()) goto _MERR;
        reg_x = temp.num;
    }
    NEXT(itRR);
    _LENL: {
        temp.num = reg_y;
        reg_x.long_long = mem_.ListLength(temp.list);
        if (mem_.mem_error()) goto _MERR;
        NEXT(itRR);
    }
    _EQL: {
        temp.num = reg_x;
        ZValue opr = {reg_y};
        reg_x.long_long = mem_.ListCompare(temp.list, opr.list);
        if (mem_.mem_error()) goto _MERR;
    }
    NEXT(itRR);
    _GETL: {
        temp.num = reg_y;
        reg_x = mem_.GetListItem(temp.list, reg_x.long_long);
        if (mem_.mem_error()) goto _MERR;
        NEXT(itRR);
    }
    _SETL: {
        temp.num = reg_x;
        ZValue opr = {reg_y};
        if (!mem_.SetListItem(temp.list, opr.num.long_long, reg_[*(char *)(cache_.data() + reg_pc + itRR)])) goto _MERR;
    }
    NEXT(itRRR);

#undef reg_x
#undef reg_y
#undef NEXT
}

} // namespace zvm
