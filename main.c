#include <stdio.h>
#include <stdlib.h>
#include "operations.h"

//Constantes de registros, maquina virtual y tamaños definidos en operations.h

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

int leeOp(maquinaV *mv,int tOp){
    int valor = 0;
    char byteAct;

    for(int i = 0; i < tOp; i++){
        mv->regs[IP]++;
        byteAct = mv->mem[mv->regs[IP]];
        valor = (valor << 8) | byteAct;
    }

    return valor;
}

void ejecVmx(maquinaV *mv, int flagD){
    /*
    IMPORTANTE:
    ->Esta función al llamar al disassembler despues de "ejecutar", printea las lineas
    de codigo ASM desordenadas si encuentra un JMP, porque despues de leer el JMP se va
    a donde saltó, no sigue con la linea de abajo del JMP.
    
    ->La solución a esto es llamar a la función de disassembler en la función readFile.
    readFile lee byte por byte del .vmx, o sea, línea por línea del assembler.
    Habría nada más que hacer la lógica para que readFile llame al disassembler pasándole como
    parámetros los operandos y las instrucciones a la vez que se leen.
    */
    
    unsigned char byteAct, ins, tOpB, tOpA;
    unsigned int opA, opB;
    byteAct= mv->mem[mv->regs[IP]];
    while (mv->regs[IP] >= 0 && (mv->regs[IP] <= mv->regs[DS]-1)) { //ciclo principal de lectura
        //frena al leer todo el CS || encontrar el mnemónico STOP
        byteAct = mv->mem[mv->regs[IP]];
        ins = byteAct & 0x1F;
        mv->regs[OPC] = ins;
        tOpB = (byteAct >> 6) & 0x03;
        if (tOpB == 0) {
           // STOP(mv);
           //disassembler(*mv, tOpA, tOpB, mnem, registros);
        }
        else { //1 o 2 operandos
            tOpA = (byteAct >> 4) & 0x03;
            opA = 0;
            opB = 0;
            
            opB = leeOp(mv,tOpB);
            mv->regs[OP2] = opB; //lee y carga opB
            
            opA = leeOp(mv,tOpA); //lee y carga opA
            mv->regs[OP1] = opA;

            if (tOpB != 0 && tOpA != 0){
                //two_op_fetch(mv,tOpa,tOpB);
            }
            else{
                //one_op_fetch(mv);
            }
            
            /*
            if (flagD == 1)
                disassembler(*mv, tOpA, tOpB, mnem, registros); //lama a la funcion dissasembler si se introdujo la flag -d
            */
        }
        mv->regs[IP]++;
    }
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

/******FUNCIONES PARA BUSQUEDA******/


void twoOpFetch (maquinaV *mv, char topA, char topB){

    switch (mv -> regs[OPC]){                                               
        case 0x10:  MOV(mv, topA, topB);break;
        case 0x11:  ADD(mv, topA, topB);break;
        case 0x12:  SUB(mv, topA, topB);break;
        case 0x13:  MUL(mv, topA, topB);break;
        case 0x14:  DIV(mv, topA, topB);break;
        case 0x15:  CMP(mv, topA, topB);break;
        case 0x16:  SHL(mv, topA, topB);break;
        case 0x17:  SHR(mv, topA, topB);break;
        case 0x18:  SAR(mv, topA, topB);break;
        case 0x19:  AND(mv, topA, topB);break;
        case 0x1A:   OR(mv, topA, topB);break;
        case 0x1B:  XOR(mv, topA, topB);break;
        case 0x1C: SWAP(mv, topA, topB);break;
        case 0x1D:  LDL(mv, topA, topB);break;
        case 0x1E:  LDH(mv, topA, topB);break;
        case 0x1F:  RND(mv, topA, topB);break;
        default: mv -> error = 3;
    }
}

/*
int is_jump(maquinaV *mv){
    if (mv -> regs[OPC] > 0x00 && mv -> regs[OPC] < 0x08 && topA == 0)
    {
        switch (mv -> regs[OPC]){
            case 0x01: return 1;    //JMP 
            case 0x02: return mv->Z;    //JZ
            case 0x03: return !(mv->N) & !(mv->Z);  //JP
            case 0x04: return mv->N;    //JN
            case 0x05: return !(mv->Z);   //JNZ
            case 0x06: return (mv->N) || (mv->Z);
            case 0x07: return !(mv->N);   //JNN
        }
    }    
    return 0;
}
*/

void NZ (maquinaV *mv){ 
    mv -> N = mv -> regs[OP1] >> 15;
    mv -> Z = mv -> regs[OP1] == 0;
}

/*
void oneOpFetch (maquinaV *mv, char topB){ //*EDX va en caso de que sea sys despues debemos correjir por si el registro que creamos no coincide
    if (mv -> regs[OPC] > 0x00 && mv -> regs[OPC] < 0x08)   //si la instruccion es salto
    {
        if (mv -> regs[OPB] < mv -> tablaSeg[0][1]) //me fijo si es un salto valido
        {
            if (is_jump(mv)) //verifico la condicion
                mv -> regs[IP] = getValor(mv, topB);   //salto
            else
                mv -> regs[IP] += 1;    //ignoro y paso al siguiente
        } else 
            mv -> error = 1;  //si no es valido marco que hay un error en la ejecucion, esto puede servir para cortar el programa en caso de error    
    
    
    } else {
        if (mv -> regs[OPC] == 0x00)    //si la instruccion es sys
            SYS(mv, topB); 
        else
            if (mv -> regs[OPC] == 0x08)
                NOT(mv, topB);
            else
                mv -> error = 3;
    }       
}
*/

/******FUNCIONES PARA TRADUCIR EL ARCHIVO*****/

void disassembler(maquinaV mv, char topA, char topB){
    int offset, reg;

    printf("%s ", mnem[mv.regs[OPC]]);

    // Operando A
    if(topA != 0){
        if(topA == 1){
            printf("%s , ", registros[mv.regs[OP1]%32]);
        } else {
            reg = (mv.regs[OP1] >> 16) % 32;
            offset = mv.regs[OP1] & 0x00FF;
            if(offset >> 7 == 1) offset = (~offset+1)*-1;
            if(offset == 0) printf("[%s] , ", registros[reg]);
            else printf("[%s%+d] , ", registros[reg], offset);
        }
    }

    // Operando B
    if(topB != 0){
        if(topB == 1){
            printf("%s ", registros[mv.regs[OP2]%32]);
        } else if(topB == 2){
            printf("%d ", mv.regs[OP2]);
        } else {
            reg = (mv.regs[OP2] >> 16) % 32;
            offset = mv.regs[OP2] & 0x00FF;
            if(offset >> 7 == 1) offset = (~offset+1)*-1;
            if(offset == 0) printf("[%s] ", registros[reg]);
            else printf("[%s%+d] ", registros[reg], offset);
        }
    }

    printf("\n");
}

/*void writeCycle(maquinaV *mv){
    int topA, topB;
    mv->regs[IP] = 0;

    while(mv->regs[IP] < mv->tablaSeg[0][1]){
        topA = getTopA(mv);
        topB = getTopB(mv);

        mv->regs[OP1] = 0;
        mv->regs[OP2] = 0;
        mv->regs[OPC] = getIns(mv);

        for(int i=0;i<topB;i++){
            mv->regs[IP]++;
            mv->regs[OP2] = mv->mem[mv->regs[IP]] | (mv->regs[OP2]<<8);
        }
        for(int i=0;i<topA;i++){
            mv->regs[IP]++;
            mv->regs[OP1] = (mv->regs[OP1]<<8) | mv->mem[mv->regs[IP]];
        }

        disassembler(*mv, topA, topB);
        mv->regs[IP]++;
    }
}*/


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





