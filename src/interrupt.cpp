#include "interrupt.h"

#include <cstdio>

namespace {

zvm::ZValue null_value;

zvm::ZValue PutChar(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	std::fputc((int)(arg[0].long_long & 0xFF), stderr);
	return null_value;
}

zvm::ZValue GetChar(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	zvm::ZValue ret;
	ret.num.long_long = (long long)std::getchar();
	return ret;
}

zvm::ZValue PutInteger(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	std::fprintf(stderr, "%lld", arg[0].long_long);
	return null_value;
}

zvm::ZValue PutFloat(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	std::fprintf(stderr, "%lf", arg[0].doub);
	return null_value;
}

zvm::ZValue PutString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	zvm::ZValue temp;
	temp.num = arg[0];
	std::fprintf(stderr, "%s", mem.data() + temp.str.position);
	temp.num.long_long = 0;
	return temp;
}

zvm::ZValue GetInteger(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	zvm::ZValue ret;
	ret.num.long_long = 0;
	std::scanf("%lld", &ret.num.long_long);
	return ret;
}

zvm::ZValue GetFloat(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	zvm::ZValue ret;
	ret.num.doub = 0;
	std::scanf("%lf", &ret.num.doub);
	return ret;
}

zvm::ZValue GetString(const zvm::IntFuncArg &arg, zvm::IntFuncMem &mem) {
	std::scanf("%s", mem.data() + arg[0].long_long);
	return null_value;
}

}

namespace zvm {

InterruptManager::InterruptManager() {
	null_value.num.long_long = 0;
	RegisterInterrupt(PutChar);
	RegisterInterrupt(GetChar);
	RegisterInterrupt(PutInteger);
	RegisterInterrupt(PutFloat);
	RegisterInterrupt(PutString);
	RegisterInterrupt(GetInteger);
	RegisterInterrupt(GetFloat);
	RegisterInterrupt(GetString);
}

int InterruptManager::RegisterInterrupt(IntFunc func) {
	func_vector_.push_back(func);
	return (int)func_vector_.size() - 1;
}

void InterruptManager::RaiseInterrupt(unsigned int index, std::array<Register, kRegisterCount> &reg, std::array<char, kMemorySize> &mem) {
	if (index > func_vector_.size() - 1) return;

	IntFuncArg arg;
	for (int i = 0; i < kArgRegisterCount; ++i) {
		arg[i] = reg[i + kArgRegisterOffset];
	}
	ZValue ret = func_vector_[index](arg, mem);
	reg[kArgRegisterOffset + kArgRegisterCount] = ret.num;
}

}
