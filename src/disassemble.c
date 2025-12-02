#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "disassemble.h"
#include "read_elf.h"

void handle_type_U(uint32_t instruction, char *result) {}

void handle_type_J(uint32_t instruction, char *result) {}

void handle_type_B(uint32_t instruction, char *result) {}

void handle_type_I_load(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);
  
  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFFFF000;
  }

  switch (f3) {
    case 0x0:
      sprintf(result, "%-5s %d %d(%d)", "LB", rd, imm, rs1);
      break;
    case 0x1:
      sprintf(result, "%-5s %d %d(%d)", "LH", rd, imm, rs1);
      break;
    case 0x2:
      sprintf(result, "%-5s %d %d(%d)", "LW", rd, imm, rs1);
      break;
    case 0x4:
      sprintf(result, "%-5s %d %d(%d)", "LBU", rd, imm, rs1);
      break;
    case 0x5:
      sprintf(result, "%-5s %d %d(%d)", "LHU", rd, imm, rs1);
      break;
    default:
      break;
  }

}

void handle_type_I_call(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);
  
  switch (f3) {
    case 0x0:
      sprintf(result, "%-5s", "ECALL");
      break;
    default:
      break;
  }
}

void handle_type_I_jump(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFFFF000;
  }

  switch (f3) {
    case 0x0:
      sprintf(result, "%-5s %d %d %d", "JALR", rd, rs1, imm);
      break;
    default:
      break;
  }
}

void handle_type_I_imm(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);
  
  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFFFF000;
  }

  switch (f3) {
    case 0x0:
      sprintf(result, "%-5s %d %d %d", "ADDI", rd, rs1, imm);
      break;
    case 0x4:
      sprintf(result, "%-5s %d %d %d", "XORI", rd, rs1, imm);
      break;
    case 0x6:
      sprintf(result, "%-5s %d %d %d", "ORI", rd, rs1, imm);
      break;
    case 0x7:
      sprintf(result, "%-5s %d %d %d", "ANDI", rd, rs1, imm);
    //case 0x1:
    //  sprinf(result, "%-5s %d %d %d", "SLLI", rd, rs1, imm);
    //  break;
    //case 0x5:
    //  sprinf(result, "%-5s %d %d %d", "SRLI", rd, rs1, imm);
    //  break;
    //case 0x5:
    //  sprinf(result, "%-5s %d %d %d", "SRAI", rd, rs1, imm);
    //  break;
    case 0x2:
      sprintf(result, "%-5s %d %d %d", "SLTI", rd, rs1, imm);
      break;
    case 0x3:
      sprintf(result, "%-5s %d %d %d", "SLTIU", rd, rs1, imm);
      break;
    default:
      break;
  }
}

void handle_type_S(uint32_t instruction, char *result) {}

void handle_type_R(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F; 
  uint32_t f3 = (instruction >> 12) & 0x7; 
  uint32_t rs1 = (instruction >> 15) & 0x1F; 
  uint32_t rs2 = (instruction >> 20) & 0x1F; 
  uint32_t f7 = (instruction >> 25) & 0x7F; 
}

void disassemble(uint32_t addr, uint32_t instruction, char *result, size_t buf_size,
                 struct symbols *symbols) {

  // Reset result string so it doesn't contain the previous instruction
  memset(result, '\0', buf_size);

  char instruction_text[buf_size];
  memset(instruction_text, 0, buf_size);

  uint32_t opcode =  instruction & 0x7F; //takes the opcode from [0:6]
  switch (opcode) {
    case 0x37:  // 0110111 - U-type
    case 0x17:  // 0010111 - U-type
      handle_type_U(instruction, instruction_text);
      break;
    case 0x6F:  // 1101111 - J-type
      handle_type_J(instruction, instruction_text);
      break;
    case 0x63:  // 1100011 - B-type
      handle_type_B(instruction, instruction_text);
      break;

    
    case 0x67:  // 1100111 - I-type
      handle_type_I_jump(instruction, instruction_text);
      break;

    case 0x03:  // 0000011 - I-type
      handle_type_I_load(instruction, instruction_text);
      break;
    case 0x13:  // 0010011 - I-type
      handle_type_I_imm(instruction, instruction_text);
      break;
    case 0x73:  // 1110011 - I-type
      handle_type_I_call(instruction, instruction_text);
      break;


      break;
    case 0x23:  // 0100011 - S-type
      handle_type_S(instruction, instruction_text);
      break;
    case 0x33:  // 0110011 - R-type
      handle_type_R(instruction, instruction_text);
      break;
  }

  // Add the symbol to result, if applicable
  const char *symbol = symbols_value_to_sym(symbols, addr);
  if (symbol != NULL) {
    sprintf(result, "%-25s: %s", symbol, instruction_text);
  } else {
    sprintf(result, "%-25s %s", "", instruction_text);
  }
}
