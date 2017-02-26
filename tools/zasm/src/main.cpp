#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>

#include "lexer.h"
#include "gen.h"

namespace {

void PrintError(const char *description) {
	std::fprintf(stderr, "\033[31m\033[1merror:\033[0m %s\n", description);
}

void PrintHelp() {
	std::fprintf(stderr, "usage: zasm <input file> [options]\n\n");
	std::fprintf(stderr, "options:\n");
	std::fprintf(stderr, "  --help\t\tDisplay this help information\n");
	std::fprintf(stderr, "  --version\t\tDisplay zasm version information\n");
	std::fprintf(stderr, "  -o <file>\t\tPlace the output into <file>\n\n");
	std::fprintf(stderr, "For bug reporting instructions, please see:\n");
	std::fprintf(stderr, "\033[1m<GitHub link>\033[0m\n");
}

void PrintVersion() {
	std::fprintf(stderr, "zasm (ZexVM Assmebler) version 0.0.1\n");
	std::fprintf(stderr, "Copyright (C) 2010-2017 MaxXSoft\n");
	std::fprintf(stderr, "This is a free software. For more information, please check:\n");
	std::fprintf(stderr, "\033[1m<GitHub link>\033[0m\n");
}

void GenerateBytecode(std::ifstream &in, std::ofstream &out, const char *file_name) {
	std::string file(file_name);
	Lexer lexer(in);
	Generator gen(lexer, out);

	auto error_num = gen.Generate() + lexer.error_num();
	if (error_num > 0) {
		std::fprintf(stderr, "generation failed, %d error(s) detected.\n", error_num);
		std::remove(file.c_str());
	}
}

std::string GetOutputFile(const char *input) {
	std::string temp = input;
	temp = temp.substr(0, temp.rfind(".") + 1) + "zbc";
	return temp;
}

}

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
