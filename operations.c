#include "operations.h"
#include <stdio.h>

void readMem(maquinaV *mv){
    mv->regs[MBR] = mv->mem[mv->regs[MAR]];
}

void writeMem(maquinaV *mv){
    mv->mem[mv->regs[MAR]] = mv->regs[MBR];
}

void getValor(maquinaV *mv, int *opB, char topB) {
    int offset, reg;

    if (topB == 2) { // inmediato
        *opB = mv->regs[OP2];
    } 
    else if (topB == 1) { // registro
        *opB = mv->regs[mv->regs[OP2]];
    } 
    else { // memoria
        offset = mv->regs[OP2] & 0x00FF;
        reg = mv -> regs[OP2] >> 16;
        if ((mv->regs[reg] + offset > mv->tablaSeg[1][1]) & ((mv->regs[reg] + offset < mv->tablaSeg[0][1])) ) {
            mv->error = 1; // me caigo del data segment
        } else {
            *opB = mv->mem[mv->regs[OP2] + offset];
        }
    }
}

void MOV(maquinaV *MV, char topA, char topB){
    int aux1,aux2;
    getValor(mv,&aux1,topA);
    getValor(mv,&aux2,topB);
    
}

void ADD(maquinaV *MV, char topA, char topB){

}
void MUL(maquinaV *MV, char topA, char topB){

}
void SUB(maquinaV *MV, char topA, char topB){

}

void DIV(maquinaV *MV, char topA, char topB){

}

void CMP(maquinaV *MV, char topA, char topB){

}

void SHL(maquinaV *MV, char topA, char topB){

}

void SHR(maquinaV *MV, char topA, char topB){

}

void SAR(maquinaV *MV, char topA, char topB){

}

void AND(maquinaV *MV, char topA, char topB){

}

void OR(maquinaV *MV, char topA, char topB){

}

void XOR(maquinaV *MV, char topA, char topB){

}

void SWAP(maquinaV *MV, char topA, char topB){

}

void LDL(maquinaV *MV, char topA, char topB){

}

void LDH(maquinaV *MV, char topA, char topB){

}

void RND(maquinaV *MV, char topA, char topB){

}


void SYS(maquinaV *mv){

}

void JZ(maquinaV *mv){

}

void JP(maquinaV *mv){

}

void JN(maquinaV *mv){

}

void JNZ(maquinaV *mv){

}

void JNP(maquinaV *mv){

}

void STOP(maquinaV *mv){
    mv->regs[IP] = 0xFFFFFFFF;
}
