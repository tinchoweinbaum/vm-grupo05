#include <stdio.h>
#include "operations.h"

#define MEM_SIZE 16000 //16 mil bytes == 16 KiB
#define REG_SIZE 32 //32 registros en el procesador de la VM.


int is_jump(int N, int Z, char ins, char topA){
    if (ins > 0x00 && ins < 0x08 && topA == 0)
    {
        switch (ins){
            case 0x01: return 1;    //JMP 
            case 0x02: return Z;    //JZ
            case 0x03: return !N & !Z;  //JP
            case 0x04: return N;    //JN
            case 0x05: return !Z;   //JNZ
            case 0x06: return N || Z;
            case 0x07: return !N;   //JNN
        }
    }    
    return 0;
}

void two_op_fetch (char ins, char *opA, char opB){ //reescribir cuando tengamos la parte que lee el archivo de código máquina.
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



void one_op_fetch (int *inm, int *ip, char *EDX,char ins, char *opB, int N, int Z, int error, int tam){ //*EDX va en caso de que sea sys despues debemos correjir por si el registro que creamos no coincide
    if (ins > 0x00 && ins < 0x08)   //si la instruccion es salto
    {
        if (*opB < tam) //me fijo si es un salto valido
        {
            if (is_jump(N, Z, ins, topA)) //verifico la condicion
                ip = *opB;   //salto
            else
                ip +=1;    //ignoro y paso al siguiente
        } else 
            error = 1;  //si no es valido marco que hay un error en la ejecucion, esto puede servir para cortar el programa en caso de error    
    
    
    } else {
        if (ins == 0x00)    //si la instruccion es sys
        {
            if (*opB == 1)
                scanf("%d",*EDX);
            else{
                if (*opB == 2)
                    printf("%d", *EDX);
                else  
                    error = 1;
            }           
            

        } else // si la instruccion es not
            *opB = ~(*opB);     
    }
    
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



int main(){
    char mem[MEM_SIZE]; //no me gusta el nombre mem para el vector de la memoria principal xd
    char regs[REG_SIZE];
}