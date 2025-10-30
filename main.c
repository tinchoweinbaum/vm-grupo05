#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operations.h"

//Constantes de registros, maquina virtual y tamaños definidos en operations.h

extern int posPS;
extern int posKS;
extern int posCS;
extern int posDS;
extern int posES;
extern int posSS;


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

void iniciaPila(maquinaV *mv, int argC, int argV);


int esCodeSegment(maquinaV *mv){
    int seg, offset;
    seg = mv -> regs[IP] >> 16;
    offset = mv -> regs[IP] & 0xFFFF;

    if (seg == posCS)
        return offset < mv -> tablaSeg[posCS][1];
    else {
        return 0;
    }
}

void leeVmx_MV1(FILE *arch, maquinaV *mv) {

    unsigned char byteAct;
    int tamCod = 0,i;
    fseek(arch,0,0);

    //////////  HEADER  //////////

    for(int i = 0; i <= HEADER_SIZE_V1 - 3; i++) { 
        fread(&byteAct, 1, sizeof(byteAct), arch);      // VMX25
        printf("%c", byteAct); 
    }

    fread(&byteAct, 1, sizeof(byteAct), arch);   // Version
    printf("\nVersion: %x\n",byteAct);

    for(int i = HEADER_SIZE_V1 - 2; i < HEADER_SIZE_V1; i++) {      // Tamaño del codigo
        fread(&byteAct, 1, sizeof(byteAct), arch);
        tamCod = (tamCod << 8) | byteAct;
    }
    
    //////////  TABLA DE SEGMENTOS Y PUNTEROS  //////////

    if(tamCod > MEM_SIZE){
        mv->error = 6;
    }
    else {
        mv->tablaSeg[0][0] = 0;
        mv->tablaSeg[0][1] = tamCod;
        posCS = 0;
        mv->tablaSeg[1][0] = tamCod;
        mv->tablaSeg[1][1] = MEM_SIZE - tamCod;
        posDS = 1;

        mv->regs[CS] = 0; 
        mv->regs[DS] = tamCod; 
        mv->regs[IP] = 0;

        for (i=0; i < tamCod; i++){              // Lectura
            fread(&byteAct,1,sizeof(byteAct),arch);
            mv->mem[i] = byteAct;
        }
    }
    fclose(arch);
} 

void tabla_segmentos (maquinaV *mv, int VectorSegmentos[], unsigned int TopeVecSegmentos){
    unsigned int i, postablaseg = 0;

    for(i = 0; i < TopeVecSegmentos; i++){
        /*SI EL SEGMENTO EXISTE*/
        if (VectorSegmentos[i] != -1){
            /*SI ES EL PRIMERO*/
            if (postablaseg == 0){
                
                mv->tablaSeg[postablaseg][0] = 0;     
                mv->tablaSeg[postablaseg][1] = VectorSegmentos[i];    
            
            } else {
                
                mv->tablaSeg[postablaseg][0] = mv->tablaSeg[postablaseg-1][1] + mv->tablaSeg[postablaseg-1][0];
                mv->tablaSeg[postablaseg][1] = VectorSegmentos[i];
            
            }

            switch (i){ // Establezco punteros y posiciones de los segmentos de la tabla en las variables
                case 0: {posPS = postablaseg; mv->regs[PS] = mv->tablaSeg[postablaseg][0] ; break;}
                case 1: {posKS = postablaseg; mv->regs[KS] = mv->tablaSeg[postablaseg][0] ; break;} 
                case 2: {posCS = postablaseg; mv->regs[CS] = mv->tablaSeg[postablaseg][0] ; break;} 
                case 3: {posDS = postablaseg; mv->regs[DS] = mv->tablaSeg[postablaseg][0] ; break;} 
                case 4: {posES = postablaseg; mv->regs[ES] = mv->tablaSeg[postablaseg][0] ; break;} 
                case 5: {posSS = postablaseg; mv->regs[SS] = mv->tablaSeg[postablaseg][0] ; break;} 
            }

            postablaseg ++;

        }
    }

    //Relleno la tabla con 0 en los lugares restantes:
    for(int i = postablaseg; i < 8; i++)
        for(int j = 0; j <= 1; j++)
            mv->tablaSeg[i][j] = 0;
}

int swap_endian(int x) {
    return ((x>>24)&0xFF) |        // MSB → LSB
           ((x>>8)&0xFF00) |
           ((x<<8)&0xFF0000) |
           ((x<<24)&0xFF000000);
}


void leeVmx_MV2(FILE *arch, maquinaV *mv, unsigned int M, char Parametros[][LEN_PARAM], int posPara, unsigned short int *entrypoint,  int VectorSegmentos[], unsigned int *TopeVecSegmentos) {
    
    unsigned char byteAct;
    unsigned int j, paramlen ,memor = 0 ,VecArgu[CANT_PARAM];
    unsigned short int tamseg;
    int i, posArgu = 0;
    fseek(arch,0,0); //me paro en el inicio del archivo.

    for (i = CS; i <= PS; i++)
    {
        mv -> regs[i] = -1;
    }
    

    //////////  MEMORIA  //////////   

    if (M != 0) 
        mv->tamMem = M;
    else
        mv->tamMem = MEM_SIZE;

    printf("Memoria disponible: %d bytes.\n",mv->tamMem);


    //////////  HEADER  //////////

    for(int i = 0; i <= 4; i++) {                       // VMX25
        fread(&byteAct, 1, sizeof(byteAct), arch);
        printf("%c", byteAct); 
    }

    fread(&byteAct, 1, sizeof(byteAct), arch);          // VERSION
    printf("\nVersion: %x\n",byteAct);

   
    for (i=0; i <= CANT_SEG; i++) // Inicia en -1 el tamaño de cada segmento
        VectorSegmentos[i] = -1;

    for(i = 6; i <= HEADER_SIZE_V2-8; i++) {         // Tamaños desde Code Segment hasta Stack Segment

        fread(&tamseg, 1, sizeof(tamseg), arch);
        tamseg = (tamseg >> 8) | (tamseg << 8);

        if (tamseg > 0){
            VectorSegmentos[*TopeVecSegmentos]= tamseg;   
            memor += tamseg;}

        (*TopeVecSegmentos)++;
    }

    fread(&tamseg, 1, sizeof(tamseg), arch);    // Constant Segment
    tamseg = (tamseg >> 8) | (tamseg << 8); 

    if (tamseg > 0){
       VectorSegmentos[1]= tamseg;   
        memor += tamseg;
    }
                             
    fread(entrypoint, 1, sizeof(tamseg), arch);      //Entry ponint
    *entrypoint = (*entrypoint << 8) | ((*entrypoint >> 8) & 0xff); 

    //////////  CARGA MV  //////////
    

    if (memor > mv->tamMem)    //Segmentos mayor que memoria disponible 
        mv->error = 6; 
    else{
        memor = 0;
        tamseg = 0;
        if (posPara != 0){     //  Si existe param segment.
        
            for (i =0; i <= posPara; i++)           // Tamaño
                tamseg += strlen(Parametros[i]) + 5; //  5 = 1 (el 0 que separa cada palabra) + 4 (puntero a la palabra)
               
            VectorSegmentos[0] = tamseg;
 
            printf("\n %s", Parametros[0]);

            for (i=0; i<posPara; i++){
                VecArgu[posArgu++]=memor;
                paramlen = strlen(Parametros[i]);
                for (j = 0; j < paramlen; j++)
                    mv->mem[memor++]= Parametros[i][j];
                mv->mem[memor++] = 0;
            }
            
            int argV = memor + 1;
            int argC = posPara;
            printf("\npuntero a argv: %04x",argV);
            printf("\ncantidad parametros: %04x",argC);

            iniciaPila(mv, argC, argV);

            for (i=0; i<posArgu; i++){
                mv->mem[memor++] = (VecArgu[i] >> 24) & 0xFF;
                mv->mem[memor++] = (VecArgu[i] >> 16) & 0xFF;
                mv->mem[memor++] = (VecArgu[i] >> 8) & 0xFF;
                mv->mem[memor++] = VecArgu[i] & 0xFF;
            }



        }

        if (VectorSegmentos[1] != -1)               // Si hay constant segment empiezo a escribir el codigo desde el final del const segment
            memor = VectorSegmentos[1]  + memor;
        else
            memor = memor;   // sino lo escribo desde la ultima posicion en memoria

       // Carga el Code Segment y Const Segment   //    
        
        for (i = 0; i < VectorSegmentos[2] ; i++){           
            fread(&byteAct,1,sizeof(byteAct),arch);
            mv->mem[memor] = byteAct;
            memor++;
        }

        for (i = 0; i < VectorSegmentos[1] ; i++){
            fread(&byteAct,1,sizeof(byteAct),arch);
            mv->mem[memor] = byteAct;
            memor++;
        }

    }

    fclose(arch); 

}

void leeVmi(maquinaV *mv, FILE *archVmi){ 

    unsigned char byteAct;
    unsigned int cantSeg = 0, auxInt;
    short int auxShort;

    //HEADER Y VERSIÓN//

    for (int i = 0; i <= HEADER_SIZE_VMI - 3; i++){     // VMI25
        fread(&byteAct,1,sizeof(byteAct),archVmi);
        printf("%c",byteAct); 
    }

    fread(&byteAct,1,sizeof(byteAct),archVmi);   // Version
    printf("\nVersion de .vmi: %d \n",byteAct);

    fread(&(mv->tamMem),1,sizeof(mv->tamMem),archVmi); //Lee tamMem
    printf("\nMemoria: %d KiB",mv->tamMem);

        //LEE MAL TAMAÑO DE MEMORIA.

        //VOLCADO DE REGISTROS//

    for(int i = 0; i < REG_SIZE; i++){
        fread(&auxInt,1,sizeof(auxInt),archVmi);
        mv->regs[i] = auxInt;
    }
    printf("\nREG PS: %d", mv->regs[PS]);
    printf("\n");

    //TABLA DE SEGMENTOS//

    if (mv->regs[PS] != -1) { // Cantidad de segmentos cargados.
        posPS = cantSeg;
        cantSeg++;
    }

    if (mv->regs[KS] != -1) {
        posKS = cantSeg;
        cantSeg++;
    }

    //printf("\nEl KS leido del .vmi es: %d", mv->regs[KS]);

    if (mv->regs[CS] != -1) {
        posCS = cantSeg;
        cantSeg++;
    }
    printf("\nposPS vale: %d",posPS);

    printf("\nEl CS leido del .vmi es: %d", mv->regs[CS]);

    printf("\nposCS vale: %d",posCS);

    if (mv->regs[DS] != -1) {
        posDS = cantSeg;
        cantSeg++;
    }

    if (mv->regs[ES] != -1) {
        posES = cantSeg;
        cantSeg++;
    }

    if (mv->regs[SS] != -1) {
        posSS = cantSeg;
        cantSeg++;
    }


        //LECTURA DE LA TABLA//

    for (int i = 0; i < 8; i++){ //lee 8 bloques de 4 bytes (tabla de segmentos)
        fread(&auxShort,1,sizeof(auxShort),archVmi);
        mv->tablaSeg[i][0] = auxShort;
        fread(&auxShort,1,sizeof(auxShort),archVmi);
        mv->tablaSeg[i][1] = auxShort;
        //printf("\nPOSICION 0 ACTUAL DE LA TABLA: %d",mv->tablaSeg[i][0]);
        //printf("\nPOSICION 1 ACTUAL DE LA TABLA: %d",mv->tablaSeg[i][1]);
    }

        //VOLCADO DE MEMORIA//

        //mv->tamMem = swap_endian(mv->tamMem);

    printf("\ntamanio de memoria leido: %d %08X",mv->tamMem,mv->tamMem);

    for (unsigned int i = 0; i < mv->tamMem; i++){
        fread(&byteAct,1,sizeof(byteAct),archVmi);
        mv->mem[i] = byteAct;
        printf("%02X ",mv->mem[i]);
    }

    fclose(archVmi);

    printf("Tabla de segmentos (leeVmi): \n");
    for(int i = 0; i < 8; i++){
        for (int j = 0; j <= 1; j++){
            printf("%d ",mv->tablaSeg[i][j]);
        }
        printf("\n");
    }

}

int leeOp(maquinaV *mv,int tOp){
    int valor = 0;
    unsigned char byteAct;

    for(int i = 0; i < tOp; i++){
       if(!esCodeSegment(mv)){ //si me caigo del CS
            mv->error = 1;
            break;
        }
        mv->regs[IP]++; //Está bien avanzar el IP antes de leer porque leeOp se llama con el IP apuntando al byte de instrucción.
        byteAct = mv->mem[mv->regs[IP]];
        valor = (valor << 8) | byteAct;
    }

    if (tOp == 2 && (valor & 0x8000)) //sign extend
        valor |= 0xFFFF0000;
    return valor;
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
    tope = mv -> tablaSeg[posCS][1];
    offset = val & 0xFFFF;
    if (mv -> regs[OPC] > 0x00 && mv -> regs[OPC] < 0x08)
    {
        if (topB == 2 && (val < 0 || val > mv -> tablaSeg[posDS][0])){
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

    //printf(" Llamado de UN operandos: %s\n",mnem[mv->regs[OPC]]);
    if (mv -> regs[OPC] > 0x00 && mv -> regs[OPC]<0x08){ //si es salto
        getValor(mv,OP2,&dirsalto,topB);
        if (dirsalto > 0 && dirsalto < mv->tablaSeg[posCS][1])
            jump(mv,topB);
        else{
            mv -> error = 1;
            return;
        }

    } else { //si no es salto

        switch (mv -> regs[OPC]){
            printf("entre al sys");
            case 0x00: menuSYS(mv); break;
            case 0x08: NOT(mv, topB); break;
            case 0x0B: PUSH(mv, topB);break;
            case 0x0C: POP(mv, topB);break;
            case 0x0D: CALL(mv);break;                                                         
            default: mv -> error = 3; break;
        }
    }
}

void ejecVmx(maquinaV *mv) {
    unsigned char byteAct;
    char ins, tOpB, tOpA;
    int opA, opB, auxIp;

    while (mv -> error == 0 && mv -> regs[IP] != -1 && esCodeSegment(mv)){
        //ASIGNACION VARIABLES
        byteAct = mv -> mem[mv -> regs[IP]];

        ins = byteAct & 0x1F;
        tOpA = (byteAct >> 4) & 0x3;
        tOpB = (byteAct >> 6) & 0x3;

        printf("\nbyte de instruccion: %02x\t", ins);
        mv -> regs[OPC] = ins;

        printf("OPERACION: %s\n", mnem[(unsigned char)ins]);
        //LA FUNCION NO TIENE OPERANDOS
        if (tOpB == 0){
            switch (mv -> regs[OPC]) {
                case 0xE: RET(mv); break;
                case 0xF: STOP(mv); break;
                default: mv -> error = 3; break;
            }
        } else {

            /* CARGO OPERANDO B */
            opB = leeOp(mv, tOpB);
            if (mv->error != 0) break;
            mv->regs[OP2] = opB;

            /* CARGO OPERANDO A */
            opA = leeOp(mv, tOpA);
            if (mv->error != 0) break;
            mv->regs[OP1] = opA;

            auxIp = mv->regs[IP];

            /* EJECUTO OPERACIONES */
            if (tOpA != 0 && tOpB != 0) {
                twoOpFetch(mv, tOpA, tOpB);
            } else {
                oneOpFetch(mv, tOpB);
            }

            if (mv->error != 0) break;

            /* AVANZO IP MANUALMENTE SI NO SALTÉ */
            if (mv->regs[IP] == auxIp) {
                mv->regs[IP]++;
            }
        }

    }
    
}





/******FUNCIONES PARA TRADUCIR EL ARCHIVO*****/

void disassembler(maquinaV mv, char topA, char topB){
    int reg, tamMem;
    short inm, offset;
    printf("%s ", mnem[mv.regs[OPC]]);

    // Operando A
    if(topA != 0){
        if(topA == 1){
            printf("%s , ", registros[mv.regs[OP1]%32]);
        } else {
            reg = (mv.regs[OP1] >> 16) % 32;
            offset = mv.regs[OP1] & 0x00FF;

            tamMem = mv.regs[OP1] >> 22;

            switch (tamMem){
                case 0b00: printf("l"); break;
                case 0b10: printf("w"); break;
                case 0b11: printf("b"); break;
                default: break;
            }

            if(offset >> 7 == 1) offset = (~offset+1)*-1;
            if(offset == 0) 
                printf("[%s] , ", registros[reg]);
            else 
                printf("[%s%+d] , ", registros[reg], offset);
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
            
            tamMem = mv.regs[OP2] >> 22;
            
            switch (tamMem){
                case 0b00: printf("l"); break;
                case 0b10: printf("w"); break;
                case 0b11: printf("b"); break;
                default: break;
            }
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
    int topA, topB, ipaux;
    ipaux = mv -> regs[CS];

    printf("\nel CS antes de entrar al ciclo de escritura es %d Y LLEGA HASTA %d\n",mv->regs[CS],mv->tablaSeg[posCS][1]);

    while (ipaux < mv -> regs[CS] + mv->tablaSeg[posCS][1]) {
        //printf("IPAUX APUNTA A: %02X \n",mv->mem[ipaux]);
        char byte = mv->mem[ipaux];
        topA = (byte >> 4) & 0x03;
        topB = (byte >> 6) & 0x03;
        mv->regs[OP1] = 0;
        mv->regs[OP2] = 0;
        mv->regs[OPC] = byte & 0x1F; 

        for (int i = 0; i < topB; i++) {
            ipaux++;
            mv->regs[OP2] = (mv->regs[OP2] << 8) | mv->mem[ipaux];
        }
        for (int i = 0; i < topA; i++) {
            ipaux++;
            mv->regs[OP1] = (mv->regs[OP1] << 8) | mv->mem[ipaux];
        }
        disassembler(*mv, topA, topB);
        ipaux++;
    }
    printf("\n\n");
}


void checkError(maquinaV mv){
    switch(mv.error){
        case 0: break;
        case 1: printf("\nError: Segmentation fault.\n");break;
        case 2: printf("\nError: División por 0.\n");break;
        case 3: printf("\nError: Instrucción inválida.\n");break;
        case 4: printf("\nError: Stack Overflow ");break;
        case 5: printf("\nError: Stack Underflow ");break;
        case 6: printf("\nError: Memoria insuficiente ");break;
        default: printf("\nError desconocido.\n");break;
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

void push4b(maquinaV *mv, int valor) {
    mv->regs[SP] -= 4; 
    printf("%d",valor);
    for (int i = 0; i < 4; i++){
        mv->mem[mv->regs[SP] + i] = (valor >> (8 * i)) & 0xFF;
        printf("mem[%d] = %02X\n", mv->regs[SP]+i, mv->mem[mv->regs[SP]+i]);
    }
}


void iniciaPila(maquinaV *mv, int argC, int argV){
    printf("argc: %d", argC);

    printf("argV: %04x", argV);
    
    if(argC != 0)
        push4b(mv,argV);
    else 
        push4b(mv,-1);
    printf("hola");

    push4b(mv,argC);
    push4b(mv,0xFFFFFFFF);

    printf("\nPushie %08X y %08X en la pila en iniciaPila",argC,argV);
}

void iniciaVm(maquinaV *mv,int argc, char *argv[]){
   
    char flagD, ArchVMX[ARCH_NAME_SIZE], ArchVMI[ARCH_NAME_SIZE], Parametros[CANT_PARAM][LEN_PARAM];    //Vector de parametros                                                
    unsigned int M = 0, TopeVecSegmentos = 2, cantParam = 0; //2???
    unsigned short int entrypoint = 0;
    int posPara = 0, i=0 , VectorSegmentos[CANT_SEG]; //  -1 por si no llega a haber ParaSegment 
    unsigned char Version;
    FILE *archvmx;

    /*Reescribir la lógica sin tantos ifs anidados, usar break y return. Queda más legible.*/

    if(argc <= 1)
        printf("\n No se especifico un archivo. \n");  
    else{
        if(strcmp(argv[1] + strlen(argv[1]) - 4, ".vmi") == 0){ //archivo.vmi, no hay .vmx = nada más cargar imagen
            FILE *archVmi = fopen(argv[1],"rb"); 
            if(!archVmi)
                printf("\nNo existe el .vmi especificado.");
            else{
                if(argc == 3 && strcmp(argv[2],"-d") == 0) //checkeo disassembler
                    flagD = 'S'; 

                leeVmi(mv,archVmi);         //si solo se especifica .vmi, se ignoran los parametros -p y la memoria m=M    

                if (mv->error == 6)    //Error en el tamaño de la memoria?
                    checkError(*mv);
                else{
                    if (flagD == 'S') //writeCycle usa ipaux, no toca el registro IP de la maquina.
                        writeCycle(mv);

                    ejecVmx(mv); //Continúa la ejecución desde donde la dejó el .vmi

                    checkError(*mv); //Las invocaciones de checkError tienen que ir en el main. Si hay error se tiene que cortar con break o return;
                }
            }
        }
        else
            if (strcmp(argv[1] + strlen(argv[1]) - 4, ".vmx") == 0){ //Se especificó un .vmx
                printf("\nApertura de .vmx");
                strcpy(ArchVMX,argv[1]);
                archvmx = fopen(ArchVMX,"rb");    // Abro .vmx  
                
                if (archvmx != NULL){
                    
                    for (i=0; i<=4 ; i++) // Bytes de descarte para llegar a leer la version
                        fread(&Version, 1, sizeof(Version), archvmx);
                    
                    fread(&Version, 1, sizeof(Version), archvmx);    //Lee Version
                    printf("\n");

                    if (Version == 1){
                        printf("\nVersion 1");
                        if (argc == 3 && strcmp(argv[2],"-d") == 0)//   .vmx -d
                            flagD = 'S';
                        leeVmx_MV1(archvmx, mv);       
                    }
                    else
                        if (Version == 2){
                            printf("\nejecucion de parte 2\n");
                            i=0;
                            while (i < argc){ // MAQUINA VIRTUAL PARTE 2     .vmx .vmi m=M -d -p

                                if (strcmp(argv[i] + strlen(argv[i]) - 4, ".vmx") == 0)
                                    strcpy(ArchVMX,argv[i]);

                                if (strcmp(argv[i] + strlen(argv[i]) - 4, ".vmi") == 0 ) //rescribir con un case
                                    strcpy(ArchVMI,argv[i]);

                                if (strncmp(argv[i],"m=",2) == 0) //si se especifica memoria
                                    M = tamaniomemoria(argv[i]);

                                if (strcmp(argv[i],"-d") == 0) //Si se llama a disassembler
                                    flagD = 'S';

                                if(strcmp(argv[i],"-p") == 0) //Si hay parametros
                                    for(int h = i+1 ; h < argc; h++ ){ 
                                        posPara += 1; //cantidad de parametros especificados.
                                        strcpy(Parametros[posPara],argv[h]); 
                                    }
                                i++;  
                            }

                            printf("Cantidad de parámetros: %d",cantParam);

                            leeVmx_MV2(archvmx, mv, M,Parametros,posPara,&entrypoint,VectorSegmentos,&TopeVecSegmentos);

                            tabla_segmentos (mv,VectorSegmentos,TopeVecSegmentos);

                            mv->regs[IP] =  (posCS << 16) | entrypoint;
                            printf("\nEL IP INICIA EN: %08x", mv->regs[IP]);                            
                            mv->regs[SP]= mv->tablaSeg[posSS][0] + mv->tablaSeg[posSS][1] + 1;  //Inicializa SP

                        }
                    
                    if (mv->error == 6) //Error en el tamaño de la memoria
                        checkError(*mv);
                    else{
                        if (flagD == 'S')
                            writeCycle(mv);
                        
                        ejecVmx(mv);
                        checkError(*mv);
                    }
                    printf("\n");
                }
                else
                    printf("Error al abrir el archivo. vmx");
            }             
            else
                printf("No existe el archivo vmx");
    }    
}

int main(int argc, char *argv[]) {
    maquinaV mv;
    mv.error = 0;

    memset(mv.mem, 0 ,MEM_SIZE);
    iniciaVm(&mv,argc, argv);

    printf("\nIP: %08X",mv.regs[IP]);

    

    return 0;        
}