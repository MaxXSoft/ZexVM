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
    MOV, POP, PUSH, LD, ST, STR, INT,   // Basic
    ITF, FTI, ITS, STI, FTS, STF,   // Convert
    ADDS, LENS, EQS,   // String
    ADDL, MOVL, CPL, LENL, EQL   // List
};

enum InstReg {
    IMM,   // marked as an immediate number
    R1, R2, R3, R4, R5, R6, R7,   // general registers
    A1, A2, A3, A4, A5, A6, RV,   // function registers
    // (A1-A5: args, A6: arg6 or arg stack pointer, R14: ret value)
    PC   // program counter
};

enum InstLen {
    itVOID = sizeof(unsigned char), 
    itR = sizeof(unsigned char) * 2, 
    itI = sizeof(unsigned char) + sizeof(unsigned int), 
    itRR = sizeof(unsigned char) * 2, 
    itRI = sizeof(unsigned char) * 2 + sizeof(unsigned int), 
    itRIF = sizeof(unsigned char) * 2 + sizeof(double), 
    itII = sizeof(unsigned char) * 2 + sizeof(unsigned int) * 2
    // itRRR = sizeof(unsigned char) * 3, 
    // itRRI = sizeof(unsigned char) * 2 + sizeof(unsigned int)
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

template<typename T>
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

}

namespace zvm {

void ZexVM::Initialize() {
    program_error_ = true;

    Register zero;
    zero.long_long = 0;
    reg_.fill(zero);

    cache_.fill(0);
    mem_.fill(0);
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
    if (version[0] > kCurrentVersion[0]) {
        return false;
    }
    else if(version[1] > kCurrentVersion[1]) {
        return false;
    }

    unsigned int mem_size = 0, arg_stack_size = 0, const_pool_size = 0, temp = 0;
    file.read((char *)&mem_size, sizeof(unsigned int));
    file.read((char *)&arg_stack_size, sizeof(unsigned int));
    file.read((char *)&const_pool_size, sizeof(unsigned int));
    file.read((char *)&temp, sizeof(unsigned int));
    const_pool_size = temp - const_pool_size;
    if (mem_size > kMemorySize || arg_stack_size + const_pool_size >= mem_size) return false;

    for (int i = 0; i < const_pool_size; ++i) {
        file >> mem_[arg_stack_size + i];
    }

    if ((unsigned int)len - temp >= kCacheSize) return false;
    for (int i = 0; !file.eof(); ++i) {
        file >> cache_[i];
    }

    program_error_ = false;

    return !program_error_;
}

int ZexVM::Run() {
    if (program_error_) return kProgramError;

    VMInst inst;
    ZValue temp;
    auto &reg_pc = reg_[PC].long_long;

    for (;;) {
        if (reg_pc >= kCacheSize) {
            program_error_ = true;
            return kCacheError;
        }

        inst = *(VMInst *)(cache_.data() + reg_pc);
        
        auto &reg_x = reg_[inst.reg >> 4];
        auto &reg_y = reg_[inst.reg & 0x0F];
        auto imm_mode = !(inst.reg & 0x0F);

        switch (inst.op) {
            case END: {
                return kFinished;
            }
            case AND: case XOR: case OR: case SHL: case SHR:
            case ADD: case SUB: case MUL: case DIV: case MOD:
            case LT: case GT: case LE: case GE: case EQ: case NEQ: {
                reg_x.long_long = CalcExpression(reg_x.long_long, (imm_mode ? (long long)inst.imm.int_val : reg_y.long_long), inst.op);
                reg_pc += imm_mode ? itRI : itRR;
                break;
            }
            case ADDF: case SUBF: case MULF: case DIVF: case POW: {
                reg_x.doub = CalcExpression(reg_x.doub, (imm_mode ? inst.imm.fp_val : reg_y.doub), inst.op);
                reg_pc += imm_mode ? itRIF : itRR;
                break;
            }
            case LTF: case GTF: case LEF: case GEF: {
                reg_x.long_long = (long long)CalcExpression(reg_x.doub, (imm_mode ? inst.imm.fp_val : reg_y.doub), inst.op);
                reg_pc += imm_mode ? itRIF : itRR;
                break;
            }
            case NOT: case NEG: {
                reg_x.long_long = CalcExpression(reg_x.long_long, 0LL, inst.op);
                reg_pc += itR;
                break;
            }
            case NEGF: {
                reg_x.doub = -reg_x.doub;
                reg_pc += itR;
                break;
            }
            case JMP: {
                reg_pc = imm_mode ? inst.imm.int_val : reg_x.long_long;
                break;
            }
            case JZ: {
                if (reg_x.long_long == 0) {
                    reg_pc = imm_mode ? inst.imm.int_val : reg_x.long_long;
                }
                else {
                    reg_pc += imm_mode ? itRI : itRR;
                }
                break;
            }
            case JNZ: {
                if (reg_x.long_long != 0) {
                    reg_pc = imm_mode ? inst.imm.int_val : reg_x.long_long;
                }
                else {
                    reg_pc += imm_mode ? itRI : itRR;
                }
                break;
            }
            case CALL: {
                temp.num.doub = imm_mode ? inst.imm.fp_val : reg_x.doub;
                if (temp.func.arg_count > 6) {
                    reg_[A6].long_long = temp.func.arg_stack_pointer;
                }
                reg_pc += imm_mode ? itRIF : itR;
                stack_.push(reg_[PC]);
                reg_pc = temp.func.position;
                break;
            }
            case RET: {
                if (stack_.size() == 0) {
                    program_error_ = true;
                    return kStackError;
                }
                reg_pc = stack_.top().long_long;
                stack_.pop();
                break;
            }
            case MOV: {
                reg_x.long_long = imm_mode ? inst.imm.int_val : reg_y.long_long;
                reg_pc += imm_mode ? itRI : itRR;
                break;
            }
            case POP: {
                if (stack_.size() == 0) {
                    program_error_ = true;
                    return kStackError;
                }
                reg_x = stack_.top();
                stack_.pop();
                reg_pc += itR;
                break;
            }
            case PUSH: {
                temp.num.long_long = inst.imm.int_val;
                stack_.push(imm_mode ? temp.num : reg_x);
                reg_pc += imm_mode ? itRI : itR;
                break;
            }
            case LD: {
                temp.num.long_long = imm_mode ? inst.imm.int_val : reg_y.long_long;
                if (temp.num.long_long >= kMemorySize - sizeof(Register)) {
                    program_error_ = true;
                    return kMemoryError;
                }
                reg_x = *(Register *)(mem_.data() + temp.num.long_long);
                reg_pc += imm_mode ? itRI : itRR;
                break;
            }
            case ST: {
                // ST: I, R/I;   inst.imm -> I, reg_x/temp.num -> R/I
                if (inst.imm.int_val >= kMemorySize - sizeof(Register)) {
                    program_error_ = true;
                    return kMemoryError;
                }
                if (imm_mode) {
                    temp.num.long_long = *(unsigned int *)(cache_.data() + reg_pc + itRI);
                    *(Register *)(mem_.data() + inst.imm.int_val) = temp.num;
                    reg_pc += itII;
                }
                else {
                    *(Register *)(mem_.data() + inst.imm.int_val) = reg_x;
                    reg_pc += itRI;
                }
                break;
            }
            case STR: {
                if (reg_x.long_long >= kMemorySize - sizeof(Register)) {
                    program_error_ = true;
                    return kMemoryError;
                }
                temp.num.long_long = inst.imm.int_val;
                *(Register *)(mem_.data() + reg_x.long_long) = imm_mode ? temp.num : reg_y;
                reg_pc += imm_mode ? itRI : itRR;
                break;
            }
            case INT: {
                auto opr = *(unsigned int *)(cache_.data() + reg_pc + itVOID);
                int_manager_.TriggerInterrupt(opr, reg_, mem_);
                reg_pc += itI;
                break;
            }
            case ITF: {
                reg_x.doub = (double)reg_x.long_long;
                reg_pc += itR;
                break;
            }
            case FTI: {
                reg_x.long_long = (long long)reg_x.doub;
                reg_pc += itR;
                break;
            }
            case ITS: case FTS: {
                // GC
                auto str = std::to_string(inst.op == ITS ? reg_y.long_long : reg_y.doub);
                temp.num = reg_x;
                if (temp.str.position + str.size() > kMemorySize - 1) {
                    program_error_ = true;
                    return kMemoryError;
                }
                strcpy((char *)(mem_.data() + temp.str.position), str.c_str());
                reg_pc += itRR;
                break;
            }
            case STI: case STF: {
                // GC
                temp.num = reg_y;
                if (temp.str.position > kMemorySize - 1) {
                    program_error_ = true;
                    return kMemoryError;
                }
                if (inst.op == STI) {
                    reg_x.long_long = strtoll(mem_.data() + temp.str.position, nullptr, 10);
                }
                else {
                    reg_x.doub = strtod(mem_.data() + temp.str.position, nullptr);
                }
                reg_pc += itRR;
                break;
            }
            case ADDS: case EQS: {
                // GC
                temp.num = reg_x;
                ZValue opr;
                opr.num = reg_y;
                if (temp.str.position > kMemorySize - 1 || opr.str.position > kMemorySize - 1) {
                    program_error_ = true;
                    return kMemoryError;
                }
                if (inst.op == ADDS) {
                    strcat((char *)(mem_.data() + temp.str.position), (char *)(mem_.data() + opr.str.position));
                }
                else {
                    reg_x.long_long = !strcmp((char *)(mem_.data() + temp.str.position), (char *)(mem_.data() + opr.str.position));
                }
                reg_pc += itRR;
                break;
            }
            case LENS: {
                // GC
                temp.num = reg_y;
                if (temp.str.position > kMemorySize - 1) {
                    program_error_ = true;
                    return kMemoryError;
                }
                reg_x.long_long = strlen((char *)(mem_.data() + temp.str.position));
                reg_pc += itRR;
                break;
            }
            case ADDL: {
                // GC
                temp.num = reg_x;
                //
                reg_pc += itRR;
                break;
            }
            case MOVL: {
                // GC?
                reg_x.doub = inst.imm.fp_val;
                reg_pc += itRIF;
                break;
            }
            case CPL: {
                // GC
                temp.num = reg_x;
                //
                reg_pc += itRR;
                break;
            }
            case LENL: {
                temp.num = reg_y;
                reg_x.long_long = temp.list.len;
                reg_pc += itRR;
                break;
            }
            case EQL: {
                // GC
                //
                reg_pc += itRR;
                break;
            }
            default: {
                program_error_ = true;
                return kProgramError;
            }
        }
    }
}

}
