#include <stddef.h>
#include <stdint.h>

void disassemble(uint32_t addr, uint32_t instruction, char *result, size_t buf_size,
                 struct symbols *symbols) {
  uint32_t opcode =  instruction & 0x7F; //takes the opcode from [0:6]
}
