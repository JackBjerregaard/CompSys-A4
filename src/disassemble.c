#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "disassemble.h"
#include "read_elf.h"

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
      break;
    case 0x6F:  // 1101111 - J-type
      break;
    case 0x63:  // 1100011 - B-type
      break;
    case 0x67:  // 1100111 - I-type
    case 0x03:  // 0000011 - I-type
    case 0x13:  // 0010011 - I-type
    case 0x73:  // 1110011 - I-type

      break;
    case 0x23:  // 0100011 - S-type
      break;
    case 0x33:  // 0110011 - R-type
      break;
  }

}
