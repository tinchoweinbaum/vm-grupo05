#include <operations.h>
#include <stdio.h>

void MOV(maquinaV *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->regs[OP1] = aux;
    else {
        writemem(MV);
    }
}

void ADD(maquinaV *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->regs[OP1] += aux;
    else {
        writemem(MV);
    }
}
void MUL(maquinaV *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->regs[OP1] *= aux;
    else {
        writemem(MV);
    }
}
void SUB(maquinaV *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->regs[OP1] -= aux;
    else
        writemem(MV);
}

void STOP(maquinaV *mv){
    mv->regs[IP] = 0xFFFFFFFF;
}
