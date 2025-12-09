        .globl _start
_start:
        li      t0, 32
        srli    t1, t0, 2
        li      a7, 93
        ecall
