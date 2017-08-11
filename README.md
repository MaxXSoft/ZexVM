# ZexVM

**Zexium Virtual Machine** (aka. **ZexVM** or **ZVM**) is a register-based high-level language virtual machine (HLLVM). 

Same as JVM, ZexVM is designed to solve the problems about cross-platform. It can run a certain kind of bytecode file, which is platform-independent. 

ZexVM has its own instruction set and assembly language (similar to x86), also has an assembler (**ZASM**). The assembler can convert ZexVM assembly to bytecode file. In the future, there will be a programming language called **Arsha** can be compiled to ZexVM bytecode. 

---

## Features

- Tracing garbage collection while runtime
- `String` and `List` type data structure (controled by garbage collector)
- `Function` type for a better support of anonymous functions and closures
- 64-bit integer and floating point number
- Call an external function by using `INT` instruction

There is no appealing feature in the current version (000.006), but we will add a lot of new features in the future, such as: 

- Just-In-Time (JIT) compilation or other runtime optimizations
- Class support and OOP
- More advanced functional programming support

## Build

This project contains a `makefile`. Please use `make` command to build the vitrual machine **ZexVM** and the assembler **ZASM**: 

```
make all
```

Or build one of them: 

```
make zvm
make zasm
```

Before you build the project, you should make sure that your compiler supported the C++ 14 standard. 

## Usage

To generate a ZexVM bytecode file from the ZexVM assembly, please run: 

```
./zasm <assembly file> [ -o <output file> ]
```

If there are no errors in the assembly file, **ZASM** will generate a `.zbc` file, and then you can run this file by using: 

```
./zvm <zbc file>
```

For help information, please run command `-h` or `--help`. 

## Instruction Set

Please read [zvm-is.md](zvm-is.md). 

## Test Cases

All of the test cases are in `test/` directory. 

- **[hello.zasm](test/hello.zasm):** print "Hello world." in command line
- **[hanoi.zasm](test/hanoi.zasm):** recursively solve the Hanoi Tower problem
- **[heart.zasm](test/heart.zasm):** print a heart pattern
- **[conv.zasm](test/conv.zasm):** test about type conversion
- **[funcs.zasm](test/funcs.zasm)** functional test about interrupts and GC

## Copyright and License

Copyright (C) 2010-2017 MaxXSoft. License GPL-3.0. 