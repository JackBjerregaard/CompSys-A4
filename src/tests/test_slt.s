        .globl _start
_start:
        li      t0, 5
        li      t1, 10
        slt     t2, t0, t1
        li      a7, 93
        ecall
