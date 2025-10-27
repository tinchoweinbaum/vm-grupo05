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

void tabla_segmentos (maquinaV *mv){

    unsigned int  i, postablaseg = 0;

    mv->tablaSeg[0][0] = 0; // Siempre voy a tener la 1ra posicion de la table de segmentos en 0

    for (i=10; i < 16; i++)     //EAX ... EFX
        if (mv->regs[i] != -1){
            if (postablaseg == 0)   //Tabla vacia
                mv->tablaSeg[postablaseg][1] = mv->regs[i];       
            else{
                mv->tablaSeg[postablaseg][0] = mv->tablaSeg[postablaseg-1][1];
                mv->tablaSeg[postablaseg][1] = mv->regs[i] + mv->tablaSeg[postablaseg][0];
            }
            switch (i){
                case 10: posPS = postablaseg; mv->regs[PS] = mv->tablaSeg[postablaseg][0] ; break;  // Establezco punteros y posiciones de los segmentos de la tabla en las variables
                case 11: posKS = postablaseg; mv->regs[KS] = mv->tablaSeg[postablaseg][0] ; break;
                case 12: posCS = postablaseg; mv->regs[CS] = mv->tablaSeg[postablaseg][0] ; break;
                case 13: posDS = postablaseg; mv->regs[DS] = mv->tablaSeg[postablaseg][0] ; break;
                case 14: posES = postablaseg; mv->regs[ES] = mv->tablaSeg[postablaseg][0] ; break;
                case 15: posSS = postablaseg; mv->regs[SS] = mv->tablaSeg[postablaseg][0] ; break;
            }
            postablaseg++;     
        }
}


void leeVmx_MV2(FILE *arch, maquinaV *mv, unsigned int M, char Parametros[][LEN_PARAM], int posPara, unsigned int *entrypoint) {
    
    unsigned char byteAct;
    unsigned int j, tamseg, paramlen ,memor = 0 ,VecArgu[CANT_PARAM];
    int i, posArgu = 0;
    
    fseek(arch,0,0);


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


    for(i = 6; i <= HEADER_SIZE_V2-8; i++) {         // Tamaños desde Code Segment hasta Stack Segment

        tamseg = 0;
        fread(&byteAct, 1, sizeof(byteAct), arch);
        tamseg = (tamseg << 8) | byteAct;
        fread(&byteAct, 1, sizeof(byteAct), arch);
        tamseg = (tamseg << 8) | byteAct; 

        if (tamseg > 0){
            mv->regs[i + 6]= tamseg;    // Guardo los tamaños en ECX hasta EFX
            memor += tamseg;
        }
        else
            mv->regs[i + 6]= -1;   
    }

    tamseg = 0;                                     // Constant Segment
    fread(&byteAct, 1, sizeof(byteAct), arch);
    tamseg = (tamseg << 8) | byteAct;
    fread(&byteAct, 1, sizeof(byteAct), arch);
    tamseg = (tamseg << 8) | byteAct; 

    if (tamseg > 0){
        mv->regs[EBX]= tamseg;   
        memor += tamseg;
    }
    else
        mv->regs[EBX]= -1;   
                                
    fread(&byteAct, 1, sizeof(byteAct), arch);      //Entry pont
    *entrypoint = (*entrypoint << 8) | byteAct;
    fread(&byteAct, 1, sizeof(byteAct), arch);
    *entrypoint = (*entrypoint << 8) | byteAct; 


    //////////  CARGA MV  //////////
    
    if (memor > mv->tamMem)    //Segmentos mayor que memoria disponible 
        mv->error = 6; 
    else{
        memor = 0;
        tamseg = 0;
        if (posPara != -1){     //  Param Segment
        
            for (i =0; i <= posPara; i++)           // Tamaño
                tamseg += strlen(Parametros[i]) + 5; //  5 = 1 (el 0 que separa cada palabra) + 4 (puntero a la palabra)
               
            mv->regs[EAX] = tamseg;

            for (i=0; i<=posPara; i++){
                VecArgu[posArgu++]=memor;
                paramlen = strlen(Parametros[i]);
                for (j = 0; j < paramlen; j++)
                    mv->mem[memor++]= Parametros[i][j];
                mv->mem[memor++] = 0;
            }

            for (i=0; i<posArgu; i++){
                mv->mem[memor++] = (VecArgu[i] >> 24) & 0xFF;
                mv->mem[memor++] = (VecArgu[i] >> 16) & 0xFF;
                mv->mem[memor++] = (VecArgu[i] >> 8) & 0xFF;
                mv->mem[memor++] = VecArgu[i] & 0xFF;
            }
        }
        else
            mv->regs[EAX] = -1;


        if (mv->regs[EBX] != -1)                       
            posCS = mv->regs[EBX] + memor;
        else
            posCS = memor;
        
        for (i = 0; i < mv->regs[ECX] ; i++){           // Carga el Code Segment y Const Segment
            fread(&byteAct,1,sizeof(byteAct),arch);
            mv->mem[posCS] = byteAct;
            posCS++;
        }

        for (i = 0; i < mv->regs[EBX] ; i++){
            fread(&byteAct,1,sizeof(byteAct),arch);
            mv->mem[memor] = byteAct;
            memor++;
        }

    }
        
    fclose(arch); 

}

void leeVmi(maquinaV *mv, FILE *archVmi){ 

    unsigned char byteAct;
    unsigned int tamseg, base = 0, i, tamMem = 0, cantSeg = 0;

    //HEADER Y VERSIÓN//

    for (int i = 0; i <= HEADER_SIZE_VMI - 3; i++){     // VMI25
        fread(&byteAct,1,sizeof(byteAct),archVmi);
        printf("%c",byteAct); 
    }

    fread(&byteAct,1,sizeof(byteAct),archVmi);   // Version
    printf("\nVersion de .vmi: %d \n",byteAct);

    fread(&byteAct,1,sizeof(byteAct),archVmi);  // Tamaño de memoria
    tamMem = tamMem | byteAct;
    fread(&byteAct,1,sizeof(byteAct),archVmi); 
    tamMem = (tamMem << 8) | byteAct; 
    printf("Memoria: %d \n",tamMem);
 

    if(tamMem > MEM_SIZE)
        mv->error = 6;          //Memoria insuficiente
    else{

        mv->tamMem = tamMem;

        //VOLCADO DE REGISTROS//

        for (i = 0; i < REG_SIZE; i ++)     //Setea registros en cero 
           mv->regs[i] = 0;  

        for (i = 0; i < REG_SIZE; i ++){             //Guarda registros
            fread(&byteAct,1,sizeof(byteAct),archVmi);
            mv->regs[i] = mv->regs[i] | byteAct;   
        }
        
        printf("\n");

        //100	150  300    3294	30	846 tamaños que puse en .vmi para probar
        //TABLA DE SEGMENTOS//
        for (i = 0; i < 16; i++){ //lee 8 bloques de 4 bytes (tabla de segmentos)
             
            tamseg = 0;                                  
            fread(&byteAct,1,sizeof(byteAct),archVmi);
            tamseg = tamseg | byteAct;
            fread(&byteAct,1,sizeof(byteAct),archVmi);    
            tamseg = (tamseg << 8) | byteAct;

            if (base == 0){   
                mv->tablaSeg[i][0] = tamseg; // Asigna BASE del segmento
                printf("%d ",mv->tablaSeg[i][0]); 
                base++;
            }   
           else{
            mv->tablaSeg[i][1] = tamseg; // Asigna OFFSET del segmento
            printf("%d \n",mv->tablaSeg[i][1]); 
            base = 0;
           }
        }

        //CHECKEO DE SEGMENTOS//

        if(mv->regs[PS] != -1){
            cantSeg++;
            posPS = cantSeg;
        }

        if(mv->regs[KS] != -1){
            cantSeg++;
            posKS = cantSeg;
        }

        if(mv->regs[CS] != -1){
            cantSeg++;
            posCS = cantSeg;
        }

        if(mv->regs[DS] != -1){
            cantSeg++;
            posDS = cantSeg;
        }

        if(mv->regs[ES] != -1){
            cantSeg++;
            posES = cantSeg;
        }

        if(mv->regs[SS] != -1){
            cantSeg++;
            posSS = cantSeg;
        }

        //VOLCADO DE MEMORIA//

        for ( i = 0; i < tamMem; i++){
            fread(&byteAct,1,sizeof(byteAct),archVmi);
            mv->mem[i] = byteAct;
            printf("%d ",mv->mem[i]);
        }
    }
    fclose(archVmi);
}

int leeOp(maquinaV *mv,int tOp){
    int valor = 0;
    unsigned char byteAct;

    for(int i = 0; i < tOp; i++){
        if(mv->regs[IP] < mv->tablaSeg[posCS][0] || mv->regs[IP] > mv->tablaSeg[posCS][1]){ //si me caigo del CS
            mv->error = 1;
            break;
        }
        mv->regs[IP]++;
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
            case 0x00: menuSYS(mv); break;                                            
            case 0x08: NOT(mv, topB); break;
            case 0x0B: PUSH(mv, topB);break;
            case 0x0C: POP(mv, topB);break;
            case 0x0D: CALL(mv);break;                                                         
            default: mv -> error = 3; break;
        }
    }
}




void ejecVmx(maquinaV *mv){

    unsigned char byteAct;
    char ins, tOpB, tOpA;
    int opA, opB, auxIp;

    while ( mv -> error == 0 && mv -> regs[IP] >= mv -> tablaSeg[posCS][0] && mv -> regs[IP] <= mv -> tablaSeg[posCS][1]) {
        byteAct = mv -> mem[mv ->regs[IP]];

        ins = byteAct & 0x1F;
        tOpA = (byteAct >> 4) & 0x3;
        tOpB = (byteAct >> 6) & 0x3;

        mv -> regs[OPC] = ins;


        /*SIN OPERANDOS*/
        if (tOpB == 0){
            switch (mv -> regs[OPC])
            {
                case 0xE: RET(mv); break;
                case 0xF: STOP(mv); break;
                default: mv -> error = 3; break;
            }
        } else {

        /*CARGO OPERANDO B*/
        opB = leeOp(mv, tOpB);
        if (mv->error != 0) break;
        mv->regs[OP2] = opB;

        /*CARGO OPERANDO A*/
        opA = leeOp(mv, tOpA);//si el operando no existe no lee y salta
        if (mv->error != 0) break;
        mv->regs[OP1] = opA;

        auxIp = mv -> regs[IP];

        /*MENUES DE OPERACIONES*/
        if (tOpA != 0 && tOpB != 0)
            twoOpFetch(mv, tOpA, tOpB);
        else
            oneOpFetch(mv, tOpB);
        }

        if (mv->error != 0) break;

        /*SI NO SALTE AVANZO MANUALMENTE*/
        if (mv -> regs[OPC] != 0x0d ||  mv->regs[IP] == auxIp) {
            mv->regs[IP]++;
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
    while (ipaux < mv->tablaSeg[posCS][1]) {
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

void iniciaVm(maquinaV *mv,int argc, char *argv[]){
   
    char flagD, ArchVMX[ARCH_NAME_SIZE], ArchVMI[ARCH_NAME_SIZE], Parametros[CANT_PARAM][LEN_PARAM];    //Vector de parametros                                                
    unsigned int M = 0, entrypoint = 0;
    int posPara = -1, i=0 ; //  -1 por si no llega a haber ParaSegment 
    unsigned char Version;
    FILE *archvmx;

    if(argc <= 1)
        printf("\n No se especifico un archivo. \n");  
    else{
        if(strcmp(argv[1] + strlen(argv[1]) - 4, ".vmi") == 0){ //archivo.vmi, no hay .vmx = nada más cargar imagen
            FILE *archVmi = fopen(argv[1],"rb"); 
            if(!archVmi)
                printf("\nNo existe el .vmi especificado.");
            else{
                if(argc == 3 && strcmp(argv[2],"-d") == 0)
                    flagD = 'S'; 

                leeVmi(mv,archVmi);         //si solo se especifica .vmi, se ignoran los parametros -p y la memoria m=M       
                if (mv->error == 6)    //Error en el tamaño de la memoria
                    checkError(*mv);
                else{
                    if (flagD == 'S')
                        writeCycle(mv);
                    ejecVmx(mv);
                    checkError(*mv);
                }
            }
        }
        else
            if (strcmp(argv[1] + strlen(argv[1]) - 4, ".vmx") == 0){
                strcpy(ArchVMX,argv[1]);
                archvmx = fopen(ArchVMX,"rb");    // Abro .vmx  
                
                if (argv[1] != NULL){

                    for (i=0; i<=4 ; i++) // Bytes de descarte para llegar a leer la version
                        fread(&Version, 1, sizeof(Version), archvmx);
                    
                    fread(&Version, 1, sizeof(Version), archvmx);    //Lee Version
                    printf("\n");

                    if (Version == 1){
                        if (argc == 3 && strcmp(argv[2],"-d") == 0)//   .vmx -d
                            flagD = 'S';
                    leeVmx_MV1(archvmx, mv);       
                    }
                    else
                        if (Version == 2){
                            i=0;
                            while (i < argc){ // MAQUINA VIRTUAL PARTE 2     .vmx .vmi m=M -d -p
                                if (strcmp(argv[i] + strlen(argv[i]) - 4, ".vmx") == 0)
                                    strcpy(ArchVMX,argv[i]);

                                if (strcmp(argv[i] + strlen(argv[i]) - 4, ".vmi") == 0 )
                                    strcpy(ArchVMI,argv[i]);

                                if (strncmp(argv[i],"m=",2) == 0)
                                    M = tamaniomemoria(argv[i]);

                                if (strcmp(argv[i],"-d") == 0)
                                    flagD = 'S';

                                if(strcmp(argv[i],"-p") == 0)
                                    for(int h = i+1 ; h < argc; h++ ){                
                                        posPara += 1;
                                        strcpy(Parametros[posPara],argv[h]); 
                                    }
                                i++;  
                            }

                            leeVmx_MV2(archvmx, mv, M,Parametros,posPara,&entrypoint);
                            tabla_segmentos (mv);

                            tabla_segmentos (mv);

                            int aux = mv->regs[CS];
                            mv->regs[IP] = 0;


                            aux = aux << 16;
                            mv->regs[IP]= mv->regs[IP] | aux;

                            mv->regs[IP] =  mv->regs[IP] | entrypoint;
                            mv->regs[SP]= mv->tablaSeg[posSS][1];                //Inicializa SP

                        }
                    
                    if (mv->error == 6) //Error en el tamaño de la memoria
                        checkError(*mv);
                    else{
                        if (flagD == 'S')
                            writeCycle(mv);
                        
                        ejecVmx(mv);
                        checkError(*mv);
                    }
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
    int i;

    memset(mv.mem, 0 ,MEM_SIZE);
    iniciaVm(&mv,argc, argv);

<<<<<<< HEAD
=======
   /* for (int i = mv.regs[SP]; i < mv.tablaSeg[posSS][0]+mv.tablaSeg[posSS][1]; i++)
    {
        printf("%2x ",mv.mem[i]);
    }
    */


>>>>>>> f5a11100ce69bb65f078b7d62f980092c9e28ef1
    return 0;        
}
