#include "lexer.h"

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <algorithm>

namespace {

const char *op_str[] = {
    "END",
    "AND", "XOR", "OR", "NOT", "SHL", "SHR",
    "ADD", "ADDF", "SUB", "SUBF", "MUL", "MULF", "DIV", "DIVF", "NEG", "NEGF", "MOD", "POW",
    "LT", "LTF", "GT", "GTF", "LE", "LEF", "GE", "GEF", "EQ", "NEQ",
    "JMP", "JZ", "JNZ", "CALL", "RET",
    "MOV", "MOVL", "POP", "PUSH", "PEEK", "LD", "ST", "STR", "STC", "INT",
    "NEWS", "NEWL", "DELS", "DELL", "SETR", "ADR", "RMR",
    "ITF", "FTI", "ITS", "STI", "FTS", "STF",
    "ADDS", "CPS", "LENS", "EQS", "GETS", "SETS",
    "ADDL", "CPL", "LENL", "EQL", "GETL", "SETL",
    "DEF", "HEADER"
};

const char *reg_str[] = {
    "",
    "R1", "R2", "R3", "R4", "R5", "R6", "R7",
    "A1", "A2", "A3", "A4", "A5", "A6", "RV",
    "PC"
};

int IsOperator(const char *str) {
    auto len = sizeof(op_str) / sizeof(op_str[0]);
    for (int i = 0; i < len; ++i) {
        if(!strcmp(str, op_str[i])) return i;
    }
    return kError;
}

int IsRegister(const char *str) {
    auto len = sizeof(reg_str) / sizeof(reg_str[0]);
    for (int i = 0; i < len; ++i) {
        if(!strcmp(str, reg_str[i])) return i;
    }
    return kError;
}

int GetDLE(const std::string &str) {
    switch (str[0]) {
        case 'a': return '\a';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'v': return '\v';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0': return '\0';
        case 'x': {
            auto hex = str.substr(1);
            char *end_pos = nullptr;
            auto ret = (int)strtol(hex.c_str(), &end_pos, 16);
            return end_pos - hex.c_str() == hex.length() ? ret : kError;
        }
        default: return kError;
    }
}

} // namespace

int Lexer::PrintError(const char *description) {
    fprintf(stderr, "\033[1mlexer\033[0m(line %u): \033[31m\033[1merror:\033[0m %s\n", line_pos_, description);
    ++error_num_;
    return kError;
}

int Lexer::NextToken() {
    if (in_.eof()) return kEOF;
    static char last_char = ' ';

    auto IsEndOfLine = [&]() {
        return in_.eof() || last_char == '\n' || last_char == '\r';
    };

    while (!IsEndOfLine() && isspace(last_char)) in_ >> last_char;

    if (last_char == ';') {
        do {
            in_ >> last_char;
        } while (!IsEndOfLine());
    }

    if (isalpha(last_char) || last_char == '_') {
        std::string id;

        do {
            id += last_char;
            in_ >> last_char;
        } while (!in_.eof() && (isalnum(last_char) || last_char == '_'));
        transform(id.begin(), id.end(), id.begin(), toupper);

        int temp = 0;
        if ((temp = IsOperator(id.c_str()/*, line_pos_*/)) != kError) {
            op_val_ = temp;
            return kOperator;
        }
        else if ((temp = IsRegister(id.c_str()/*, line_pos_*/)) != kError) {
            reg_val_ = temp;
            return kRegister;
        }
        else if(last_char == ':') {
            lab_val_ = id;
            in_ >> last_char;
            return kLabelDef;
        }
        else {
            lab_val_ = id;
            return kLabelRef;
        }
    }

    if (isdigit(last_char) || last_char == '.' || last_char == '-') {
        std::string num_str;
        char *end_pos = nullptr;
        auto is_double = (last_char == '.');
        auto IsValidConv = [&end_pos, &num_str]() {
            return end_pos - num_str.c_str() == num_str.length();
        };

        if (last_char == '0') {
            in_ >> last_char;
            if (toupper(last_char) == 'X') {
                in_ >> last_char;
                while (isalnum(last_char)) {
                    num_str += last_char;
                    in_ >> last_char;
                }
                num_val_ = (unsigned int)strtoul(num_str.c_str(), &end_pos, 16);
                return IsValidConv() ? kNumber : PrintError("invalid hex");
            }
            else if (last_char == ' ' || last_char == ';' || last_char == ',' || IsEndOfLine()) {
                num_val_ = 0;
                return kNumber;
            }
            else if (last_char != '.') {
                return PrintError("invalid immediate number");
            }
        }
        do {
            if (!is_double && last_char == '.') is_double = true;
            num_str += last_char;
            in_ >> last_char;
        } while (isdigit(last_char) || last_char == '.' || last_char == 'e');

        if (is_double) {
            float_val_ = strtod(num_str.c_str(), &end_pos);
            return IsValidConv() ? kFloat : PrintError("invalid floating point number");
        }
        else {
            num_val_ = (unsigned int)strtoul(num_str.c_str(), &end_pos, 0);
            return IsValidConv() ? kNumber : PrintError("invalid immediate number");
        }
    }

    if (last_char == '\'') {
        std::string str;
        in_ >> last_char;
        while (last_char != '\'') {
            str += last_char;
            in_ >> last_char;
            if (IsEndOfLine()) return PrintError("expected \"\'\"");
        }
        in_ >> last_char;

        if (str.length() == 1) {
            char_val_ = (unsigned char)str[0];
            return kChar;
        }
        else if (str.length() == 0) {
            return PrintError("invalid character constant");
        }
        else if (str[0] == '\\') {
            auto ret = GetDLE(str.substr(1));
            if (ret != kError) {
                char_val_ = (unsigned char)ret;
                return kChar;
            }
            else {
                return PrintError("unknown escaped character");
            }
        }
        else {
            return PrintError("invalid character constant");
        }
    }

    if (last_char == '\"') {
        std::string str, temp;
        in_ >> last_char;

        while (last_char != '\"') {
            if (last_char == '\\') {
                in_ >> last_char;
                if (IsEndOfLine()) return PrintError("expected \'\"\'");
                temp += last_char;
                if (last_char == 'x') {
                    for (int i = 0; i < 2; ++i) {
                        in_ >> last_char;
                        if (IsEndOfLine()) return PrintError("expected \'\"\'");
                        temp += last_char;
                    }
                }
                auto ret = GetDLE(temp);
                if (ret != kError) {
                    last_char = ret;
                }
                else {
                    return PrintError("unknown escaped character");
                }
            }
            str += last_char;
            in_ >> last_char;
            if (IsEndOfLine()) return PrintError("expected \'\"\'");
        }
        in_ >> last_char;

        str_val_ = str;
        return kString;
    }

    if (IsEndOfLine()) {
        ++line_pos_;
        in_ >> last_char;
        return NextToken();
    }

    auto cur_char = last_char;
    in_ >> last_char;
    return cur_char;
}
