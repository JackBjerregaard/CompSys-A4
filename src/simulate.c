#include <stdint.h>
#include <stdio.h>

#include "simulate.h"
#include "memory.h"

void handle_type_U(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
}

void handle_type_J(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
}

void handle_type_B(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
}

void handle_type_I(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
}

void handle_type_S(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
}

void handle_type_R(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
}

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
  // Mask to get last 7 bites
  // 0x7F = 0b01111111 = 127 decimal (extracts bits [6:0])
  uint32_t opcode = memory_rd_b(mem, start_addr) & 0x7F;

  // Determine type of instruction by using opcodes from instruction set
  switch (opcode) {
    case 0x37:  // 55 decimal - LUI (Load Upper Immediate)
    case 0x17:  // 23 decimal - AUIPC (Add Upper Immediate to PC)
      handle_type_U(mem, start_addr, log_file, symbols);
    case 0x6F:  // 111 decimal - JAL (Jump and Link)
      handle_type_J(mem, start_addr, log_file, symbols);
    case 0x63:  // 99 decimal - Branches (BEQ, BNE, BLT, BGE, BLTU, BGEU)
      handle_type_B(mem, start_addr, log_file, symbols);
    case 0x67:  // 103 decimal - JALR (Jump and Link Register)
    case 0x03:  // 3 decimal - Loads (LB, LH, LW, LBU, LHU)
    case 0x13:  // 19 decimal - I-type arithmetic (ADDI, SLTI, XORI, ORI, ANDI, SLLI, SRLI, SRAI)
    case 0x73:  // 115 decimal - System (ECALL, EBREAK)
      handle_type_I(mem, start_addr, log_file, symbols);
    case 0x23:  // 35 decimal - Stores (SB, SH, SW)
      handle_type_S(mem, start_addr, log_file, symbols);
    case 0x33:  // 51 decimal - R-type (ADD, SUB, etc.) and RV32M (MUL, DIV, etc.)
      handle_type_R(mem, start_addr, log_file, symbols);
  }
}
