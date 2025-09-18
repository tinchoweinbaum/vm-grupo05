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

void MUL(maquinaV *MV, char topA, char topB){
    
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

void STOP(maquinaV *mv){

}

void SYS(maquinaV *mv,char opB){

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
