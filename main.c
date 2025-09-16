#include <stdio.h>
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

const char* mnem[32] = {
    "SYS","JMP","JZ","JP","JN","JNZ","JNP","JNN",
    "NOT","09","0A","0B","0C","0D","0E","STOP",
    "MOV","ADD","SUB","MUL","DIV","CMP","SHL","SHR",
    "SAR","AND","OR","XOR","SWAP","LDL","LDH","RND"
};

const char* registros[32] = {
    "LAR", "MAR", "MBR", "IP", "OPC", "OP1", "OP2", "-",
    "-", "-", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
    "AC", "CC", "-", "-", "-", "-", "-", "-",
    "-", "-", "CS", "DS", "-", "-", "-", "-"
};

typedef struct maquinaV{
    unsigned char mem[MEM_SIZE]; //vector de memoria
    unsigned char regs[REG_SIZE]; //vector de registros
    int tablaSeg[1][1]; // tabla de segmentos: matriz de 2x2
} maquinaV;

void readFile(FILE *arch, maquinaV *mv, int *error) {
    //esta función se llama SÓLO después de verificar que existe el archivo.
    unsigned char byteAct;
    int tamCod = 0;
    char tOpA, tOpB, ins = 0;
    int opA, opB;

    for(int i = 0; i <= HEADER_SIZE-3; i++) { //lee el header del archivo, excluyendo el tamaño del código
        fread(&byteAct, 1, sizeof(byteAct), arch);
        printf("%c", byteAct); //printea VMX25
    }

    fread(&byteAct, 1, sizeof(byteAct), arch); //lee version
    printf("\nVersion: %x\n",byteAct);

    for(int i = HEADER_SIZE-2; i < HEADER_SIZE; i++) { //lee el tamaño del codigo
        fread(&byteAct, 1, sizeof(byteAct), arch);
        tamCod = (tamCod << 8) | byteAct;
    }
    if(tamCod > MEM_SIZE) //asignar un código de error
        printf("\nEl código supera el tamaño máximo.\n"); 
    else {
        mv->tablaSeg[0][0] = 0;
        mv->tablaSeg[0][1] = tamCod; //define segmentos de memoria
        mv->tablaSeg[1][0] = tamCod;
        mv->tablaSeg[1][1] = MEM_SIZE - tamCod;
        mv->regs[CS] = 0; //inicializa CS
        mv->regs[DS] = tamCod; //inicializa DS
        mv->regs[IP] = 0; //inicializa IP
        for (int i=0; i < tamCod; i++){ //ciclo principal de lectura
            fread(&byteAct,1,sizeof(byteAct),arch);
            mv->mem[i] = byteAct;
        }
    }
    fclose(arch);
}

void ejecVmx(maquinaV *mv, int flagD){
    unsigned char byteAct, ins, tOpB, tOpA;
    unsigned int opA, opB;
    byteAct= mv->mem[mv->regs[IP]];
    while (mv->regs[IP] >= 0 && (mv->regs[IP] <= mv->regs[DS])) { //ciclo principal de lectura
        //frena al leer todo el CS || encontrar el mnemónico STOP
        byteAct = mv->mem[mv->regs[IP]];
        ins = byteAct & 0x1F;
        mv->regs[OPC] = ins;
        tOpB = (byteAct >> 6) & 0x03;
        if (tOpB == 0) {
           // STOP(mv);
           disassembler(*mv, tOpA, tOpB, mnem, registros);
        }
        else { //1 o 2 operandos
            tOpA = (byteAct >> 4) & 0x03;
            opA = 0;
            opB = 0;
            
            for (int i = 0; i < tOpB; i++) { //lee el valor del operando B
                mv->regs[IP]++;
                byteAct = mv->mem[mv->regs[IP]];
                opB = (opB << 8) | byteAct;
            }
            mv->regs[OP2] = opB;
            
            for (int i = 0; i < tOpA; i++) { //lee el valor del operando A
                mv->regs[IP]++;
                byteAct = mv->mem[mv->regs[IP]];
                opA = (opA << 8) | byteAct;
            }
            mv->regs[OP1] = opA;

            if (tOpB != 0 && tOpA != 0){
                //two_op_fetch(mv);
            }
            else{
                //one_op_fetch(mv);
            }
                
            if (flagD == 1)
                disassembler(*mv, tOpA, tOpB, mnem, registros); //lama a la funcion dissasembler si se introdujo la flag -d
        }
        mv->regs[IP]++;
    }
}

void disassembler(maquinaV mv, char topA, char topB, const char* mnem[], const char* registros[]) {
    int offset, reg;

    printf("%s ", mnem[mv.regs[OPC]]);

    // Operando A
    if (topA != 0) {
        if (topA == 1) {
            printf("%s , ", registros[mv.regs[OP1] % 32]); // asegurar rango válido
        } else {
            reg = (mv.regs[OP1] >> 16) % 32;
            offset = mv.regs[OP1] & 0x00FF;
            if (offset >> 7 == 1) // 8 bits signed
                offset = (~offset + 1) * -1;

            if (offset == 0)
                printf("[%s] , ", registros[reg]);
            else
                printf("[%s%+d] , ", registros[reg], offset);
        }
    }

    // Operando B
    if (topB != 0){
        if (topB == 1) {
            printf("%s ", registros[mv.regs[OP2] % 32]);
        } else if (topB == 2) {
            printf("%d ", mv.regs[OP2]);
        } else {
            reg = (mv.regs[OP2] >> 16) % 32;
            offset = mv.regs[OP2] & 0x00FF;
            if (offset >> 7 == 1)
                offset = (~offset + 1) * -1;

            if (offset == 0)
                printf("[%s] ", registros[reg]);
            else
                printf("[%s%+d] ", registros[reg], offset);
        }
    }
    


    printf("\n");
}

void setReg(maquinaV *mv,int index_reg, char val){
    mv->regs[index_reg]=val;
}

char getReg(maquinaV mv, int index_reg){
    return mv.regs[index_reg];
}

/*int get_op(maquinaV *MV, char topB){
    if (topB == 0b01){
        if (opB >= 0 && opB < 32)
        {
            return MV.reg[opB];
        } else {
            *MV.error = 1 // segmentation fault
        }
    }else {
        if (topB == 0b10)
            return opB;
        else {
            if (opB >= MV.segmentTable[1] && opB < MAX)
            {
                return MV.mem[opB];
            } else {
                *MV.error = 1 // segmentation fault
            }
        }       
    }            
}*/

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

/*void two_op_fetch (char ins, char *opA, char opB, char tOpA, char tOpB){ //reescribir cuando tengamos la parte que lee el archivo de código máquina.
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
}*/

/*void one_op_fetch (int *inm, int *ip, char *EDX,char ins, char *opB, int N, int Z, int error, int tam){ //*EDX va en caso de que sea sys despues debemos correjir por si el registro que creamos no coincide
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
    
}*/

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
    maquinaV mv;
    FILE *arch = fopen("sample.vmx","rb");
    if(arch != NULL){
        int error = 0;
        readFile(arch, &mv, &error);
        ejecVmx(&mv,1);
    }
    else{
        printf("No existe el archivo.");
    }
    return 0;
}