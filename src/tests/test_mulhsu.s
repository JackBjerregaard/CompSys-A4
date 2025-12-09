        .globl _start
_start:
        li      t0, -2
        li      t1, 2
        mulhsu  t2, t0, t1
        li      a7, 93
        ecall
