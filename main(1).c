include <stdio.h>
#include <stdlib.h>
#include "operations.h"

/**********REGISTROS***********/

#define LAR 0
#define MAR 1
#define MBR 2

#define IP 3
#define OPC 4
#define OP1 5
#define OP2 6

#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15

#define AC 16
#define CC 17

#define CS 26
#define DS 27

/***************TAMANIOS**************/

#define MEM_SIZE 16384 //16384 bytes == 16 KiB
#define REG_SIZE 32 //32 registros en el procesador de la VM.
#define HEADER_SIZE 7 //el encabezado ocupa del byte 0 al 7 de un archivo

typedef struct maquinaV{
    char mem[MEM_SIZE]; //vector de memoria
    char regs[REGS_SIZE]; //vector de registros
    int tablaSeg[1][1]; // tabla de segmentos: matriz de 2x2
} maquinaV;

void writeMem(maquinaV *mv, int aux, char topB){
    int marAux;
    marAux = pos_mem(mv,1,topB);
    if (marAux == -1)
        mv->error = 1;
    else{
        mv->regs[MAR] = marAux;
        mv->mem[MAR] = aux;
    }
}

int pos_mem(maquinaV mv, int nop, char top){//posicion para cargar el MAR/ la variable nop indica con 1 o 2 si necesito la posicion del vector 1 o 2
    if(top == 0x01){ //si la posicion en memoria es un registro
        if (nop == 1){
            if (mv.regs[OP1] >= 0 && mv.regs[OP1] < REG_SIZE) //si el operando apunta a un registro valido devuelvo el valor del registro al que apunta
                return mv.regs[mv.regs[OP1]];
            else 
                return -1; //segmentation fault retorno 1 por que es una funcion entera no puedo modificar aca
        } else {
            if (mv.regs[OP2] >= 0 && mv.regs[OP2] < REG_SIZE)
                return mv.regs[mv.regs[OP2]]; 
            else
                return -1; //segmentation fault
        }
        
    } else { //es un inmediato      
        if (nop == 1){
            if (mv.regs[OP1] >= mv.tablaSeg[1][0] && mv.regs[OP1] < mv.tablaSeg[1][0] + mv.tablaSeg[1][1]) //si el operando apunta a una posicion valida devuelvo el valor del operando
                return mv.regs[OP1];
            else 
                return -1; //segmentation fault
        } else {
            if (mv.regs[OP2] >= mv.tablaSeg[1][0] && mv.regs[OP2] < mv.tablaSeg[1][0] + mv.tablaSeg[1][1])
                return mv.regs[OP2]; 
            else
                return -1; //segmentation fault
        }
    }
}




void readFile(FILE *arch, maquinaV mv, int *error, int flagD) { //parámetros a definir, pero lo importante serían los vectores de memoria y registros que están dentro del struct maquinaV
    //esta función se llama SÓLO después de verificar que existe el archivo.
    //falta implementar el flag -d para el disassembler.
    char byteAct;
    int tamCod = 0;
    int tOpA, tOpB, ins = 0;
    for(int i = 0; i <= HEADER_SIZE-2; i++) { //lee el header del archivo, excluyendo el tamaño del código
        fread(byteAct, 1, sizeof(byteAct), arch);
        printf("%c", byteAct); //printea VMX25
        printf("\n");
    }
    for(int i = HEADER_SIZE-2; i <= HEADER_SIZE; i++) { //lee el tamaño del codigo
        fread(byteAct, 1, sizeof(byteAct), arch);
        tamCod += byteAct;
    }
    
    if(tamCod > mv.tablaSeg[0][1]) 
        printf("El código supera el tamaño máximo."); 
    else { //el código del programa entra en el CS de la memoria.
        while(fread(byteAct,1,sizeof(byteAct),arch) || codOp != ins){ //ciclo principal de lectura, conviene hacer for? tengo el tamaño del código en tamCod.
            //frena al leer todo el archivo || encontrar el mnemónico STOP
            ins = byteAct & 0x1F;

            //tOpaA = tOpA
        }
    }
    fclose(arch);
}

void setReg(maquinaV *mv,int index_reg, char val){
    mv.regs[index_reg]=val;
}

char getReg(maquinaV mv, int index_reg){
    return mv.regs[index_reg];
}

int getOp(mv *mv, char topB){
    int marAux;
    if (topB == 0b01){
        if (mv->regs[OP2] >= 0 && mv->regs[OP2] < 32)
        {
            return mv.reg[mv->regs[OP2]];
        } else {
            mv->error = 1 // segmentation fault
        }
    }else {
        if (topB == 0b10)
            return mv->regs[OP2];
        else {
            marAux = pos_mem(mv,2,topB)
            if (marAux == -1){
                mv->error = 1;
            } else {
                mv->regs[MAR] = marAux;
                return mv->mem[mv->regs[MAR]];
            }
        }       
    }            
}

int isJump(int N, int Z, char ins, char topA){
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

void twoOpFetch (char ins, char *opA, char opB){ //reescribir cuando tengamos la parte que lee el archivo de código máquina.
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

void oneOpFetch (int *inm, int *ip, char *EDX,char ins, char *opB, int N, int Z, int error, int tam){ //*EDX va en caso de que sea sys despues debemos correjir por si el registro que creamos no coincide
    if (ins > 0x00 && ins < 0x08)   //si la instruccion es salto
    {
        if (*opB < tam) //me fijo si es un salto valido
        {
            if (is_jump(N, Z, ins, topA)) //verifico la condicion
                ip = *opB;   //salto
            else
                ip += 1;    //ignoro y paso al siguiente
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

char getIns(char aux){//consigo el tipo de instruccion
    return aux & 0b00011111;
}

char getTopA(char aux){ //consigo el tipo de operando A
    return (aux >> 4) & 0b00000011;
}

char getTopB(char aux){//consigo el tipo de operando A
    return (aux >> 6) & 0b00000011;
}

int main(){
    maquinaV mv; //inicializar en 0??
}