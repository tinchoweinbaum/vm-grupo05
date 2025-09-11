
void MOV(mv *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->reg[OP1] = aux;
    else {
        writemem(MV);
    }
}

void ADD(mv *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->reg[OP1] += aux;
    else {
        writemem(MV);
    }
}
void MUL(mv *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->reg[OP1] *= aux;
    else {
        writemem(MV);
    }
}
void SUB(mv *MV, char topA, char topB){
    int aux;

    aux = get_op(MV,topB);

    if (topA == 0b01)
        MV->reg[OP1] -= aux;
    else
        writemem(MV);
}
