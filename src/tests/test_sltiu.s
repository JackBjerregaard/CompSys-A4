        .globl _start
_start:
        li      t0, 5
        sltiu   t1, t0, 10
        li      a7, 93
        ecall
