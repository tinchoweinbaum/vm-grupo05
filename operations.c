#include "operations.h"
#include <stdio.h>

void readMem(maquinaV *mv){
    mv->regs[MBR] = mv->mem[mv->regs[MAR]];
}

void writeMem(maquinaV *mv){
    mv->mem[mv->regs[MAR]] = mv->regs[MBR];
}

void setValor(maquinaV *mv, int iOP, int OP, char top) { // iOP es el indice de operacion, se le debe pasar OP1 o OP2 si hay que guardar funciones en el otro operando por ejemplo en el SWAP, OP es el valor extraido de GETOPERANDO
   int offset,reg,espacio;

    if (top == 1){ // registro 
        if (mv -> regs[iOP]>= 0 && mv -> regs[iOP]<= 31) 
            mv -> regs[mv -> regs[iOP]] = OP;            
        else 
            mv -> error = 1;
    } else {
        if(top == 3){ //memoria

            reg = mv -> regs[iOP] >> 16;//cargo el registro
                
                if (reg >= 0 && reg <= 31){ // si es un registro valido

                    offset = mv -> regs[iOP] & 0x00FF; //cargo el offset
                    espacio = mv -> regs[reg] + offset; // cargo el espacio en memoria
                
                    if ((espacio >= mv -> tablaseg[1][0]) && (espacio < mv -> tablaseg[1][0] + mv -> tablaseg[1][1])){ // si el espacio en memoria es valido
                    
                        mv -> mem[espacio] = OP; // guardo el valor

                    } else 
                        mv -> error = 1; // si no error 1
                } else 
                    mv -> error = 1;// si no es un registro valido error 1
        }
    } 
}


void getValor(maquinaV *mv,int iOP, int *OP, char top) {
    int offset, reg;

    if (top == 2) // inmediato
        
        *OP = mv->regs[iOP];


    else if (top == 1) { // registro
        *OP = mv->regs[mv->regs[iOP]];
    } 
    else { // memoria
        offset = mv->regs[iOP] & 0x00FF;
        reg = mv -> regs[iOP] >> 16;
        if ((mv->regs[reg] + offset > mv->tablaSeg[1][1]) & ((mv->regs[reg] + offset < mv->tablaSeg[0][1])) ) {
            mv->error = 1; // me caigo del data segment
        } else {
            *OP = mv->mem[mv->regs[iOP] + offset];
        }
    }
}

void MOV(maquinaV *MV, char topA, char topB){
    int aux1,aux2;
    getValor(MV,&aux1,topA);
    getValor(MV,&aux2,topB);

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
