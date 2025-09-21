#include "operations.h"
#include <stdio.h>
#include <time.h>

void actNZ(maquinaV *mv,int valor){
    if(valor == 0)
        mv->regs[CC] = 0;
    else{
        if (valor > 0)
            mv->regs[CC] = 1;
        else
            mv->regs[CC] = -1;
    }
}

int NZ(maquinaV mv){
    switch (mv.regs[CC]){
        case -1: return -1; break;
        case 0: return 0; break;
        case 1: return 0; break;
    }
    return 0;
}

void readMem(maquinaV *mv){
    mv->regs[MBR] = mv->mem[mv->regs[MAR]];
}

void writeMem(maquinaV *mv){
    mv->mem[mv->regs[MAR]] = mv->regs[MBR];
}

void setValor(maquinaV *mv, int iOP, int OP, char top) { // iOP es el indice de operando, se le debe pasar OP1 o OP2 si hay que guardar funciones en el otro operando por ejemplo en el SWAP, OP es el valor extraido de GETOPERANDO
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
                
                    if ((espacio >= mv -> tablaSeg[1][0]) && (espacio < mv -> tablaSeg[1][0] + mv -> tablaSeg[1][1])){ // si el espacio en memoria es valido
                    
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

void MOV(maquinaV *mv, char tOpA, char tOpB){
    int aux;
    getValor(mv,OP2,&aux,tOpB);
    setValor(mv,OP1,aux,tOpA);
}

void ADD(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = aux1 + aux2;
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);
}

void MUL(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = aux1*aux2;
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);
}
void SUB(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = aux1 - aux2;
    setValor(mv,OP1,aux1 - aux2,tOpA);
    actNZ(mv,res);
}

void DIV(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    if(aux2 == 0)
        mv->error = 2;
    else{
        getValor(mv,OP1,&aux1,tOpA);
        res = aux1 / aux2;
        actNZ(mv,res);
        mv->regs[AC] = aux1 % aux2;
    }
}

void CMP(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    actNZ(mv,aux2 - aux1);
}

void SHL(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = (int)((unsigned int) aux1 << aux2);
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);   
}

void SHR(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = (int)((unsigned int) aux1 >> aux2);
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);   
}

void SAR(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = aux1 >> aux2;
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);   
}

void AND(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = aux2 & aux1;
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);
}

void OR(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = aux2 | aux1;
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);
}

void XOR(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, res;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    res = aux2 ^ aux1;
    setValor(mv,OP1,res,tOpA);
    actNZ(mv,res);
}

void SWAP(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    setValor(mv,OP1,aux2,tOpA);
    setValor(mv,OP2,aux1,tOpB);
}

void LDL(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    aux2 = aux2 & 0x0000FFFF;
    aux1 = aux1 & 0xFFFF0000;
    aux1 = aux1 | aux2;
    setValor(mv,OP1,aux1,tOpA);
}

void LDH(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2;
    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    aux2 = (aux2 & 0x0000FFFF) << 16;
    aux1 = aux1 & 0x0000FFFF; 
    aux1 = aux1 | aux2;
    setValor(mv,OP1,aux1,tOpA);
}

void RND(maquinaV *mv, char tOpA, char tOpB){ //No contempla valores negativos ni si opA < opB
    /*int aux2;
    srand(time(NULL));
    getValor(mv,OP2,&aux2,tOpB);
    setValor(mv,OP1,rand() % (aux2 + 1),tOpA);*/
}

void NOT(maquinaV *mv,char tOpA){
    int aux;
    getValor(mv,OP2,&aux,tOpA);
    aux = ~aux;
    setValor(mv,OP2,aux,tOpA);
    actNZ(mv,aux);
}

void SYS(maquinaV *mv){

}

/*
    IMPORTANTE: Los jumps actualmente saltan a la posición de memoria
    realtiva al CS indicada por el parámetro de su llamado. Es decir, no van
    a la línea "13" si se escribe JMP 13, si no que van a la posicion de memoria 13.
    Una posible solución a esto sería tener un vector de instrucciones cada uno con su poisición
    en memoria y hacer que los jumps vayan en memoria a donde se encuentra la instrucción señalada en este
    vector de instrucciones por su parámetro.
*/

void JMP(maquinaV *mv,int opB){
    mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void JZ(maquinaV *mv,int opB){
    if(NZ(*mv) == 0)
     mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void JP(maquinaV *mv,int opB){
    if(NZ(*mv) > 0)
     mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void JN(maquinaV *mv,int opB){
    if(NZ(*mv) < 0)
     mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void JNZ(maquinaV *mv,int opB){
    if(NZ(*mv) > 0 || NZ(*mv) < 0)
     mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void JNP(maquinaV *mv,int opB){
    if(NZ(*mv) <= 0)
     mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void JNN(maquinaV *mv, int opB){
    if(NZ(*mv) >= 0)
        mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void STOP(maquinaV *mv){
    mv->regs[IP] = 0xFF;
}