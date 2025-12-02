#include <stdint.h>
#include <stdio.h>

#include "memory.h"
#include "simulate.h"

void handle_type_U_simulate(struct memory *mem, uint32_t *current, uint32_t *registerr,
                   uint32_t full_instruction, FILE *log_file) {
  uint32_t opcode = full_instruction & 0x7F;
  uint32_t rd = (full_instruction >> 7) & 0x1F;
  int32_t imm_u = (int32_t)(full_instruction & 0xFFFFF000);

  if (opcode == 0x37) { // LUI
    registerr[rd] = imm_u;
  } else if (opcode == 0x17) { // AUIPC
    registerr[rd] = *current + imm_u;
  }

  *current += 4;
}

void handle_type_J_simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

void handle_type_B_simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

void handle_type_I_simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

void handle_type_S_simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

void handle_type_R_simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {

  uint32_t current = start_addr;
  uint32_t registerr[32] = {0};
  long int insn_count = 0;
  int running = 1;

  while (running) { // keep running until syscall to exit
    uint32_t full_instruction = memory_rd_w(mem, current);
    uint32_t opcode = full_instruction & 0x7F;
    switch (opcode) {
    case 0x37: // 0110111 - U-type
    case 0x17: // 0010111 - U-type
      handle_type_U_simulate(mem, &current, registerr, full_instruction, log_file);
      break;
    case 0x6F: // 1101111 - J-type
      handle_type_J_simulate(mem, start_addr, log_file, symbols);
      break;
    case 0x63: // 1100011 - B-type
      handle_type_B_simulate(mem, start_addr, log_file, symbols);
      break;
    case 0x67: // 1100111 - I-type
    case 0x03: // 0000011 - I-type
    case 0x13: // 0010011 - I-type
    case 0x73: // 1110011 - I-type
      handle_type_I_simulate(mem, start_addr, log_file, symbols);
      break;
    case 0x23: // 0100011 - S-type
      handle_type_S_simulate(mem, start_addr, log_file, symbols);
      break;
    case 0x33: // 0110011 - R-type
      handle_type_R_simulate(mem, start_addr, log_file, symbols);
      break;
    }

    registerr[0] = 0; // x0 always zero
    insn_count++;
  }

  return (struct Stat){.insns = insn_count};
}
