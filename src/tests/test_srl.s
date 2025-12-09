        .globl _start
_start:
        li      t0, 32
        li      t1, 2
        srl     t2, t0, t1
        li      a7, 93
        ecall
