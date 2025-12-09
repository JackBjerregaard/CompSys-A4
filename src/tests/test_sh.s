        .globl _start
_start:
        li      t0, 0x10000
        li      t1, 0xABCD
        sh      t1, 0(t0)
        lw      t2, 0(t0)
        li      a7, 93
        ecall
