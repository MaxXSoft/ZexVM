#ifndef ZVM_ZVM_H_
#define ZVM_ZVM_H_

#include <fstream>
#include <cstddef>
#include <stack>
#include <array>

#include "type.h"
#include "interrupt.h"

namespace zvm {

class ZexVM {
public:
	ZexVM(InterruptManager &int_manager) : int_manager_(int_manager) { Initialize(); }
	~ZexVM() {}

	bool LoadProgram(std::ifstream &file);
	int Run();

	bool program_error() const { return program_error_; }

private:
	void Initialize();

	bool program_error_;
	std::array<Register, kRegisterCount> reg_;
	std::array<char, kCacheSize> cache_;
	std::array<char, kMemorySize> mem_;   // argument stack, constant pool, other data
	std::stack<Register> stack_;
	InterruptManager &int_manager_;
};

}

#endif // ZVM_ZVM_H_
