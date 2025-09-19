void MOV(mv *MV, char topA, char topB, int opA, char opB){
    int aux;
    aux = get_op(MV, topB, opB);

    switch (topA){
        case 0b01: *MV.reg[opA] = aux; break;
        case 0b11: *MV.mem[opA] = aux; break;
    }
}

void ADD(mv *MV, char topA, char topB, int *opA, char opB){
    int aux;
    aux = get_op(*MV, topB, opB);

    switch (topA){
        case 0b01: MV.reg[opA] += aux; break;
        case ob10: *opA += aux; break;
        case 0b11: *MV.mem[opA] += aux; break;
    }
}

void MUL(mv *MV, char topA, char topB, int *opA, char opB){
    int aux;
    aux = get_op(*MV, topB, opB);

    switch (topA){
        case 0b01: MV.reg[opA] *= aux; break;
        case ob10: *opA *= aux; break;
        case 0b11: *MV.mem[opA] *= aux; break;
    }
}

void SUB(mv *MV, char topA, char topB, int *opA, char opB){
    int aux;
    aux = get_op(*MV, topB, opB);

    switch (topA){
        case 0b01: MV.reg[opA] -= aux; break;
        case ob10: *opA -= aux; break;
        case 0b11: *MV.mem[opA] -= aux; break;
    }
}
