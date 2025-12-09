        .globl _start
_start:
        li      t0, 0x10000
        li      t1, 0x1234FFFF
        sw      t1, 0(t0)
        lh      t2, 0(t0)
        li      a7, 93
        ecall
