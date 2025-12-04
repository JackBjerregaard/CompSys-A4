#include <stdint.h>
#include <stdio.h>

#include "memory.h"
#include "simulate.h"
#include "disassemble.h"

int running = 1;
int current = 0;

void simulate_U(struct memory *mem, uint32_t instruction) {
  uint32_t opcode = instruction & 0x7F;
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = instruction & 0xFFFFF000;

  switch (opcode) {
  case 0x37:
    memory_wr_w(mem, current >> 7, rd); // Insert to rd 
    memory_wr_w(mem, current >> 12, imm);   
    break;
  case 0x17:
    break;
  }
}

void simulate_J(struct memory *mem, uint32_t instruction) {}

void simulate_B(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

void simulate_I(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

void simulate_S(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

void simulate_R(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {}

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols *symbols) {
  long int insn_count = 0;
  current = start_addr; // Get the first instruction
  
  while (running) { // keep running until syscall to exit
    uint32_t full_instruction = memory_rd_w(mem, current);
    uint32_t opcode = full_instruction & 0x7F;
    switch (opcode) {
    case 0x37: // 0110111 - U-type
    case 0x17: // 0010111 - U-type
      simulate_U(mem, full_instruction);
      break;
    case 0x6F: // 1101111 - J-type
      simulate_U(mem, full_instruction);
      break;
    case 0x63: // 1100011 - B-type
      simulate_B(mem, start_addr, log_file, symbols);
      break;
    case 0x67: // 1100111 - I-type
    case 0x03: // 0000011 - I-type
    case 0x13: // 0010011 - I-type
    case 0x73: // 1110011 - I-type
      simulate_I(mem, start_addr, log_file, symbols);
      break;
    case 0x23: // 0100011 - S-type
      simulate_S(mem, start_addr, log_file, symbols);
      break;
    case 0x33: // 0110011 - R-type
      simulate_R(mem, start_addr, log_file, symbols);
      break;
    }

    insn_count++;
  }

  return (struct Stat){.insns = insn_count};
}
