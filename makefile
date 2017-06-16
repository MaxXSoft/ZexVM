export cc = g++
export opt_level = 3
export build_dir = build/
export debug = false

zvm_dir = src/
zvm_targets = $(zvm_dir)main.cpp $(zvm_dir)interrupt.cpp $(zvm_dir)zvm.cpp
zvm_out = $(build_dir)zvm

zasm_dir = tools/zasm/src/
zasm_targets = $(zasm_dir)main.cpp $(zasm_dir)lexer.cpp $(zasm_dir)gen.cpp
zasm_out = $(build_dir)zasm

ifeq ($(debug), true)
	debug_arg = -g
	opt_arg = 
else
	debug_arg = 
	opt_arg = -O$(opt_level)
endif

.PHONY: all zvm zasm clean

all: zvm zasm

zvm: $(zvm_targets)
	$(cc) $(debug_arg) $(zvm_targets) -o $(zvm_out) -std=c++14 $(opt_arg)

zasm: $(zasm_targets)
	$(cc) $(debug_arg) $(zasm_targets) -o $(zasm_out) -std=c++14 $(opt_args)

clean: clean_dbg
	rm $(zvm_out) $(zasm_out)

clean_dbg:
	rm -r $(build_dir)*.dSYM
