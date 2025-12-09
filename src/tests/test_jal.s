        .globl _start
_start:
        jal     ra, target
        li      a7, 93
        ecall

target:
        li      t0, 42
        ret
