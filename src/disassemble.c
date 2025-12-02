#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "disassemble.h"
#include "read_elf.h"

void handle_type_U(uint32_t instruction, char *result) {}

void handle_type_J(uint32_t instruction, char *result) {}

void handle_type_B(uint32_t instruction, char *result) {}

void handle_type_I(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFFFF000;
  }

  printf("%d\t", rd);
  printf("%d", imm);
}

void handle_type_S(uint32_t instruction, char *result) {}

void handle_type_R(uint32_t instruction, char *result) {}

void disassemble(uint32_t addr, uint32_t instruction, char *result, size_t buf_size,
                 struct symbols *symbols) {

  // Reset result string so it doesn't contain the previous instruction
  memset(result, '\0', buf_size);

  // Add the symbol to result, if applicable
  const char *symbol = symbols_value_to_sym(symbols, addr);
  if (symbol != NULL) {
    sprintf(result, "%-25s:", symbol);
  }

  uint32_t opcode =  instruction & 0x7F; //takes the opcode from [0:6]
  switch (opcode) {
    case 0x37:  // 0110111 - U-type
    case 0x17:  // 0010111 - U-type
      handle_type_U(instruction, result);
      break;
    case 0x6F:  // 1101111 - J-type
      handle_type_J(instruction, result);
      break;
    case 0x63:  // 1100011 - B-type
      handle_type_B(instruction, result);
      break;
    case 0x67:  // 1100111 - I-type
    case 0x03:  // 0000011 - I-type
    case 0x13:  // 0010011 - I-type
    case 0x73:  // 1110011 - I-type
      handle_type_I(instruction, result);
      break;
    case 0x23:  // 0100011 - S-type
      handle_type_S(instruction, result);
      break;
    case 0x33:  // 0110011 - R-type
      handle_type_R(instruction, result);
      break;
  }
}
