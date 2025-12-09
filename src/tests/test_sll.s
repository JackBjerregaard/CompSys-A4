        .globl _start
_start:
        li      t0, 8
        li      t1, 2
        sll     t2, t0, t1
        li      a7, 93
        ecall
