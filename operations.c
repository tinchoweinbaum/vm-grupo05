#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
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
        case 1: return 1; break;
    }
    return 0;
}

void escribeIntMem(maquinaV *mv, int dir, int valor) {
    if (dir < mv->tablaSeg[1][0] || dir + 3 >= mv->tablaSeg[1][0] + mv->tablaSeg[1][1]) {
        mv->error = 1;
        return;
    }
    for (int i = 0; i < 4; i++) {
        unsigned char byte = (valor >> (8 * (3 - i))) & 0xFF;
        mv->mem[dir + i] = byte;
    }
    mv->regs[MAR] = dir;
    mv->regs[MBR] = valor;
    mv->regs[LAR] = dir - mv->tablaSeg[1][0];
}

void leeIntMem(maquinaV *mv, int dir, int *valor) {
    *valor = 0;

    if (dir < mv->tablaSeg[1][0] || dir + 3 >= mv->tablaSeg[1][0] + mv->tablaSeg[1][1]) {
        mv->error = 1;
        return;
    }

    for (int i = 0; i < 4; i++) {
        *valor = (*valor << 8) | (unsigned char)mv->mem[dir + i];
    }
    //actualizo los registros
    mv -> regs[MAR] = dir;
    mv -> regs[MBR] = *valor;
    mv -> regs[LAR] = dir - mv -> tablaSeg[1][0];
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
                
                    if ((espacio + 3>= mv -> tablaSeg[1][0]) && (espacio + 3 < mv -> tablaSeg[1][0] + mv -> tablaSeg[1][1])) // si el espacio en memoria es valido
                        escribeIntMem(mv,espacio,OP); // guardo el valor
                    else 
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
        reg = mv->regs[iOP] >> 16;
        int dir = mv->regs[reg] + offset;

        if (dir < mv->tablaSeg[1][0] || dir + 3 >= mv->tablaSeg[1][0] + mv->tablaSeg[1][1]) {
            mv->error = 1;
        } else {
            leeIntMem(mv, dir, OP);
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
    //printf("\nVOY A SUMAR %d y %d",aux1,aux2);
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
    if(aux2 == 0){
        mv->error = 2;
        return;
    }else{
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
    actNZ(mv,aux1 - aux2);
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
    unsigned int aux1, aux2;
    getValor(mv, OP2, (int*)&aux2, tOpB);
    getValor(mv, OP1, (int*)&aux1, tOpA);

    aux2 = aux2 & 0x0000FFFF;  
    aux1 = aux1 & 0xFFFF0000;   
    aux1 = aux1 | aux2;      

    setValor(mv, OP1, (int)aux1, tOpA);
}

void LDH(maquinaV *mv, char tOpA, char tOpB){
    unsigned int aux1, aux2;
    getValor(mv, OP2, (int*)&aux2, tOpB);
    getValor(mv, OP1, (int*)&aux1, tOpA);

    aux2 = (aux2 & 0x0000FFFF) << 16;  
    aux1 = aux1 & 0x0000FFFF;          
    aux1 = aux1 | aux2;                

    setValor(mv, OP1, (int)aux1, tOpA);
}

void RND(maquinaV *mv, char tOpA, char tOpB){ 
    int aux2, max, min;
    srand(time(NULL));
    getValor(mv,OP2,&aux2,tOpB);
    if(aux2 < 0){
        max = 0;
        min = aux2;
    } else {
        max = aux2;
        min = 0;
    }
    setValor(mv,OP1,min + rand() % (max - min + 1),tOpA);
}

void NOT(maquinaV *mv,char tOpA){
    int aux;
    getValor(mv,OP2,&aux,tOpA);
    aux = ~aux;
    setValor(mv,OP2,aux,tOpA);
    actNZ(mv,aux);
}

void binario(int val) {
    int i;
    int bit;

    for (i = 16; i >= 0; i--) {
        bit = (val >> i) & 1;
        printf("%d", bit);
    }

    printf("\n");
}


void SYS(maquinaV *mv){
    int bytes, pos,inicio, tipo, orden, n, i, j, val, base, limite; //val = valor a escribir/leer

    orden = mv -> regs[OP2]; //operando del SYS
    pos = mv->regs[EDX]; //donde escribir/leer en memoria
    tipo = mv->regs[EAX]; //en que base están los datos a escribir/leer
    bytes = mv->regs[ECX] >> 16; //cant. de bytes a escribir/leer
    n = mv->regs[ECX] & 0xFFFF; //tamaño de los datos a escribir
    base = mv->tablaSeg[1][0]; //inicio del segmento
    limite = mv->tablaSeg[1][0] + mv->tablaSeg[1][1]; //final del segmento
    //printf("BYTES %x N %x\n", bytes, n);

    if (pos >= base && pos + bytes * n < limite){ //si no me salgo del segmento
        
        if (n != 0 && bytes != 0){ //si voy a leer o escribir algo
            if (orden == 2){ // si escribo

                val = 0;
                for ( i = 0; i < n; i++)
                {
                    inicio = pos;
                    for ( j = 0; j < bytes; j++)
                    {
                        val = (val << 8) | mv->mem[pos];
                        pos++;
                    }
                    printf("\n[%04X]: ", inicio);
                    if(tipo & 0x10){binario(val);}
                    if(tipo & 0x08){printf(" 0x%X\t", val);}
                    if(tipo & 0x04){printf(" 0o%o\t", val);}
                    if(tipo & 0x02){printf(" %c\t", (char)val);}
                    if(tipo & 0x01){printf(" %d\t", val);}
                    printf("\n");

                }
                
            }else{

                if (orden == 1){
                    for ( i = 0; i < n; i++)
                    {

                        
                        if (tipo & 0x10)
                        {
                            char bit;
                            val = 0;
                            j = 0;
                            scanf("%c",&bit);
                            while (j < (bytes * 8) && (bit == '1' || bit == '0'))
                            {
                                val = (val << 1);
                                if (bit == '1') val = val | 1;
                                scanf("%c",&bit);
                                j++;
                            }                       
                            
                            
                        } else {
                                        
                            if(tipo & 0x08)scanf("%x", &val); else
                            if(tipo & 0x04)scanf("%o", &val); else 
                            if(tipo & 0x02)scanf("%c", (char*)&val); else
                            if(tipo & 0x01)scanf("%d", &val); 
                            else {
                                mv -> error = 3;
                                return;
                            }
                        }
                        escribeIntMem(mv,pos,val);

                    }   
                } else {
                    mv -> error = 3;
                    return;
                }
                
            }
            
        }
        
    } else {
        mv -> error = 1;
        return;
    }    
}

void JMP(maquinaV *mv,int opB){
        mv->regs[IP] = mv->tablaSeg[0][0] + opB;

}

void JZ(maquinaV *mv,int opB){
    if(NZ(*mv) == 0)
     mv->regs[IP] = mv->tablaSeg[0][0] + opB;
}

void JP(maquinaV *mv,int opB){
    if(NZ(*mv) > 0)
        mv->regs[IP] = mv->tablaSeg[0][0] + opB;;
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


