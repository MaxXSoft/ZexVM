#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

#include <ctime>

#include "type.h"
#include "interrupt.h"
#include "zvm.h"

using namespace zvm;

namespace {

void PrintMessage(const std::string &str, int msg_code = 0) {
    std::cout << std::endl << str;
    if (msg_code) std::cout << msg_code;
    std::cout << std::endl;
}

void PrintHelp() {
    std::cout << "usage: zvm <bytecode file | options>" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  --help\t\tDisplay this help information" << std::endl;
    std::cout << "  --version\t\tDisplay zasm version information" << std::endl;
    std::cout << "For bug reporting instructions, please see:" << std::endl;
    std::cout << "\033[1mhttps://github.com/MaxXSoft/ZexVM/issues\033[0m" << std::endl;
}

void PrintVersion() {
    std::cout << "Zexium Virtual Machine (aka. ZexVM or ZVM) version ";
    std::cout << std::setfill('0') << std::setw(3);
    std::cout << (int)(kCurrentVersion[0]) << ".";
    std::cout << std::setw(3) << (int)(kCurrentVersion[1]) << std::endl;
    std::cout << "Copyright (C) 2010-2017 MaxXSoft" << std::endl;
    std::cout << "This is a free software. For more information, please check:" << std::endl;
    std::cout << "\033[1mhttps://github.com/MaxXSoft/ZexVM\033[0m" << std::endl;
}

}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        PrintMessage("invalid command\ninput \"zvm --help\" for help");
        return 0;
    }
    if (!strcmp(argv[1], "--help")) {
        PrintHelp();
        return 0;
    }
    if (!strcmp(argv[1], "--version")) {
        PrintVersion();
        return 0;
    }

    InterruptManager int_manager;
    ZexVM vm(int_manager);
    std::ifstream in;
    in.open(argv[1], std::ios_base::in | std::ios_base::binary);

    if (vm.LoadProgram(in)) {
        auto ret_val = vm.Run();
        if(ret_val == kFinished) {
            PrintMessage("success!");
        }
        else {
            PrintMessage("runtime error. return: ", ret_val);
        }
    }
    else {
        PrintMessage("program error");
    }

    return 0;
}
