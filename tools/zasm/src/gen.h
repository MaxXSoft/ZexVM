#ifndef ZVM_TOOLS_ZASM_GEN_H_
#define ZVM_TOOLS_ZASM_GEN_H_

#include <fstream>

#include "lexer.h"

const unsigned char kZBCHead[3] = {0x93, 0x94, 0x86};
const unsigned char kZBCVersion[2] = {0, 2};

class Generator {
public:
    Generator(Lexer &lexer, std::ofstream &out) : lexer_(lexer), out_(out), error_num_(0) {}
    ~Generator() {}

    int Generate();

    unsigned int error_num() const { return error_num_; }

private:
    void PrintError(const char *description);
    void HandleLabelRef();
    bool HandleOperator();

    Lexer &lexer_;
    std::ofstream &out_;
    unsigned int error_num_;
};

#endif // ZVM_TOOLS_ZASM_GEN_H_
