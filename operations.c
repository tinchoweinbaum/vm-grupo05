//habría que incluír la funcion NZ del main.c en operations.c para que las instrucciones
//justamente afecten a NZ


//faltan las instrucciones de logica de bits: shifts y puertas logicas

#include "operations.h"

void MOV(char opB, char *opA){
    *opA = opB;
}

void ADD(char opB, char *opA){
    *opA += opB;
}


void SUB(char opB, char *opA){
    *opA -= opB;
}

void MUL(char opB, char *opA){
    *opA = (*opA) * opB;
}

void DIV(char opB, char *opA, int *N , int *Z, int *error, char *ac){
    if(opB!=0){
        (*opA)/=opB;
        *ac= (*opA) % opB;
    }
    else
        *error = 1; //hacer los códigos de errores por valores de la variable
}

void CMP(char opB, char opA){
    int res = opA - opB;
}

void SHL(char opB, char *opA){

}

void SHR(char opB, char *opA){
    
}

void SAR(char opB, char *opA){
    
}

void AND(char opB, char *opA){
    (*opA) = (*opA) & opB;
}

void OR(char opB, char *opA){
    (*opA) = (*opA) | opB;
}

void XOR(char opB, char *opA){

}

void SWAP(char *opB, char *opA){
    (*opA)=(*opB);
}

void LDL(char opB, char *opA){

}

void LDL(char opB, char *opA){

}

void RND(char opB, char *opA){
    if(opB>0){
        int random = rand() % opB + 1;
        *opA = random;
    }
}