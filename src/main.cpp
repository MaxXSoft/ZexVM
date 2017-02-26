#include <fstream>
#include <iostream>
#include <string>

#include <ctime>

#include "interrupt.h"
#include "zvm.h"

using namespace zvm;

void PrintMessage(const std::string &str, int msg_code = 0) {
	std::cout << std::endl << str;
	if (msg_code) std::cout << msg_code;
	std::cout << std::endl;
}

int main(int argc, const char *argv[]) {
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
