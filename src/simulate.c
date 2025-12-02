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
  // 1
  // 2
  uint32_t opcode = memory_rd_b(mem, start_addr) & 0x7F;

  // 3
  switch (opcode) {
    case 0x37:  // 4
    case 0x17:  // 5
      handle_type_U(mem, start_addr, log_file, symbols);
      break;
    case 0x6F:  // 6
      handle_type_J(mem, start_addr, log_file, symbols);
      break;
    case 0x63:  // 7
      handle_type_B(mem, start_addr, log_file, symbols);
      break;
    case 0x67:  // 8
    case 0x03:  // 9
    case 0x13:  // 10
    case 0x73:  // 11
      handle_type_I(mem, start_addr, log_file, symbols);
      break;
    case 0x23:  // 12
      handle_type_S(mem, start_addr, log_file, symbols);
      break;
    case 0x33:  // 13
      handle_type_R(mem, start_addr, log_file, symbols);
      break; 
  }
}
