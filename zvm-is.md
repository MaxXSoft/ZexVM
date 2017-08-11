# ZexVM Instruction Set

ZexVM is a register-based virtual machine, it has its own instruction set and assembly language (similar to x86). 

---

## Virtual Hardwares

ZexVM provides 15 registers, a memory (including garbage collector) and a stack. 

### Registers

There are 15 registers can be access in ZexVM. Each register can store a 64-bit integer or a double-precision floating-point number. 

- *R0 (inaccessible):* used to mark whether the instruction uses an immediate number as a parameter
- **R1 - R7:** general registers
- **A1 - A6:** argument registers, also can be used as general registers
	- When a function is called, these registers used to store the 1st to 6th argument
	- If there are more than 6 arguments, program should push the other arguments into stack, and use PEEK instruction to read them from stack
- **RV:** return value register， also can be used as a general register
	- This register stores the environment pointer before a function call
	- This register stores the return value after a function call
- **PC:** program counter. If you try to modify the value of this register at will, the program may be crash

### Memory

ZexVM (version 000.006) provides a memory manager, and any kind of memory operation *(including operations about stack and read or write the objects in garbage collector)* will be taken over this manager.

The program can specify how much memory it need to take before running, and the virtual machine will dynamically scales the size of memory based on this value. 

Memory is divided into the following sections: 

- **Constant pool:** store all of the constants used in the program
- **Program-managed memory:** the rest of the memory

### Stack

You can use `PUSH`, `POP` and `PEEK` instruction to access the stack. 

Stack can store the value of PC register when a function is called, in order to return from the function call. 

### Garbage collector

ZexVM has a built-in **tracing garbage collector** since version 000.006. 

All the `String` and `List` type data will be put in GC pool, and garbage collector will store their position as items in a list called *OBJ_SET*. If `DELS` or `DELL` instructions are called, the corresponding list item will be deleted but the space they occupied won't be free immediately. 

Considering that the problem of reference cycles, ZexVM uses a kind of tracing garbage collecting algorithm. When GC pool is full of data, garbage collector will trace from the root object (which can be set via `SETR`, `ADR` and `RMR` instructions), mark all the reachable object, then remove those objects which is unreachable. 

After doing that, collector will start to reallocate the pool space according to the remaining items in *OBJ_SET*, copy them to a new memory and defragment the rest of space.

## Instruction Format

There are 7 types of instructions in ZexVM. 

- Void

	| Inst. ID (1 byte) |
	|---|

- Single register

	| Inst. ID (1 byte) | Reg. ID (4 bits) | Reserve (4 bits) |
	|---|---|---|

- Single immediate number

	| Inst. ID (1 byte) | Reserve (1 byte) | Imm. Number (4 bytes) |
	|---|---|---|

- Double register

	| Inst. ID (1 byte) | Reg1 (4 bits) | Reg2 (4 bits) |
	|---|---|---|

- Triple register

	| Inst. ID (1 byte) | Reg1 (4 bits) | Reg2 (4 bits) | Reg3 (4 bits) |
	|---|---|---|---|

- Register and immediate number

	| Inst. ID (1 byte) | Reg. ID (4 bits) | Zero (4 bits) | Imm. Number (4 bytes) |
	|---|---|---|---|

- Register and 64-bit immediate number

	| Inst. ID (1 byte) | Reg. ID (4 bits) | Zero (4 bits) | Imm. Number (8 bytes) |
	|---|---|---|---|

- Double immediate number

	| Inst. ID (1 byte) | Zero (1 byte) | Imm1 (4 bytes) | Imm2 (4 bytes) |
	|---|---|---|---|

## Supported Instructions

*When there is `[F]` behind an `Inst. ID`, it means that the instruction has its floating-point form.* 

*e.g. `ADD R1, 123` and `ADDF R1, 123.45`*

| Inst. ID | Syntax | Description |
|---|---|---|
| END | `END` | Mark the end of a program. |
| AND | `AND Reg1, <Reg2/Imm>` | Reg1 &= Reg2 or Imm |
| XOR | `XOR Reg1, <Reg2/Imm>` | Reg1 ^= Reg2 or Imm |
| OR | `OR Reg1, <Reg2/Imm>` | Reg1 \|= Reg2 or Imm |
| NOT | `NOT Reg` | Reg = ~Reg |
| SHL | `SHL Reg1, <Reg2/Imm>` | Reg1 <<= Reg2 or Imm |
| SHR | `SHR Reg1, <Reg2/Imm>` | Reg1 >>= Reg2 or Imm |
| ADD[F] | `ADD[F] Reg1, <Reg2/Imm>` | Reg1 += Reg2 or Imm |
| SUB[F] | `SUB[F] Reg1, <Reg2/Imm>` | Reg1 -= Reg2 or Imm |
| MUL[F] | `MUL[F] Reg1, <Reg2/Imm>` | Reg1 *= Reg2 or Imm |
| DIV[F] | `DIV[F] Reg1, <Reg2/Imm>` | Reg1 /= Reg2 or Imm |
| NEG[F] | `NEG[F] Reg1` | Reg1 = -Reg1 |
| MOD | `MOD Reg1, <Reg2/Imm>` | Reg1 %= Reg2 or Imm |
| POW | `POW Reg1, <Reg2/Imm>` | Reg1 **= Reg2 or Imm |
| LT[F] | `LT[F] Reg1, <Reg2/Imm>` | Reg1 = Reg1 < Reg2 or Imm |
| GT[F] | `GT[F] Reg1, <Reg2/Imm>` | Reg1 = Reg1 > Reg2 or Imm |
| LE[F] | `LE[F] Reg1, <Reg2/Imm>` | Reg1 = Reg1 <= Reg2 or Imm |
| GE[F] | `GE[F] Reg1, <Reg2/Imm>` | Reg1 = Reg1 >= Reg2 or Imm |
| EQ | `EQ Reg1, <Reg2/Imm>` | Reg1 = Reg1 == Reg2 or Imm |
| NEQ | `NEQ Reg1, <Reg2/Imm>` | Reg1 = Reg1 != Reg2 or Imm |
| JMP | `JMP <Reg1/Imm>` | PC = Reg1 or Imm |
| JZ | `JZ Reg1, <Reg2/Imm>` | If Reg1 == 0 PC = Reg2 or Imm |
| JNZ | `JNZ Reg1, <Reg2/Imm>` | If Reg1 != 0 PC = Reg2 or Imm |
| CALL | `CALL EnvPointer, Addr` | Call function at Addr |
| CALL | `CALL Reg1` | Call the function according to the information in the Reg1 |
| RET | `RET` | Return from a function call |
| MOV | `MOV Reg1, <Reg2/Imm>` | Reg1 = Reg2 or Imm (32-bit) |
| MOVL | `MOVL Reg1, Imm` | Reg1 = Imm (64-bit) |
| POP | `POP Reg1` | Pop the top of the stack to Reg1 |
| PUSH | `PUSH <Reg1/Imm>` | Push Reg1 or Imm to the stack |
| PEEK | `PEEK Reg1` | Peek the value of stack at offset Reg1 to Reg1 |
| LD | `LD Reg1, <Reg2/Imm>` | Reg1 = Mem[Reg2 or Imm] |
| ST | `ST Imm1, <Reg2/Imm2>` | Mem[Imm1] = Reg2 or Imm2 |
| STR | `STR Reg1, <Reg2/Imm>` | Mem[Reg1] = Reg2 or Imm |
| STC | `STC Reg1, <Reg2/Imm>` | Mem[Reg1] = (Char)(Reg2 or Imm) |
| INT | `INT Imm` | Triggering an external interrupt |
| NEWS | `NEWS Reg1` | Reg1.String = new String(Mem[Reg1]) |
| NEWL | `NEWL Reg1, <Reg2/Imm>` | Reg1.List = new List(position = Reg1, Length = Reg2 or Imm) |
| DELS | `DELS Reg1` | delete Reg1.String |
| DELL | `DELL Reg1` | delete Reg1.List |
| SETR | `SETR Reg1` | GC.SetTracingRoot(Reg1.List) |
| ADR | `ADR Reg1, Reg2` | Add Reg2.List as sub-object of Reg1.List in order to tracing by GC |
| RMR | `RMR Reg1, Reg2` | Remove Reg2.List from list of sub-objects in Reg1.List |
| ITF | `ITF Reg1` | Reg1.Double = (Double)Reg1.Int |
| FTI | `FTI Reg1` | Reg1.Int = (Int)Reg1.Double |
| ITS | `ITS Reg1, Reg2` | Reg1.String = (String)Reg2.Int |
| STI | `STI Reg1, Reg2` | Reg1.Int = (Int)Reg2.String |
| FTS | `FTS Reg1, Reg2` | Reg1.String = (String)Reg2.Double |
| STF | `STF Reg1, Reg2` | Reg1.Double = (Double)Reg2.String |
| ADDS | `ADDS Reg1, Reg2` | Reg1.String += Reg2.String |
| CPS | `CPS Reg1, Reg2` | Reg1.String = new String(Reg2.String) |
| LENS | `LENS Reg1, Reg2` | Reg1 = Reg2.String.Length |
| EQS | `EQS Reg1, Reg2` | Reg1 = Reg1.String == Reg2.String |
| GETS | `GETS Reg1, Reg2` | Mem[Reg2] = Reg1.String |
| SETS | `SETS Reg1, Reg2` | Reg1.String = Mem[Reg2] |
| ADDL | `ADDL Reg1, Reg2` | Reg1.List += Reg2.List |
| CPL | `CPL Reg1, Reg2` | Reg1.List = Reg2.List |
| LENL | `LENL Reg1, Reg2` | Reg1 = Reg2.List.Length |
| EQL | `EQL Reg1, Reg2` | Reg1 = Reg1.List == Reg2.List |
| GETL | `GETL Reg1, Reg2` | Reg1 = Reg2.List[Reg1] |
| SETL | `SETL Reg1, Reg2, Reg3` | Reg1.List[Reg2] = Reg3 |
