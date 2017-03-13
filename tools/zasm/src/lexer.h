#ifndef ZVM_TOOLS_ZASM_LEXER_H_
#define ZVM_TOOLS_ZASM_LEXER_H_

#include <string>
#include <fstream>

enum InstOp {
    END,   // VM
    AND, XOR, OR, NOT, SHL, SHR,   // Bit
    ADD, ADDF, SUB, SUBF, MUL, MULF, DIV, DIVF, NEG, NEGF, MOD, POW,   // Math
    LT, LTF, GT, GTF, LE, LEF, GE, GEF, EQ, NEQ,   // Logic
    JMP, JZ, JNZ, CALL, RET,   // Jump
    MOV, POP, PUSH, LD, ST, STR, INT,   // Basic
    ITF, FTI, ITS, STI, FTS, STF,   // Convert
    ADDS, LENS, EQS,   // String
    ADDL, MOVL, CPL, LENL, EQL,   // List
    DEF, HEADER   // Pseudo instruction
};

enum TokenType {
    kError = -1,
    kEOF = -2,
    kNumber = -3,
    kFloat = -4,
    kChar = -5,
    kString = -6,
    kRegister = -7,
    kOperator = -8,
    kLabelDef = -9,
    kLabelRef = -10
};

class Lexer {
public:
    Lexer(std::ifstream &in) : in_(in), line_pos_(1), error_num_(0) {
        if (!in) {
            PrintError("invalid file");
        }
        else {
            in >> std::noskipws;
        }
    }
    ~Lexer() {}

    int NextToken();

    unsigned int line_pos() const { return line_pos_; }
    unsigned int error_num() const { return error_num_; }
    unsigned int num_val() const { return num_val_; }
    double float_val() const { return float_val_; }
    unsigned char char_val() const { return char_val_; }
    const std::string &str_val() const { return str_val_; }
    char reg_val() const { return reg_val_; }
    unsigned char op_val() const { return op_val_; }
    const std::string &lab_val() const { return lab_val_; }

private:
    int PrintError(const char *description);

    std::ifstream &in_;
    unsigned int line_pos_;
    unsigned int error_num_;

    unsigned int num_val_;
    double float_val_;
    unsigned char char_val_;
    std::string str_val_;
    char reg_val_;
    unsigned char op_val_;
    std::string lab_val_;
};

#endif // ZVM_TOOLS_ZASM_LEXER_H_
