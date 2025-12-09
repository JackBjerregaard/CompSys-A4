#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "disassemble.h"
#include "read_elf.h"

const char *REGISTERS[] = {
  "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
  "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
  "t3", "t4", "t5", "t6"
};

void handle_type_U(uint32_t instruction, char *result) {
  uint32_t opcode = instruction & 0x7F;
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = instruction & 0xFFFFF000;

  switch (opcode) {
  case 0x37:
    sprintf(result + strlen(result), "%s %s, 0x%x", "LUI", REGISTERS[rd], imm >> 12);
    break;
  case 0x17:
    sprintf(result + strlen(result), "%s %s, 0x%x", "AUIPC", REGISTERS[rd], imm >> 12);
    break;
  }
}

void handle_type_J(uint32_t instruction, char *result, uint32_t addr) {
  uint32_t rd = (instruction >> 7) & 0x1f;

  uint32_t imm20 = (instruction >> 31) & 0x1;
  uint32_t imm_10_1 = (instruction >> 21) & 0x3FF;
  uint32_t imm_11 = (instruction >> 20) & 0x1;
  uint32_t imm_19_12 = (instruction >> 12) & 0xFF;

  int32_t imm = 0;
  imm |= (imm20 << 20);
  imm |= (imm_19_12 << 12);
  imm |= (imm_11 << 11);
  imm |= (imm_10_1 << 1);

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x100000) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFE00000;
  }

  // Use pseudo instruction CALL and J instead of JAL to match handed out sim output
  if (rd == 0) { // for empty returns
    sprintf(result, "J %x", (imm + addr));
  } else if (rd == 1) { // for function calls
    sprintf(result, "CALL %x", (imm + addr));
  } else {
    sprintf(result + strlen(result), "%s %s, %x", "JAL", REGISTERS[rd], (imm + addr));
  }
}

void handle_type_B(uint32_t instruction, char *result, uint32_t addr) {
  uint32_t imm_11 = (instruction >> 7) & 0x1;
  uint32_t imm_4_1 = (instruction >> 8) & 0xF;
  uint32_t imm_10_5 = (instruction >> 25) & 0x3F;
  uint32_t imm_12 = (instruction >> 31) & 0x1;

  uint32_t imm = 0;
  imm |= (imm_12 << 12);
  imm |= (imm_11 << 11);
  imm |= (imm_10_5 << 5);
  imm |= (imm_4_1 << 1);

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x1000) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFFFE000;
  }

  uint32_t f3 = (instruction >> 12) & 0x7;
  uint32_t rs1 = (instruction >> 15) & 0x1F;
  uint32_t rs2 = (instruction >> 20) & 0x1F;
  switch (f3) {
    case 0x0:
      sprintf(result, "BEQ %s, %s, %x", REGISTERS[rs1], REGISTERS[rs2], (addr + imm));
      break; 
    case 0x1:
      sprintf(result, "BNE %s, %s, %x", REGISTERS[rs1], REGISTERS[rs2], (addr + imm));
      break;
    case 0x4:
      sprintf(result, "BLT %s, %s, %x", REGISTERS[rs1], REGISTERS[rs2], (addr + imm));
      break;
    case 0x5:
      sprintf(result, "BGE %s, %s, %x", REGISTERS[rs1], REGISTERS[rs2], (addr + imm));
      break;
    case 0x6:
      sprintf(result, "BLTU %s, %s, %x", REGISTERS[rs1], REGISTERS[rs2], (addr + imm));
      break;
    case 0x7:
      sprintf(result, "BGEU %s, %s, %x", REGISTERS[rs1], REGISTERS[rs2], (addr + imm));
      break;
  }
}

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
    sprintf(result, "%s %s, %d(%s)", "LB", REGISTERS[rd], imm, REGISTERS[rs1]);
    break;
  case 0x1:
    sprintf(result, "%s %s, %d(%s)", "LH", REGISTERS[rd], imm, REGISTERS[rs1]);
    break;
  case 0x2:
    sprintf(result, "%s %s, %d(%s)", "LW", REGISTERS[rd], imm, REGISTERS[rs1]);
    break;
  case 0x4:
    sprintf(result, "%s %s, %d(%s)", "LBU", REGISTERS[rd], imm, REGISTERS[rs1]);
    break;
  case 0x5:
    sprintf(result, "%s %s, %d(%s)", "LHU", REGISTERS[rd], imm, REGISTERS[rs1]);
    break;
  default:
    break;
  }

}

void handle_type_I_call(char *result) {
  sprintf(result, "%s", "ECALL");
}

void handle_type_I_jump(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFFFF000;
  }
  
  if (rd == 0 && rs1 == 1 && imm == 0) { // Use RET for jumps to return address and discarding value
    sprintf(result, "RET");
  } else {
    sprintf(result, "%s %s, %d(%s)", "JALR", REGISTERS[rd], (int32_t)imm, REGISTERS[rs1]);
  }
}

void handle_type_I_imm(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);
  uint32_t f7 = (instruction >> 25) & 0x7F; 
  uint32_t shamt = (instruction >> 20) & 0x1F;

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's complement)
    imm |= 0xFFFFF000;
  }

  switch (f3) {
  case 0x0:
    sprintf(result, "%s %s, %s, %d", "ADDI", REGISTERS[rd], REGISTERS[rs1], imm);
    break;
  case 0x4:
    sprintf(result, "%s %s, %s, %d", "XORI", REGISTERS[rd], REGISTERS[rs1], imm);
    break;
  case 0x6:
    sprintf(result, "%s %s, %s, %d", "ORI", REGISTERS[rd], REGISTERS[rs1], imm);
    break;
  case 0x7:
    sprintf(result, "%s %s, %s, %d", "ANDI", REGISTERS[rd], REGISTERS[rs1], imm);
    break;
  case 0x1:
    if (f7 == 0x00) {
      sprintf(result, "%s %s, %s, %d", "SLLI", REGISTERS[rd], REGISTERS[rs1], shamt);
    }
    break;
  case 0x5:
    if (f7 == 0x00) {
      sprintf(result, "%s %s, %s, %d", "SRLI", REGISTERS[rd], REGISTERS[rs1], shamt);
    } else if (f7 == 0x20) {
      sprintf(result, "%s %s, %s, %d", "SRAI", REGISTERS[rd], REGISTERS[rs1], shamt);
    }
    break;
  case 0x2:
    sprintf(result, "%s %s, %s, %d", "SLTI", REGISTERS[rd], REGISTERS[rs1], imm);
    break;
  case 0x3:
    sprintf(result, "%s %s, %s, %d", "SLTIU", REGISTERS[rd], REGISTERS[rs1], imm);
    break;
  default:
    break;
  }
}

void handle_type_S(uint32_t instruction, char *result) {
  uint32_t imm_11_5 = (instruction >> 25) & 0x7F;
  uint32_t imm_4_0 = (instruction >> 7) & 0x1F;
  uint32_t imm = (imm_11_5 << 5) | imm_4_0;
  uint32_t f3 = (instruction >> 12) & 0x7;
  uint32_t rs1 = (instruction >> 15) & 0x1F; 
  uint32_t rs2 = (instruction >> 20) & 0x1F;
 
  if (imm & 0x800) {
    imm |= 0xFFFFF000;
  }

  switch (f3) {
  case 0x0:
    sprintf(result, "%s %s, %d(%s)", "SB", REGISTERS[rs2], (int32_t)imm, REGISTERS[rs1]);
    break;
  case 0x1:
    sprintf(result, "%s %s, %d(%s)", "SH", REGISTERS[rs2], (int32_t)imm, REGISTERS[rs1]);
    break;
  case 0x2:
    sprintf(result, "%s %s, %d(%s)", "SW", REGISTERS[rs2], (int32_t)imm, REGISTERS[rs1]);
    break;
  default:
    break;
  }
}

void handle_type_R(uint32_t instruction, char *result) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t f3 = (instruction >> 12) & 0x7;
  uint32_t rs1 = (instruction >> 15) & 0x1F;
  uint32_t rs2 = (instruction >> 20) & 0x1F;
  uint32_t f7 = (instruction >> 25) & 0x7F;

  if (f7 != 0x01) {
    switch (f3) {
      case 0x0:
        if (f7 == 0x00) {
          sprintf(result, "ADD %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        } else {
          sprintf(result, "SUB %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        }
        break;
      case 0x4:
        sprintf(result, "XOR %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x6:
        sprintf(result, "OR %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x7:
        sprintf(result, "AND %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x1:
        sprintf(result, "SLL %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x5:
        if (f7 == 0x00) {
          sprintf(result, "SRL %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        } else {
          sprintf(result, "SRA %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        }
        break;
      case 0x2:
        sprintf(result, "SLT %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x3:
        sprintf(result, "SLTU %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
    }
  } else {
    switch (f3) {
      case 0x0:
        sprintf(result, "MUL %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x1:
        sprintf(result, "MULH %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x2:
        sprintf(result, "MULSU %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x3:
        sprintf(result, "MULU %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x4:
        sprintf(result, "DIV %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x5:
        sprintf(result, "DIVU %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x6:
        sprintf(result, "REM %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
      case 0x7:
        sprintf(result, "REMU %s, %s, %s", REGISTERS[rd], REGISTERS[rs1], REGISTERS[rs2]);
        break;
    }
  }
}

void disassemble(uint32_t addr, uint32_t instruction, char *result, size_t buf_size,
                 struct symbols *symbols) {

  // Reset result string so it doesn't contain the previous instruction
  memset(result, '\0', buf_size);

  char instruction_text[buf_size];
  memset(instruction_text, 0, buf_size);

  uint32_t opcode = instruction & 0x7F; // takes the opcode from [0:6]
  switch (opcode) {
  case 0x37:  // 0110111 - U-type
  case 0x17:  // 0010111 - U-type
    handle_type_U(instruction, instruction_text);
    break;
  case 0x6F:  // 1101111 - J-type
    handle_type_J(instruction, instruction_text, addr);
    break;
  case 0x63:  // 1100011 - B-type
    handle_type_B(instruction, instruction_text, addr);
    break;

  // All I-type
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
    handle_type_I_call(instruction_text);
    break;

  // All S-type
  case 0x23:  // 0100011 - S-type
    handle_type_S(instruction, instruction_text);
    break;
  
  // All R-type
  case 0x33:  // 0110011 - R-type
    handle_type_R(instruction, instruction_text);
    break;
  default:
    sprintf(instruction_text, "Unknown Instruction");
    break;
  }


  // Add the symbol to result, if applicable
  const char *symbol = symbols_value_to_sym(symbols, addr);
  if (symbol != NULL) {
    sprintf(result, "%-20s:   %s", symbol, instruction_text);
  } else {
    sprintf(result, "%-21s   %s", "", instruction_text);
  }
}
