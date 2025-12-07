#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "disassemble.h"
#include "memory.h"
#include "read_elf.h"
#include "simulate.h"

const char *REGISTERS_NAMES[] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

int running = 1;
int current;
uint32_t registers[32];
char log_str[100] = {0};
char jump_str[10] = {0};

// Helper function for logging whenever a reg is edited
void log_register_edit(uint32_t rd) {
  if (rd != 0) {
    sprintf(log_str, "R[%2d] <- %x", rd, registers[rd]);
  }
}

void simulate_U(struct memory *mem, uint32_t instruction) {
  uint32_t opcode = instruction & 0x7F;
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = instruction & 0xFFFFF000;

  switch (opcode) {
  case 0x37: // LUI
    registers[rd] = imm;
    break;
  case 0x17: // AUIPC
    registers[rd] = current + imm;
    break;
  }

  log_register_edit(rd);
  current += 4;
}

void simulate_J(struct memory *mem, uint32_t instruction) {
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
    // Set all other bits after the imm bits to 1 to indicate negative (two's
    // complement)
    imm |= 0xFFE00000;
  }

  // JAL
  registers[rd] = current + 4;
  log_register_edit(rd);
  current += (int32_t)imm;
}

void simulate_B(struct memory *mem, uint32_t instruction) {
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
    // Set all other bits after the imm bits to 1 to indicate negative (two's
    // complement)
    imm |= 0xFFFFE000;
  }

  uint32_t f3 = (instruction >> 12) & 0x7;
  uint32_t rs1 = (instruction >> 15) & 0x1F;
  uint32_t rs2 = (instruction >> 20) & 0x1F;

  switch (f3) {
  case 0x0: // BEQ
    if (registers[rs1] == registers[rs2]) {
      sprintf(log_str, "{T}");
      current += (int32_t)imm;
    } else {
      sprintf(log_str, "{_}");
      current += 4;
    }
    break;
  case 0x1: // BNE
    if (registers[rs1] != registers[rs2]) {
      sprintf(log_str, "{T}");
      current += (int32_t)imm;
    } else {
      sprintf(log_str, "{_}");
      current += 4;
    }
    break;
  case 0x4: // BLT
    if ((int32_t)registers[rs1] < (int32_t)registers[rs2]) {
      sprintf(log_str, "{T}");
      current += (int32_t)imm;
    } else {
      sprintf(log_str, "{_}");
      current += 4;
    }
    break;
  case 0x5: // BGE
    if ((int32_t)registers[rs1] >= (int32_t)registers[rs2]) {
      sprintf(log_str, "{T}");
      current += (int32_t)imm;
    } else {
      sprintf(log_str, "{_}");
      current += 4;
    }
    break;
  case 0x6: // BLTU
    if (registers[rs1] < registers[rs2]) {
      sprintf(log_str, "{T}");
      current += (int32_t)imm;
    } else {
      sprintf(log_str, "{_}");
      current += 4;
    }
    break;
  case 0x7: // BGEU
    if (registers[rs1] >= registers[rs2]) {
      sprintf(log_str, "{T}");
      current += (int32_t)imm;
    } else {
      sprintf(log_str, "{_}");
      current += 4;
    }
    break;
  default:
    current += 4;
    break;
  }
}

void simulate_I_jump(struct memory *mem, uint32_t instruction) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's
    // complement)
    imm |= 0xFFFFF000;
  }

  // JALR
  registers[rd] = current + 4;
  log_register_edit(rd);
  current = registers[rs1] + (int32_t)imm;
}

void simulate_I_load(struct memory *mem, uint32_t instruction) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = (instruction >> 20) & 0xFFF;
  uint32_t rs1 = (instruction >> 15) & 0x1F;
  uint32_t f3 = (instruction >> 12) & 0x7;

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's
    // complement)
    imm |= 0xFFFFF000;
  }

  switch (f3) {
  case 0x0: // LB
    registers[rd] = memory_rd_b(mem, (int32_t)registers[rs1] + imm);
    if (registers[rd] & 0x80) {
      registers[rd] |= 0xFFFFFF00;
    }
    break;
  case 0x1: // LH
    registers[rd] = memory_rd_h(mem, (int32_t)registers[rs1] + imm);
    if (registers[rd] & 0x8000) {
      registers[rd] |= 0xFFFF0000;
    }
    break;
  case 0x2: // LW
    registers[rd] = memory_rd_w(mem, (int32_t)registers[rs1] + imm);
    break;
  case 0x4: // LBU
    registers[rd] = memory_rd_b(mem, registers[rs1] + imm);
    break;
  case 0x5: // LHU
    registers[rd] = memory_rd_h(mem, registers[rs1] + imm);
    break;
  }

  log_register_edit(rd);
  current += 4;
}

void simulate_I_imm(struct memory *mem, uint32_t instruction) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);
  uint32_t f7 = (instruction >> 25) & 0x7F;
  uint32_t shamt = (instruction >> 20) & 0x1F;

  // check the last bit of imm and check if we need to set negative
  if (imm & 0x800) {
    // Set all other bits after the imm bits to 1 to indicate negative (two's
    // complement)
    imm |= 0xFFFFF000;
  }

  switch (f3) {
  case 0x0: // ADDI
    registers[rd] = registers[rs1] + imm;
    break;
  case 0x4: // XORI
    registers[rd] = registers[rs1] ^ imm;
    break;
  case 0x6: // ORI
    registers[rd] = registers[rs1] | imm;
    break;
  case 0x7: // ANDI
    registers[rd] = registers[rs1] & imm;
    break;
  case 0x1:
    if (f7 == 0x00) { // SLLI
      registers[rd] = registers[rs1] << shamt;
    }
    break;
  case 0x5:
    if (f7 == 0x00) { // SRLI
      registers[rd] = registers[rs1] >> shamt;
    } else if (f7 == 0x20) { // SRAI
      registers[rd] = (int32_t)registers[rs1] >> shamt;
    }
    break;
  case 0x2: // SLTI
    registers[rd] = ((int32_t)registers[rs1] < (int32_t)imm) ? 1 : 0;
    break;
  case 0x3: // SLTIU
    registers[rd] = (registers[rs1] < imm) ? 1 : 0;
    break;
  }

  log_register_edit(rd);
  current += 4;
}

void simulate_I_call(struct memory *mem, uint32_t instruction) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t imm = ((instruction >> 20) & 0xFFF);
  uint32_t rs1 = ((instruction >> 15) & 0x1F);
  uint32_t f3 = ((instruction >> 12) & 0x7);

  // Ecall get register A7
  uint32_t ecall = registers[17];
  if (ecall == 1) {
    registers[10] = getchar(); // put char in a0
  } else if (ecall == 2) {
    putchar(registers[10]); // get a0 and put char
  } else if (ecall == 3 || ecall == 93) {
    running = 0; // stop simulation
  }
  current += 4;
}

void simulate_S(struct memory *mem, uint32_t instruction) {
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
  case 0x0: // SB
    memory_wr_b(mem, (int32_t)registers[rs1] + imm, registers[rs2]);
    sprintf(log_str, "%x -> Mem[%x]", registers[rs2] & 0xFF,
            (int32_t)registers[rs1] + imm);
    break;
  case 0x1: // SH
    memory_wr_h(mem, (int32_t)registers[rs1] + imm, registers[rs2]);
    sprintf(log_str, "%x -> Mem[%x]", registers[rs2] & 0xFFFF,
            (int32_t)registers[rs1] + imm);
    break;
  case 0x2: // SW
    memory_wr_w(mem, (int32_t)registers[rs1] + imm, registers[rs2]);
    sprintf(log_str, "%x -> Mem[%x]", registers[rs2],
            (int32_t)registers[rs1] + imm);
    break;
  }
  current += 4;
}

void simulate_R(struct memory *mem, uint32_t instruction) {
  uint32_t rd = (instruction >> 7) & 0x1F;
  uint32_t f3 = (instruction >> 12) & 0x7;
  uint32_t rs1 = (instruction >> 15) & 0x1F;
  uint32_t rs2 = (instruction >> 20) & 0x1F;
  uint32_t f7 = (instruction >> 25) & 0x7F;

  if (f7 != 0x01) {
    switch (f3) {
    case 0x0:
      if (f7 == 0x00) { // ADD
        registers[rd] = registers[rs1] + registers[rs2];
      } else { // SUB
        registers[rd] = registers[rs1] - registers[rs2];
      }
      break;
    case 0x4: // XOR
      registers[rd] = registers[rs1] ^ registers[rs2];
      break;
    case 0x6: // OR
      registers[rd] = registers[rs1] | registers[rs2];
      break;
    case 0x7: // AND
      registers[rd] = registers[rs1] & registers[rs2];
      break;
    case 0x1: // SLL
      registers[rd] = registers[rs1] << registers[rs2];
      break;
    case 0x5:
      if (f7 == 0x00) { // SRL
        registers[rd] = registers[rs1] >> registers[rs2];
      } else { // SRA
        registers[rd] = (int32_t)registers[rs1] >> (int32_t)registers[rs2];
      }
      break;
    case 0x2: // SLT
      registers[rd] =
          ((int32_t)registers[rs1] < (int32_t)registers[rs2]) ? 1 : 0;
      break;
    case 0x3: // SLTU
      registers[rd] = (registers[rs1] < registers[rs2]) ? 1 : 0;
      break;
    }
  } else {
    switch (f3) {
    case 0x0: // MUL
      registers[rd] = (int32_t)registers[rs1] * (int32_t)registers[rs2];
      break;
    case 0x1: { // MULH
      int64_t mul = (int64_t)registers[rs1] * (int64_t)registers[rs2];
      registers[rd] = (uint32_t)(mul >> 32); // make sure only 32 bits
      break;
    }
    case 0x2: { // MULSU
      int64_t mul = (int64_t)registers[rs1] * (uint64_t)registers[rs2];
      registers[rd] = (uint32_t)(mul >> 32); // make sure only 32 bits
      break;
    }
    case 0x3: { // MULU
      uint64_t mul = (uint64_t)registers[rs1] * (uint64_t)registers[rs2];
      registers[rd] = (uint32_t)(mul >> 32); // make sure only 32 bits
      break;
    }
    case 0x4: // DIV
      registers[rd] = (int32_t)registers[rs1] / (int32_t)registers[rs2];
      break;
    case 0x5: // DIVU
      registers[rd] = registers[rs1] / registers[rs2];
      break;
    case 0x6: // REM
      registers[rd] = (int32_t)registers[rs1] % (int32_t)registers[rs2];
      break;
    case 0x7: // REMU
      registers[rd] = registers[rs1] % registers[rs2];
      break;
    }
  }

  log_register_edit(rd);
  current += 4;
}

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file,
                     struct symbols *symbols) {
  long int insn_count = 0;
  current = start_addr;
  strcpy(jump_str, "=>");

  registers[0] = 0;
  for (int i = 1; i < 32; i++) {
    registers[i] = 0xFFFFFFFF;
  }

  while (running) {
    registers[0] = 0;
    uint32_t instruction = memory_rd_w(mem, current);
    uint32_t opcode = instruction & 0x7F;
    insn_count++;
    int old_current = current;
    uint32_t instr_addr = current; // Save address before execution changes it

    char disassembly[100] = {0};
    if (log_file) {
      disassemble(instr_addr, instruction, disassembly, 100, symbols);
    }

    switch (opcode) {
    case 0x37: // 0110111 - U-type
    case 0x17: // 0010111 - U-type
      simulate_U(mem, instruction);
      break;
    case 0x6F: // 1101111 - J-type
      simulate_J(mem, instruction);
      break;
    case 0x63: // 1100011 - B-type
      simulate_B(mem, instruction);
      break;

    // All I-type
    case 0x67: // 1100111 - I-type
      simulate_I_jump(mem, instruction);
      break;
    case 0x03: // 0000011 - I-type
      simulate_I_load(mem, instruction);
      break;
    case 0x13: // 0010011 - I-type
      simulate_I_imm(mem, instruction);
      break;
    case 0x73: // 1110011 - I-type
      simulate_I_call(mem, instruction);
      break;

    case 0x23: // 0100011 - S-type
      simulate_S(mem, instruction);
      break;
    case 0x33: // 0110011 - R-type
      simulate_R(mem, instruction);
      break;
    }

    // print after to make sure we have current instr
    if (log_file) {
      fprintf(log_file, "  %5ld %2s %8x : %08X       %-60s %s\n", insn_count,
              jump_str, instr_addr, instruction, disassembly, log_str);
      strcpy(log_str, "");
    }

    if (current != old_current + 4) {
      strcpy(jump_str, "=>");
    } else {
      strcpy(jump_str, "");
    }
  }

  return (struct Stat){.insns = insn_count};
}
