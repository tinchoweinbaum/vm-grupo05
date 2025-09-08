//habría que incluír la funcion NZ del main.c en operations.c para que las instrucciones
//justamente afecten a NZ


//faltan las instrucciones de logica de bits: shifts y puertas logicas

#include "operations.h"

void MOV(char opB, char *opA, int *N, int *Z){
    *opA = opB;
}

void ADD(char opB, char *opA, int *N, int *Z){
    *opA += opB;
    *N = (*opA + opB) < 0;
    *Z = (*opA + opB) == 0;
}


void SUB(char opB, char *opA, int *N, int *Z){
    *opA -= opB;
    *N = (*opA - opB) < 0;
    *Z = (*opA - opB) == 0;
}

void MUL(char opB, char *opA, int *N, int *Z){
    *opA = (*opA) * opB;
    *N = ((*opA) * opB) == 0;
    *Z = (*opA == 0) || (opB == 0);
}

void DIV(char opB, char *opA, int *N , int *Z){ //se llama siempre con opB!=0? si opB==0 que hace?
    if(opB!=0){
        (*opA)/=opB;
        *N = (*opA)/=opB == 0;
        *Z = (*opA)== 0;
    }
}

void CMP(char opB, char opA, int *N, int *Z){
    int res= opA - opB;
    *N = res < 0;
    *Z = res == 0;
}

void SHL(char opB, char *opA){

}

void SHR(char opB, char *opA){
    
}

void SAR(char opB, char *opA){
    
}

void AND(char opB, char *opA, int *N, int *Z){

}

void OR(char opB, char *opA, int *N, int *Z){

}

void XOR(char opB, char *opA, int *N, int *Z){

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