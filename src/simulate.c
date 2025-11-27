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
  uint32_t opcode = memory_rd_b(mem, start_addr) & 0x7F;

  // Determine type of instruction by using opcodes from instruction set
  switch (opcode) {
    case 0x37: 
    case 0x17:
      handle_type_U(mem, start_addr, log_file, symbols);
    case 0x6F:
      handle_type_J(mem, start_addr, log_file, symbols);
    case 0x63:
      handle_type_B(mem, start_addr, log_file, symbols);
    case 0x67:
    case 0x03:
    case 0x13:
    case 0x73:
      handle_type_I(mem, start_addr, log_file, symbols);
    case 0x23:
      handle_type_S(mem, start_addr, log_file, symbols);
    case 0x33:
      handle_type_R(mem, start_addr, log_file, symbols);
  }
}
