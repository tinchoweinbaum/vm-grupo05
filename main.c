#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operations.h"

//Constantes de registros, maquina virtual y tamaños definidos en operations.h

void leeVmx(FILE *arch, maquinaV *mv);
int leeOp(maquinaV *mv, int tOp);
void ejecVmx(maquinaV *mv);

void twoOpFetch(maquinaV *mv, char topA, char topB);
void oneOpFetch(maquinaV *mv, char topB);
void jump(maquinaV *mv, char topB);

void disassembler(maquinaV mv, char topA, char topB);
void writeCycle(maquinaV *mv);

char getIns(char aux);
char getTopA(char aux);
char getTopB(char aux);
void checkError(maquinaV mv);

const char* mnem[32] = {
    "SYS","JMP","JZ","JP","JN","JNZ","JNP","JNN",
    "NOT","09","0A","PUSH","POP","CALL","RET","STOP",
    "MOV","ADD","SUB","MUL","DIV","CMP","SHL","SHR",
    "SAR","AND","OR","XOR","SWAP","LDL","LDH","RND"
};

const char* registros[32] = {
    "LAR", "MAR", "MBR", "IP", "OPC", "OP1", "OP2", "SP",
    "BP", "-", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
    "AC", "CC", "-", "-", "-", "-", "-", "-",
    "-", "-", "CS", "DS", "ES", "SS", "KS", "PS"
};

void leeVmx(FILE *arch, maquinaV *mv);
int leeOp(maquinaV *mv, int tOp);
void ejecVmx(maquinaV *mv);

void twoOpFetch(maquinaV *mv, char topA, char topB);
void oneOpFetch(maquinaV *mv, char topB);
void jump(maquinaV *mv, char topB);

void disassembler(maquinaV mv, char topA, char topB);
void writeCycle(maquinaV *mv);

char getIns(char aux);
char getTopA(char aux);
char getTopB(char aux);
void checkError(maquinaV mv);

void iniciaVm(maquinaV *mv,int argc, char *argv[]);


void leeVmx(FILE *arch, maquinaV *mv) {
    //esta función se llama SÓLO después de verificar que existe el archivo.
    unsigned char byteAct;
    int tamCod = 0;

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

void leeVmi(maquinaV *mv, FILE *archVmi){ //Esta funcion se llama SOLAMENTE después de verificar que existe el .vmi especificado, o sea, llamar if archVmi != NULL
    
    unsigned char byteAct;
    unsigned int auxReg;
    unsigned short int tamMem;
    int cantSeg = -1;
    short  int auxShort;

    /*HEADER Y VERSIÓN*/

    for (int i = 0; i <= HEADER_SIZE_VMI - 3; i++){
        fread(&byteAct,1,sizeof(byteAct),archVmi);
        printf("%c",byteAct); //no se si hace falta printear
    }

    fread(&byteAct,1,sizeof(byteAct),archVmi); //Lee la version
    printf("\nVersion de .vmi: %c",byteAct);

    fread(&tamMem,1,sizeof(tamMem),archVmi); //Lee el tamaño de la memoria en un shortint (2 bytes)

    if(tamMem > MEM_SIZE){
        mv->error = 6; //Memoria insuficiente
        return;
    }

    /*VOLCADO DE REGISTROS*/

    for (int i = 0; i <= REG_SIZE; i ++){ //Lee los registros de la mv
        fread(&auxReg,1,sizeof(auxReg),archVmi);
        mv->regs[i] = byteAct;
    }

    /*TABLA DE SEGMENTOS*/

    for (int i = 0; i < 8; i++){ //lee 8 bloques de 4 bytes (tabla de segmentos)
        fread(&auxShort,1,sizeof(auxShort),archVmi);
        mv->tablaSeg[i][0] = auxShort;
        fread(&auxShort,1,sizeof(auxShort),archVmi);
        mv->tablaSeg[i][1] = auxShort;
    }

    /*CHECKEO DE SEGMENTOS*/

    if(mv->regs[PS] != -1){
        cantSeg++;
        mv->vecPosSeg[posPS] = cantSeg;
        mv->regs[PS] = 0; 
    }

    if(mv->regs[KS] != -1){
        cantSeg++;
        mv->vecPosSeg[posKS] = cantSeg;
        mv->regs[KS] = mv->tablaSeg[cantSeg][0];
    }

    if(mv->regs[CS] != -1){
        cantSeg++;
        mv->vecPosSeg[posCS] = cantSeg;
        mv->regs[CS] = mv->tablaSeg[cantSeg][0];
    }

    if(mv->regs[DS] != -1){
        cantSeg++;
        mv->vecPosSeg[posDS] = cantSeg;
        mv->regs[DS] = mv->tablaSeg[cantSeg][0];
    }

    if(mv->regs[ES] != -1){
        cantSeg++;
        mv->vecPosSeg[posES] = cantSeg;
        mv->regs[ES] = mv->tablaSeg[cantSeg][0];
    }

    if(mv->regs[SS] != -1){
        cantSeg++;
        mv->vecPosSeg[posSS] = cantSeg;
        mv->regs[SS] = mv->tablaSeg[cantSeg][0];
    }

    /*VOLCADO DE MEMORIA*/

    for (int i = 0; i <= tamMem; i++){
        fread(&byteAct,1,sizeof(byteAct),archVmi);
        mv->mem[i] = byteAct;
    }

    fclose(archVmi);

}

int leeOp(maquinaV *mv,int tOp){
    int valor = 0;
    unsigned char byteAct;

    for(int i = 0; i < tOp; i++){
        if(mv->regs[IP] >= mv->regs[DS]){
            mv->error = 1;
            break;
        }
        mv->regs[IP]++;
        byteAct = mv->mem[mv->regs[IP]];
        valor = (valor << 8) | byteAct;
    }

    if (tOp == 2 && (valor & 0x8000))
        valor |= 0xFFFF0000;
    return valor;
}

void ejecVmx(maquinaV *mv){

    unsigned char byteAct;
    char ins, tOpB, tOpA;
    int opA, opB, auxIp;


    mv->regs[IP] = 0;
    while (mv->regs[IP] >= 0 && (mv->regs[IP] <= mv->regs[DS]-1) && mv->error == 0) { //ciclo principal de lectura
        //frena al leer todo el CS || encontrar el mnemónico STOP
        byteAct = mv->mem[mv->regs[IP]];
        ins = byteAct & 0x1F;
        mv->regs[OPC] = ins;
        tOpB = (byteAct >> 6) & 0x03;

        if (tOpB == 0){
            switch (ins)
            {
                case 0x0E: RET(mv); break;
                case 0x0F: STOP(mv); break;
                default: mv -> error = 3; break;
            }
        
        }else { //1 o 2 operandos
        
            tOpA = (byteAct >> 4) & 0x03;
            opA = 0;
            opB = 0;
            
            opB = leeOp(mv,tOpB);
            if(mv->error == 1)
                break;
            mv->regs[OP2] = opB; //lee y carga opB
            
            opA = leeOp(mv,tOpA); //lee y carga opA
            if(mv->error == 1)
                break;
            mv->regs[OP1] = opA;

            auxIp =mv->regs[IP];

            if (tOpB != 0 && tOpA != 0)
                twoOpFetch(mv,tOpA,tOpB);
            else
                oneOpFetch(mv,tOpB);
            if(mv->error != 0)
                break;
                
            if(!(mv->regs[OPC] > 0x00 && mv->regs[OPC] < 0x08) || auxIp == mv->regs[IP])
                mv->regs[IP]++;
        }
    }
}

/******FUNCIONES PARA BUSQUEDA******/


void twoOpFetch (maquinaV *mv, char topA, char topB){
    //printf(" Llamado de dos operandos: %s\n",mnem[mv->regs[OPC]]);
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

void jump(maquinaV *mv,char topB){
int val, offset, base, tope;
    getValor(mv,OP2,&val,topB);
    base = mv -> regs[CS];
    tope = mv -> tablaSeg[mv -> vecPosSeg[posCS]][1];
    offset = val & 0xFFFF;
    if (mv -> regs[OPC] > 0x00 && mv -> regs[OPC] < 0x08)
    {
        if (topB == 2 && (val < 0 || val > mv -> tablaSeg[mv -> vecPosSeg[posDS]][0])){
            mv -> error = 1;
            return;
        }
        if (topB == 1 && (val < 0 || val > REG_SIZE)){
            mv -> error = 3;
            return;
        }
        if (topB == 3 && val + offset >= base && val + offset <= tope)
        {
            mv -> error = 1;
            return;
        }
                
        if (mv -> error == 0)
        {
            switch (mv -> regs[OPC]){
                case 0x01: JMP(mv,val); break;
                case 0x02: JZ(mv,val); break;
                case 0x03: JP(mv,val); break;
                case 0x04: JN(mv,val); break;
                case 0x05: JNZ(mv,val); break; 
                case 0x06: JNP(mv,val); break;
                case 0x07: JNN(mv,val); break;
            }
        }
    }    
}

void oneOpFetch (maquinaV *mv, char topB){
    int dirsalto;
    if (mv -> regs[OPC] > 0x00 && mv -> regs[OPC]<0x08){ //si es salto
        getValor(mv,OP2,&dirsalto,topB);
        if (dirsalto > 0 && dirsalto < mv->tablaSeg[mv -> vecPosSeg[posCS]][1])
            jump(mv,topB);
        else{
            mv -> error = 1;
            return;
        }

    } else { //si no es salto

        switch (mv -> regs[OPC]){
            case 0x00: SYS(mv); break;
            case 0x08: NOT(mv, topB); break;
            case 0x0B: PUSH(mv, topB);break;
            case 0x0C: POP(mv, topB);break;
            case 0x0D: CALL(mv);break;
            default: mv -> error = 3; break;
        }
    }
}

/******FUNCIONES PARA TRADUCIR EL ARCHIVO*****/

void disassembler(maquinaV mv, char topA, char topB){
    int reg;
    short inm, offset;

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
            inm = mv.regs[OP2];
            printf("%d ", inm);
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

void writeCycle(maquinaV *mv) {
    int topA, topB;
    mv->regs[IP] = 0;

    while (mv->regs[IP] < mv->tablaSeg[mv -> vecPosSeg[posCS]][1]) {
        char byte = mv->mem[mv->regs[IP]];
        topA = (byte >> 4) & 0x03;
        topB = (byte >> 6) & 0x03;
        mv->regs[OP1] = 0;
        mv->regs[OP2] = 0;
        mv->regs[OPC] = byte & 0x1F;

        for (int i = 0; i < topB; i++) {
            mv->regs[IP]++;
            mv->regs[OP2] = (mv->regs[OP2] << 8) | mv->mem[mv->regs[IP]];
        }
        for (int i = 0; i < topA; i++) {
            mv->regs[IP]++;
            mv->regs[OP1] = (mv->regs[OP1] << 8) | mv->mem[mv->regs[IP]];
        }

        disassembler(*mv, topA, topB);
        mv->regs[IP]++;
    }
    printf("\n\n");
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

void checkError(maquinaV mv){
    switch(mv.error){
        case 0: break;
        case 1: printf("\nError: Segmentation fault.\n");break;
        case 2: printf("\nError: División por 0.\n");break;
        case 3: printf("\nError: Instrucción inválida.\n");break;
        default: printf("\nError desconocido.\n");
    }
}

unsigned int tamaniomemoria(char *Mem){

    unsigned int len,num=0,i=2;

    len = strlen(Mem);
    num += Mem[i] - '0'; // primer numero
    for (i=3; i < len; i++){
        num *=10;
        num += Mem[i] - '0';
    }
    return num * 1024; // KiB
}

void iniciaVm(maquinaV *mv,int argc, char *argv[]){
    
    if(argc <= 1){
        printf("\nNo se especifico un archivo.");
        return;
    }

    if(strcmp(argv[1] + strlen(argv[1] - 4),".vmi") == 0){ //archivo.vmi, no hay .vmx = nada más cargar imagen
        FILE *archVmi = fopen(argv[1],"rb");
        if(!archVmi){
            printf("\nNo existe el .vmi especificado.");
            return;
        }

        leeVmi(mv,archVmi); //si solo se especifica .vmi, se ignoran los parametros -p y la memoria m=M

        if(argc >= 2 && strcmp(argv[2],"-d") == 0) //Puede venir un parámetro más en el medio???? puedo tener que argv[2] sea el m=M por mas de que el tamaño de la memoria venga en el .vmi??!?!
            writeCycle(mv);
        
        //Al hacer return y no seguir leyendo efectivamente se ignoran los parámetros.

        return;
    }

    if(strcmp(argv[1] + strlen(argv[1] - 4),".vmx") == 0){ //Si el 1er archivo es .vmx
        
        //Ejecución del .vmx, carga de parámetros, definicion de segmentos, entry point, parámetros, etc.
        
        if(strcmp(argv[2] + strlen(argv[2] - 4),".vmi") == 0){ //Si el segundo archivo es .vmi se usa para escribirlo en el SYS F
            //Archivo como parametro de salida??? o lo puede buscar la funcion de creaVmi en el directorio??
        }
    }



}

int main(int argc, char *argv[]) {
    maquinaV mv;
    mv.error = 0;

    memset(mv.mem, 0 ,MEM_SIZE);

    unsigned int M=0, MV;
    int i=0;     
    char vmi,vmx,flagD,parametros,ArchVMX[50],ArchVMI[50];


    /*
    MV=2;   // De base tomo que la maquina es la 2da parte
    vmi = vmx = parametros = 'N';

    if(argc <= 1){ // agregar error en la extension
        printf("No se especificó un archivo.\n");
        return 1;
    } //muchachos no podemos llenar el main de cosas hagamos una funcion como la gente xd
    else
        if (strcmp(argv[1] + strlen(argv[1]) - 4, ".vmi") == 0){  //----------------------  .vmi
            if(argc == 4){
                M = tamaniomemoria(argv[2]);
                if (strcmp(argv[3],"-d") == 0)
                    flagD='S';
            }
           else
                if (argc == 3){
                    if (strncmp(argv[2],"m=",2)==0)
                        M = tamaniomemoria(argv[2]);
                    else
                        if (strcmp(argv[2],"-d") == 0)
                            flagD='S';
                }
            vmi='S';strcpy(ArchVMI,argv[1]);
        }
        else
            if(argc == 2 && strcmp(argv[1] + strlen(argv[1]) - 4, ".vmx") == 0 ){
                MV = 1;
                strcpy(ArchVMX,argv[1]);
            }       
            else
                if (argc == 3 && strcmp(argv[1] + strlen(argv[1]) - 4, ".vmx") == 0 && strcmp(argv[2],"-d") == 0){
                    MV = 1;
                    strcpy(ArchVMX,argv[1]);
                    flagD = 'S';
                }
                else
                    while (i < argc){
                        if (strcmp(argv[i] + strlen(argv[i]) - 4, ".vmx") == 0){
                            strcpy(ArchVMX,argv[i]);                  vmx='S';  }

                        if (strcmp(argv[i] + strlen(argv[i]) - 4, ".vmi") == 0 ){
                            strcpy(ArchVMI,argv[i]);                            vmi='S';}

                        if (strncmp(argv[i],"m=",2) == 0)
                            M = tamaniomemoria(argv[i]);

                        if (strcmp(argv[i],"-d") == 0)
                            flagD = 'S';

                        if(strcmp(argv[i],"-p") == 0){
                            for( int h=i+1 ; h < argc; h++ )
                                printf("%s \t",argv[h]);
                            printf("\n");            
                            parametros='S';}
                        i++;  

                    }


if (MV == 1){
    FILE *arch = fopen(ArchVMX,"rb");
    if(arch != NULL){
        leeVmx(arch, &mv);
        if (flagD == 'S')
            writeCycle(&mv);
        printf("\n");
        ejecVmx(&mv);
        checkError(mv);
    }
    else
        printf("No existe el archivo.");                            
    }
    else
    if (MV == 2){
        printf("MAQUINA VIRTUAL PARTE 2 \n");
        if (flagD == 'S')
            printf("-d \n");

        if (M == 0) // memoria por defecto
            printf("%d \n",MEM_SIZE);
        else
            printf("%d \n",M);

        if (parametros == 'S')
            printf("Hay paramatros\n");
        else
            printf("NO hay parametros\n");

        if (vmi=='S')
            printf("%s \n",ArchVMI);
        else
            printf("NO hay .vmi \n");

        if (vmx  == 'S')
            printf("%s \n",ArchVMX);
        else
            printf("No hay VMX \n");

        //printf("Aca van los llamados a las funciones de la 2da parte de la maquina virtual");
    }
    
    REESCRIBIR TODO ESTO EN UNA FUNCION MAS LEGIBLE

    */    
    return 0;        
    }
