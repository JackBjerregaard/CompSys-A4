        .globl _start
_start:
        li      t0, 0x10000
        li      t1, 0x123456FF
        sw      t1, 0(t0)
        lb      t2, 0(t0)
        li      a7, 93
        ecall
