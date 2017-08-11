#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <vector>
#include <utility>

#include "type.h"
#include "interrupt.h"
#include "zvm.h"
#include "xstl/argh.h"

using namespace zvm;

namespace {

void PrintMessage(const std::string &str, int msg_code = 0) {
    std::cout << std::endl << str;
    if (msg_code) std::cout << msg_code;
    std::cout << std::endl;
}

void PrintHelp() {
    std::cout << "usage: zvm [options] <inputs>" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  -g --gc-pool <value>\t\tSet the pool size of garbage collector" << std::endl;
    std::cout << "  -a --args <value>\t\tSpecify startup arguments of a ZexVM program" << std::endl;
    std::cout << std::endl;
    std::cout << "  -h --help\t\t\tDisplay this help information" << std::endl;
    std::cout << "  -v --version\t\t\tDisplay zasm version information" << std::endl;
    std::cout << std::endl;
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

void GetArgList(std::vector<std::string> &arg_list, const std::string &v) {
    std::string temp;
    bool in_quote = false;
    for (const auto &i : v) {
        if (i == '"') {
            in_quote = !in_quote;
        }
        else if (in_quote) {
            temp.push_back(i);
        }
        else if (i == ' ') {
            if (temp.empty()) continue;
            arg_list.push_back(std::move(temp));
            temp.clear();
        }
        else {
            temp.push_back(i);
        }
    }
    if (!temp.empty()) arg_list.push_back(std::move(temp));
}

} // namespace

int main(int argc, const char *argv[]) {
    xstl::ArgumentHandler argh;
    std::ifstream in;
    std::vector<std::string> arg_list;
    MemSizeT gc_pool_size = kGCPoolSize;

    auto PrintError = [](xstl::StrRef v) {
        std::cout << "invalid command ";
        if (v != "") std::cout << "'" << v << "'";
        std::cout << std::endl << "input \"zvm --help\" for help" << std::endl;
        return 1;
    };

    if (argc < 2) PrintError("");

    argh.SetErrorHandler(PrintError);
    argh.AddHandler("h", [](xstl::StrRef v) { PrintHelp(); return 1; });
    argh.AddAlias("help", "h");
    argh.AddHandler("v", [](xstl::StrRef v) { PrintVersion(); return 1; });
    argh.AddAlias("version", "v");
    argh.AddHandler("g", [&gc_pool_size](xstl::StrRef v) {
        try {
            gc_pool_size = std::stoi(v);
        }
        catch (...) {
            std::cout << "invalid pool size" << std::endl;
            return 1;
        }
        return 0;
    });
    argh.AddAlias("gc-pool", "g");
    argh.AddHandler("a", [&arg_list](xstl::StrRef v) {
        GetArgList(arg_list, v);
        return 0;
    });
    argh.AddAlias("args", "a");
    argh.AddHandler("", [&in](xstl::StrRef v) {
        in.open(v, std::ios_base::binary);
        return 0;
    });

    if (!argh.ParseArguments(argc, argv)) return 0;

    InterruptManager int_manager;
    ZexVM vm(gc_pool_size, int_manager);

    if (vm.LoadProgram(in)) {
        if (!arg_list.empty()) vm.SetStartupArguments(arg_list);
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
