# The Simulator Assignment (A4, CompSys 2025)

Finn Schiermer Andersen, November-December 2025

version 0.1

This assignment is issued Monday 24 November and must be submitted by 16:00 on Sunday 14 December.

The assignment consists of two sub-projects,

1: implementation of a RISC-V simulator and
2: investigation of branch predictors.


## Part 1

You must implement a disassembler and simulator for RISC-V, the 32-bit base instruction set, and the extension for 32-bit multiplication/division. A few special instructions are excluded. In addition, the simulator must handle a few system calls; details are given below. The disassembler and simulator must be written in C as *one* program, where a command-line option chooses between disassembly and simulation.

To help you, an unfinished disassembler/simulator is provided. The supplied code already handles simulating memory and loading a program into that memory. To complete the simulator you must add the file `simulate.c` and implement the function `simulate()`, which is already declared in `simulate.h`. Likewise, you must add the file `disassemble.c` and implement the function `disassemble()`, which is declared in `disassemble.h`.

Several programs were previously distributed (find them in `resources/tiny_riscv` in the course's public repo). Four of them (`hello.c`, `echo.c`, `fib.c`, and `erat.c`) must be disassembled and executed on the finished simulator. These programs are written in C, and you can use the cross-compiler introduced in the 19/11 lecture (see the slides in `materials/25_11_19_tiny_riscv_setup/slides.pdf`) to translate them into an ELF format that the completed disassembler/simulator can read.

You must design and carry out a test that exercises every instruction.


## Part 2

Extend the simulator from Part 1 so it can count the number of mispredictions for four different predictors:

 * [NT] Every branch is always predicted Not Taken.
 * [BTFNT] Backward branches are always predicted taken, forward branches are always predicted not taken.
 * [Bimodal] Dynamic prediction. Find out for yourself what a bimodal predictor is.
 * [gShare] Dynamic prediction. Find out for yourself what a gShare predictor is.

For Bimodal and gShare you must collect data for four different sizes: 256, 1K, 4K, and 16K, where the size denotes the number of 2-bit state machines.

Some programs (and corresponding inputs) are supplied and must be simulated for every predictor. Report the number of predictions and the number of mispredictions.


## Submission

Your submission must contain:

* A short report briefly describing:
  * Your overall design/approach
  * How you decode the instructions (including how you generate the correct immediate when relevant)
  * How you tested and what your testing demonstrates
  * If you were unable to implement something, why you think that happened
* The implementation of the disassembly function
* The implementation of the simulation function
* Documentation of the testing. For every instruction you should state:
  * What you are testing
  * Exactly how you test it in the form of a minimal assembly program
* Documentation that the four mentioned C programs can be disassembled and executed by the simulator
* A section on predictors that briefly explains the four predictors
* A description of how you implemented predictor simulation
* A section with data plus an interpretation of the data. What do the data show about the predictors and the programs?

If the supplied programs cannot be disassembled or executed on the simulator, the submission must say so clearly. The testing should make a systematic attempt to determine which instructions work correctly and which do not at submission time.


## Supplied Files

The following files are supplied as the unfinished simulator:

~~~
main.c: - main program
memory.c, memory.h: - simulation of the computer's memory
read_elf.c, read_elf.h: - loading a RISC-V .elf file
simulate.h: - header declaring the function simulate() that must be implemented
disassemble.h: - header declaring the function disassemble() that must be implemented
~~~

You will find the files in the directory `assignments/A4/src` in the course's public repo.

To evaluate the predictors we supply a set of precompiled programs. (We hand out compiled binaries so we can be sure everyone simulates the same thing.) These programs must be simulated with their associated input sets. You can find the source files, ELF files (compiled programs), and input sets in `assignments/A4/predictor-benchmarks`.

~~~
erat.elf: - No input.
fib.elf: - Pass "32" as a command-line argument
radix.elf: - Unpack the file a.in.gz - pass "a.in a.out" as the command-line arguments
~~~

Note that the simulations contain many millions of instructions.


## Instructions

The authoritative reference (hereafter called the "riscv spec") for the instruction encodings is here:

~~~
https://github.com/riscv/riscv-isa-manual/releases/download/20240411/unpriv-isa-asciidoc.pdf

~~~

Pay special attention to the description of instruction encodings and immediate fields in Sections 2.2 and 2.3.

The simulator must handle the RISC-V 32-bit base instruction set (RV32I) and the extension for 32-bit multiplication (RV32M). Every instruction in the base instruction set appears on "the green sheet" included in COD. The sheet also lists the RV64M extension (64-bit multiplication), but you must implement the 32-bit extension. This extension is very similar but not identical. Find it described in the riscv spec.

Details about each instruction can be found in the riscv spec.

Do **not** implement the following instructions from the green sheet/RV32I:

~~~
ebreak, fence, fence.i, CSRRW, CSRRS, CSRRC, CSRRWI, CSRRSI, and CSRRCI.
~~~


## System Calls

System calls are triggered with the `ecall` instruction. The value in A7 specifies which system call is requested. Implement the following four system calls.

~~~
Call:           Effect:
1               return getchar() in A0
2               perform putchar(c), where c is taken from A0
3 and 93        terminate the simulation
~~~


## Simulation

The supplied (unfinished) disassembler/simulator takes an ELF file (extension ".elf") as its first argument. The second argument can enable logging. Any arguments for the simulated program can be given after `--` as illustrated below

~~~
./sim myprog.elf -d                # disassemble ELF file to standard output
./sim myprog.elf                   # simulation with no logging
./sim myprog.elf -l log            # simulation with per-instruction logging
./sim myprog.elf -s log            # simulation and then a summary log (instruction count, simulation time)
./sim fib.elf -l log -- 5          # simulate fib.elf with input 5, log execution to "log"
~~~

It is also possible to pass arguments to main() via argc/argv in the simulated C program. Place the arguments after `--`, for example:

~~~
./sim fib.elf -- 7                  # fib(7), no logging
./sim fib.elf -l log -- 7           # fib(7), log loading of the dis file and every instruction
./sim fib.elf -s log -- 7           # fib(7), log the final summary (instruction count, simulation time)
~~~

Arguments MUST be supplied in the order used above.

The supplied code expects the simulated program to declare the symbol `_start` and will begin simulation at the address of this symbol. Programs written as described below will automatically satisfy this requirement.


## Output Format

At a minimum, you must print the following for every instruction:

During disassembly and during logging in simulation:

 - The instruction's address.
 - The instruction encoded as a hexadecimal number
 - The disassembly of the instruction (symbolic assembly)
 - For branches and calls, the embedded constant shown as an absolute address.

During logging in simulation you must also print for every instruction:

 - The instruction number since the start of simulation
 - For conditional branches: whether the branch was taken
 - For instructions that write to a register: the register number and the register's new value
 - For instructions that write to memory: the memory address and the new value

For inspiration, here is an example of a possible logging format from the simulation:

~~~
   144 =>  10098 : 00044503     lbu      a0,0(s0)                R[10] <- 6e
   145     1009c : fe051ae3     bnez     a0,10090            {T}
   146 =>  10090 : 00140413     addi     s0,s0,1                 R[ 8] <- 119a2
   147     10094 : 4d8000ef     jal      ra,1056c                R[ 1] <- 10098
   148 =>  1056c : 00200893     li       a7,2                    R[17] <- 2
   149     10570 : 00a00833     add      a6,zero,a0              R[16] <- 6e
~~~

The symbol "=>" indicates that execution jumped to this instruction.
The symbol "{T}" indicates a taken conditional branch.
The rest should be self-explanatory.


## From "C" to ".elf" Files

We use a cross-compiler to produce ".elf" files from C or from handwritten RISC-V assembly.

Installation and use of the cross-compiler are described in the slides from the 20/11 lecture and the related exercises.

### From RISC-V Assembly to an .elf File

Use the cross-compiler. Even though it primarily targets C, it will happily accept ".s" files as input. You can write such `.s` files in any text editor.

For testing it can be worth creating small RISC-V assembly files, each of which tests a single instruction or only a few instructions. In this scenario the minimal library (`lib.c`) should not be part of the compilation. The Makefile can handle this.

The simulator expects the symbol `_start` to be declared and will start executing at the first instruction after `_start`.

Here is a very small assembly program illustrating this:

~~~
        .globl _start
_start:
        ori     a7,x0,3
        ecall
~~~

The only thing this little program does is stop the simulation (see the earlier section on system calls).


### AI Declaration

Remember to include your AI declaration in your submission.
