#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>

#include "lexer.h"
#include "gen.h"

namespace {

void PrintError(const char *description) {
    fprintf(stderr, "\033[31m\033[1merror:\033[0m %s\n", description);
}

void PrintHelp() {
    fprintf(stderr, "usage: zasm <input file> [options]\n\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  --help\t\tDisplay this help information\n");
    fprintf(stderr, "  --version\t\tDisplay zasm version information\n");
    fprintf(stderr, "  -o <file>\t\tPlace the output into <file>\n\n");
    fprintf(stderr, "For bug reporting instructions, please see:\n");
    fprintf(stderr, "\033[1mhttps://github.com/MaxXSoft/ZexVM/issues\033[0m\n");
}

void PrintVersion() {
    fprintf(stderr, "zasm (ZexVM Assmebler) version %03d.%03d\n", kZBCVersion[0], kZBCVersion[1]);
    fprintf(stderr, "Copyright (C) 2010-2017 MaxXSoft\n");
    fprintf(stderr, "This is a free software. For more information, please check:\n");
    fprintf(stderr, "\033[1mhttps://github.com/MaxXSoft/ZexVM\033[0m\n");
}

void GenerateBytecode(std::ifstream &in, std::ofstream &out, const char *file_name) {
    std::string file(file_name);
    Lexer lexer(in);
    Generator gen(lexer, out);

    auto error_num = gen.Generate() + lexer.error_num();
    if (error_num > 0) {
        fprintf(stderr, "failed to generate: %s, %d error(s) detected.\n", file_name, error_num);
        remove(file.c_str());
    }
}

std::string GetOutputFile(const char *input) {
    std::string temp = input;
    temp = temp.substr(0, temp.rfind(".") + 1) + "zbc";
    return temp;
}

} // namespace

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        PrintError("no input file");
        return 0;
    }

    if (!strcmp(argv[1], "--help")) {
        PrintHelp();
        return 0;
    }
    else if (!strcmp(argv[1], "--version")) {
        PrintVersion();
        return 0;
    }

    std::ifstream in(argv[1]);
    std::ofstream out;
    if (!in) {
        PrintError("invalid file");
        return 0;
    }

    if (argc == 4) {
        if (!strcmp(argv[2], "-o")) {
            out.open(argv[3], std::ofstream::binary);
            GenerateBytecode(in, out, argv[3]);
            return 0;
        }
        else {
            PrintError("unknown command");
            return 0;
        }
    }

    auto out_file = GetOutputFile(argv[1]).c_str();
    out.open(out_file, std::ofstream::binary);
    GenerateBytecode(in, out, out_file);

    return 0;
}
