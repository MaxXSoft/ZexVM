export cc = g++
export opt_level = 3

build_dir = build/

zvm_dir = src/
zvm_targets = $(zvm_dir)main.cpp $(zvm_dir)interrupt.cpp $(zvm_dir)zvm.cpp
zvm_out = $(build_dir)zvm

zasm_dir = tools/zasm/src/
zasm_targets = $(zasm_dir)main.cpp $(zasm_dir)lexer.cpp $(zasm_dir)gen.cpp
zasm_out = $(build_dir)zasm

.PHONY: all zvm zasm clean

all: zvm zasm

zvm: $(zvm_targets)
    $(cc) $(zvm_targets) -o $(zvm_out) -std=c++14 -O$(opt_level)

zasm: $(zasm_targets)
    $(cc) $(zasm_targets) -o $(zasm_out) -std=c++14 -O$(opt_level)

clean: 
    rm $(zvm_out) $(zasm_out)
