        .globl _start
_start:
        li      t0, 0x80000000
        li      t1, 2
        mulhu   t2, t0, t1
        li      a7, 93
        ecall
