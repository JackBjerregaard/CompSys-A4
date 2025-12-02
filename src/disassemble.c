#include <stddef.h>
#include <stdint.h>

void disassemble(uint32_t addr, uint32_t instruction, char *result, size_t buf_size,
                 struct symbols *symbols) {
  uint32_t opcode =  instruction & 0x7F; //takes the opcode from [0:6]
  uint32_t rd = (instruction >>7) &0x1F; //bits [11:7]
  uint32_t funct3 = (instruction >>7) &0x1F; //14:12
  uint32_t rs1 = (instruction >>7) &0x1F; //19:15
  uint32_t rs2 = (instruction >>7) &0x1F; //24:20 
  uint32_t funct7 = (instruction >>7) &0x1F;  //31:25
  //
  ////extract immedaites 





  ///long switch statements for opcodes

  swithch (opcode) {
    case (0x37):
      //print stuff
    break; 

    case (2) {

    }

  }

}
