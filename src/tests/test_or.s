        .globl _start
_start:
        li      t0, 12
        li      t1, 5
        or      t2, t0, t1
        li      a7, 93
        ecall
