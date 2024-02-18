#include <stdio.h>
#include <stdlib.h>
#include "core16.h"
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

word memory[memory_addr_space];
word regs[regNum]; //PC, SP, r1-r14


void read_memory(char* path){
  
  FILE* ram;
  ram = fopen(path, "rb");
  
  fread(memory, 2, memory_addr_space, ram);
}


void dump(){

  for (int i=0; i<regNum; i++){
    if (i==PC)
      printf("%s", "PC ");
    else if (i==SP)
      printf("%s", "SP ");
    else if (i==FP)
	printf("%s", "FP ");
    else
      printf("%s%d", "R", i);
    

    if ((i) < 10)
	    printf(" ");
    printf(" : ");
    printf("%04x  ", regs[i]);
    if ((i+1)%4 == 0)
	    printf("\n");
    }
  printf("inst: %04x\n", memory[regs[PC]]);
}

void mem_dump(word start, word end){
  for (int i=start; i<=end; i++){
    printf("%04x: %04x  ", i, memory[i]);
    if (i%5 == 0)
      printf("\n");
  }
}

void saveStateFile(char* path){
  FILE* stateFile = fopen(path, "ab");
  fwrite(memory, 2, memory_addr_space, stateFile);
  fwrite(regs, 2, regNum, stateFile);

}

void loadStateFile(char* path){
  word temp[memory_addr_space+16];
  FILE* stateFile = fopen(path, "rb");
  fread(temp, 2, memory_addr_space+16, stateFile);
  memcpy(memory, temp, memory_addr_space*2);
  memcpy(regs, &temp[memory_addr_space], 32);
}

void ramFileDump(char* path){
  FILE* ramFile = fopen(path, "wb");
  fwrite(memory, 2, memory_addr_space, ramFile);
}

int main(int argc, char** argv){
  char* ramFile;
  bool dumpMemory = false;
  bool saveState = false;
  bool loadState = false;

  bool clock = true;
  word inst, opcode;
  __int16_t temp;
  word args = 0;  printf("\n"); //0000LNCZR1R1R1R1XXXX

  
  if (!loadState){
  read_memory("rom.bin"); //read main memory (ram) file
  regs[PC] = memory[ENTRY]; //program entry point
  regs[SP] = STACK;
  }else
    loadStateFile("state.bin");

  while (clock){ 
    dump(); //CPU status

    //fetch
    inst = memory[regs[PC]];
    regs[PC]++;

    //decode
    opcode = (inst & 0x7f); //opcode
    args = (args&0xff00) | ((inst & 0xff00)>>8);
    int8 r1 = (args&0x0f);
    int8 r2 = (args&0xf0)>>4;
    
    //Excute
    switch(opcode){
      case 0x0:
          break;
      case 0x1: //LOAD
          regs[r1] = memory[memory[regs[PC]]];
          regs[PC]++;
          break;
      case 0x2: //SOTRE
          memory[memory[regs[PC]]]= regs[r1];
          regs[PC]++;
          break;
      case 0x3: //MOVE
          regs[r2] = regs[r1];
          break;
      case 0x4: //MOVE IMM
          regs[r1] = memory[regs[PC]];
          regs[PC]++;
          break;
      case 0x16: //LOAD MEM[REG[r2]]
          regs[r1] = memory[regs[r2]];
          break;
      case 0x18: //STORE MEM[REG[r2]]
          memory[regs[r2]] = regs[r1];
          break;
      case 0x5: //ADD
          temp = regs[r1] + regs[r2];
          regs[r1] = temp;
          break;
      case 0x6: //SUB
          temp = regs[r1] - regs[r2];
          regs[r1] = temp;
      case 0x7: //AND
          temp = regs[r1] &regs[r2];
          regs[r1] = temp;
          break;
      case 0x8: //OR
          temp = regs[r1] | regs[r2];
          regs[r1] = temp;
          break;
      case 0x9: //XOR
          temp = regs[r1] ^ regs[r2];
          regs[r1] = temp;
          break;
      case 0xa: //SHIFT LEFT
          temp = regs[r1] << regs[r2];
          regs[r1] = temp;
          break; //SGIFT RIGHT
      case 0xb:
          temp = regs[r1] >> regs[r2];
          regs[r1] = temp;          break;
      case 0xc: //INC
          regs[r1]++;
          temp = regs[r1];
          break;
      case 0xd: //DEC
          regs[r1]--;
          temp = regs[r1];
        break;
      case 0xe: //CMP
          temp = regs[r1] - regs[r2];
          if ( temp < 0){
            args = args | 0x0800;
            args = args & 0xfeff;
          }
        break;
      case 0x10: //SPS
          regs[SP] = memory[regs[PC]];
          regs[FP] = regs[SP];
          regs[PC]++;
          break;
      case 0x11:  //PUSH
          memory[regs[SP]] = regs[r1];
          regs[SP]--;
	        break;
      case 0x12: //POP
	        regs[SP]++;
          regs[r1] = memory[regs[SP]];
          break;
      case 0xf: //HALT
          clock = false;
	        break;
      case 0x13: //CALL
          memory[regs[SP]] = regs[PC]+1;
          regs[SP]--;
          memory[regs[SP]] = regs[FP];
          regs[SP]--;
          regs[FP] = regs[SP];
          regs[PC] = memory[regs[PC]];
          break;
      case 0x14: //RET
          regs[SP] = regs[FP]+2;
          regs[PC] = memory[regs[SP]];
          regs[FP] = memory[regs[SP]-1];
          break;
      case 0x15: //SYS
          memory[regs[SP]] = regs[PC];
          regs[SP]--;
          regs[PC] = memory[INT_VECT];
          break;
      case 0x17: //CALL MEM[REG[r1]]
	        memory[regs[SP]] = regs[PC];
          regs[SP]--;
          memory[regs[SP]] = regs[FP];
          regs[SP]--;
          regs[FP] = regs[SP];
          regs[PC] = regs[r1];
          break;
      case 0x19: //JMP
          regs[PC] = memory[regs[PC]];
          break;
      case 0x1a: //JIC
          if (((args&0x100)>>8) == 1)
            regs[PC] = memory[regs[PC]];
          else
            regs[PC]++;
          break;
      case 0x1b: //JOC
          if (((args&0x200)>>9) == 1)
            regs[PC] = memory[regs[PC]];
          else
            regs[PC]++;
            break;
      case 0x1c:
        regs[r1] = !regs[r1];
    }
    //Cheking destination register and setting correspondig flag bits
    if(opcode > 0x4 && opcode < 0xf){
      if (temp == 0){
        args = args | 0x0100;
        args = args & 0xf7ff;
      }
    }
    printf("LNCZ: %x\n\n\n", (args&0x0f00)>>8); //Print CPU status bits

  }

  return 0;
}
