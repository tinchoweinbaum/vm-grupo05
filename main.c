#include <stdio.h>
#include "operations.h" //falta operations.h

void two_op_fetch (char ins, char *opA, char opB, int *N, int *Z){ //checkea que instruccion se llamó
    switch (ins){
        case 0x10: MOV(opA,opB);break;
        case 0x11: ADD(opA,opB);break;
        case 0x12: SUB(opA,opB);break;
        case 0x13: MUL(opA,opB);break;
        case 0x14: DIV(opA,opB);break;
        case 0x15: CMP(opA,opB);break;
        case 0x16: SHL(opA,opB);break;
        case 0x17: SHR(opA,opB);break;
        case 0x18: SAR(opA,opB);break;
        case 0x19: AND(opA,opB);break;
        case 0x1A: OR(opA,opB);break;
        case 0x1B: XOR(opA,opB);break;
        case 0x1C: SWAP(opA,opB);break;
        case 0x1D: LDL(opA,opB);break;
        case 0x1E: LDH(opA,opB);break;
        case 0x1F: RND(opA,opB);break;
    }
}

int is_jump(int N, int Z, char ins, topA){  
    if (ins > 0x00 && ins < 0x08 && topA == 0)
    {
        switch (ins){
            case 0x01: return 1; //JMP
            case 0x02: return Z; //JZ
            case 0x03: return !N & !Z; //JP
            case 0x04: return N; //JN
            case 0x05: return !Z; //JNZ
            case 0x06: return N || Z; //
            case 0x07: return !N;  //JNN      
        }
    }    
    return 0;
}



void NZ (char opA, int *N, int *Z){//debe ir inmediatamente despues de la funcion two_op_fetch  
    *N = opA >> 7;
    *Z = opA == 0;
}

char get_ins(char aux){//consigo el tipo de instruccion
    return aux & 0b00011111;
}

char get_TopA(char aux){ //consigo el tipo de operando A
    return (aux >> 4) & 0b00000011;
}

char get_TopB(char aux){//consigo el tipo de operando A
    return (aux >> 6) & 0b00000011;
}

void move_ip(unsigned int *ip, char ins, char opB, char topA){//decido si paso a la siguiente linea o jump
    if ((topA != 0)) //si el tipo de operando A es 0, es decir 1 solo operando. el de 0 operandos queda descartado por que nos movemos por la dimension fisica establecida
        (*ip)++; //paso al siguiente
    else
        if (is_jump(N,Z,topA,ins))
            *ip = opB;
}



int main(){

}