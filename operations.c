#include "operations.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>


/*IMPORTANTE:
    En la 1ra parte del TP estuvimos usando tablaSeg[0][0] como sinónimo de CS y tablaSeg[1][0] como sinónimo de DS,
    ya vimos que ahora en la 2da parte esto no es así XD, habría que reescribir en las funciones que acceden a la tabla para buscar cualquier segmento
    la parte en la que accede a una posición de la matriz, que haga tablaSeg[CS] y no tablaSeg[0][0]*/
int posPS = -1;
int posKS = -1;
int posCS = -1;
int posDS = -1;
int posES = -1;
int posSS = -1;

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
    return mv.regs[CC]; // asumimos que CMP garantiza -1,0,1
}

int traducePuntero(maquinaV *mv,int puntero){
    
    /*En la parte alta del int puntero viene el segmento
      En la parte baja el offset*/
    
    int posFis = 0, seg, offset;
    seg = (puntero >> 16) & 0xFFFF;
    offset = puntero & 0xFFFF;
    posFis = mv->tablaSeg[seg][0] + offset;
    return posFis;
}

int checkSegFault(maquinaV *mv,int dir,int bytes){ //True si se intenta acceder a una posición inválida (segFault)
    int baseDS = mv->tablaSeg[posDS][0];
    int topeDS = mv->tablaSeg[posDS][1]; //topes y bases de segmentos de mem. en los que se puede
    int baseES = mv->tablaSeg[posES][0]; //escribir legalmente
    int topeES = mv->tablaSeg[posES][1];
    int baseSS = mv->tablaSeg[posSS][0];
    int topeSS = mv->tablaSeg[posSS][1];
    int basePS = mv->tablaSeg[posPS][0];
    int topePS = mv->tablaSeg[posPS][1];

    return (!((dir >= baseDS && dir + bytes <= topeDS) || (dir >= baseES && dir + bytes <= topeES) || (dir >= baseSS && dir + bytes <= topeSS) || (dir >= basePS && dir + bytes <= topePS)));
}

int calculabytes(maquinaV *mv, int iOp){
    int tambytes, rdo;

    tambytes = (mv -> regs[iOp] >> 22) & 0b11;
    switch (tambytes){
        case 0b00:  rdo = 4; break;
        case 0b10:  rdo = 2; break;
        case 0b11:  rdo = 1; break;
    }
    return rdo;
}

void escribeIntMem(maquinaV *mv, int dir, int valor, int iOp) {
    int bytes; //Cantidad de bytes a escribsir.
    
    if(mv -> regs[OPC] == 00) //Si se llamó a esta función desde SYS
        bytes = (mv -> regs[ECX] >> 16) & 0b11;
    else
        bytes = calculabytes(mv, iOp);
        
    if (0) { //checkSegFault devuelve True cuando hay seg fault
        mv->error = 1;
        return;
    }

    for (int i = 0; i < bytes; i++) {
        unsigned char byte = (valor >> (8 * (bytes - 1 - i))) & 0xFF; // big endian
        mv->mem[dir + i] = byte;
    }

    mv->regs[MAR] = dir;
    mv->regs[MBR] = valor;
  //  mv->regs[LAR] = (1 << 16) | (dir - base); //el lar guarda el segmento en el que escribió en los bits altos del registro?
}


void leeIntMem(maquinaV *mv, int dir, int *valor, int iOp) {
    int bytes; //Cantidad de bytes a escribir.

    if(mv -> regs[OPC] == 00) //Si se llamó a esta función desde SYS
        bytes = (mv -> regs[ECX] >> 16) & 0b11;
    else
        bytes = calculabytes(mv,iOp);

   /* if (checkSegFault(mv,dir,bytes)) { //MAL, leeIntMem puede acceder a TODA la memoria
        mv->error = 1;
        return;
    }
        */

    *valor = 0;
    for (int i = 0; i < bytes; i++) {
        *valor = (*valor << 8) | (unsigned char)mv->mem[dir + i];
    }

    mv->regs[MAR] = dir;
    mv->regs[MBR] = *valor;
    //mv->regs[LAR] = (1 << 16) | (dir - base); LAR??!!
}


void setValor(maquinaV *mv, int iOP, int OP, char top) { // iOP es el indice de operando, se le debe pasar OP1 o OP2 si hay que guardar funciones en el otro operando por ejemplo en el SWAP, OP es el valor extraido de GETOPERANDO
   int offset,reg,espacio, bytes ,cantBytes;


    if (top == 1){ // registro
        reg = mv->regs[iOP] & 0x1F;
        bytes = (mv->regs[iOP] >> 6) & 0b11;

        // aseguramos que OP tenga solo los bits válidos según bytes
        switch (bytes) {
            case 0:  // todo el registro (4 bytes)
                mv->regs[reg] = OP;
                break;
            case 1:  // AL (primer byte)
                mv->regs[reg] = (mv->regs[reg] & 0xFFFFFF00) | (OP & 0xFF);
                break;
            case 2:  // AH (tercer byte)
                mv->regs[reg] = (mv->regs[reg] & 0xFFFF00FF) | ((OP & 0xFF) << 8);
                break;
            case 3:  // AX (16 bits, dos bytes bajos)
                mv->regs[reg] = (mv->regs[reg] & 0xFFFF0000) | (OP & 0xFFFF);
                break;
        }

        // escritura segura en el registro según bytes
    } else {
            if(top == 3){ //memoria

                //printf("\nOP: %d",OP);
                //printf("\nMVREGSIOP en BINARIO %X",mv -> regs[iOP]);

                cantBytes = 4 - ((mv->regs[iOP] >> 22) & 0b11);
                //printf("cantBytes: %d\n", cantBytes);
                reg = (mv -> regs[iOP] >> 16) & 0x1F;//cargo el registro
                //printf("reg: %d %X\n", reg,mv->regs[reg]);
;

                offset = (int16_t)(mv->regs[iOP] & 0xFFFF); //OFFSET HARDCODEADO
                //printf("offset: %d\n", offset);

                espacio = traducePuntero(mv, mv->regs[reg]) + offset; // espacio = direccion en la q se comienza a escirbir
                //printf("espacio: %d\n", espacio);
                
                

                for (int i = 0; i < cantBytes; i++) {
                    mv->mem[espacio + i] = (OP >> (8 * (cantBytes - 1 - i))) & 0xFF;  // big endian
                }

            printf("\n");
        }
    } 
}



void getValor(maquinaV *mv,int iOP, int *OP, char top) {
    int i, offset, reg, bytes,cantBytes,espacio;

    if (top == 2) // inmediato
        *OP = mv->regs[iOP];
    else if (top == 1) { // registro
        reg = mv -> regs[iOP] & 0x1F;
        bytes = (mv->regs[iOP] >> 6) & 0b11;

        switch (bytes){
            case 0: *OP = mv -> regs[reg]; break;
            case 1: *OP = mv -> regs[reg] & 0xFF; break;
            case 2: *OP = (mv -> regs[reg] >> 8) & 0xFF; break;
            case 3: *OP = mv -> regs[reg] & 0xFFFF; break;
        }
    } 
    else { // memoria
        if(top == 3){ //memoria

            cantBytes = 4 - ((mv->regs[iOP] >> 22) & 0b11);

            reg = (mv -> regs[iOP] >> 16) & 0x1F;   //cargo el registro

            offset = (int16_t)(mv->regs[iOP] & 0xFFFF); 

            espacio = traducePuntero(mv, mv->regs[reg]) + offset; // espacio = direccion en la q se comienza a escirbir

            *OP = 0;

            for (i = 0; i < cantBytes; i++) {
                *OP = (*OP >> 8) | mv->mem[espacio + i];

            }
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
        setValor(mv,OP1,res,tOpA);
        actNZ(mv,res);
        mv->regs[AC] = aux1 % aux2;
    }
}

void CMP(maquinaV *mv, char tOpA, char tOpB){
    int aux1, aux2, bytes;

    getValor(mv,OP2,&aux2,tOpB);
    getValor(mv,OP1,&aux1,tOpA);
    printf("aux1 %x aux2 %x     ",aux1,aux2);

    if (tOpA == 3){
        bytes = 4 - ((mv -> regs[OP1] >> 22) & 0b11);
        if (bytes == 1) {
            aux2 = (int8_t)(aux2 & 0xFF);
            aux1 = (int8_t)(aux1 & 0xFF);
        } else if (bytes == 2) {
            aux2 = (int16_t)(aux2 & 0xFFFF);
            aux1 = (int16_t)(aux1 & 0xFFFF);
        }
    }else
        if (tOpA == 1){
            bytes = 4 - ((mv -> regs[OP1] >> 22) & 0b11);
            if (bytes == 1) {
                aux2 = (int8_t)(aux2 & 0xFF);
                aux1 = (int8_t)(aux1 & 0xFF);
            } else if (bytes == 2) {
                aux2 = (int16_t)(aux2 & 0xFFFF);
                aux1 = (int16_t)(aux1 & 0xFFFF);
            }

        }

    printf("aux1 %x aux2 %x \n",aux1,aux2);
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
}

void SYS2(maquinaV *mv){

    int pos, posfisica,tipo, n, bytes, val, inicio;

    pos = mv->regs[EDX];
    posfisica = traducePuntero(mv, mv -> regs[EDX]);
    tipo = mv->regs[EAX];
    n = mv->regs[ECX] & 0xFFFF;
    bytes = (mv->regs[ECX] >> 16) & 0xFFFF;
    inicio = pos;

    if (posfisica >= mv -> tablaSeg[posDS][0] && posfisica + n * bytes <= mv -> regs[SS] + mv -> tablaSeg[posSS][1]){
        if (n != 0 && bytes != 0)
        {
            for (int i = 0; i < n; i++)
            {
                inicio = posfisica;
                val = 0;  
                for (int j = 0; j < bytes; j++)
                {
                    val = (val << 8) | mv->mem[posfisica];
                    posfisica++;
                }

                printf("\n[%04X]: ", inicio);
                if(tipo & 0x10){binario(val);}
                if(tipo & 0x08){printf(" 0x%X\t", val);}
                if(tipo & 0x04){printf(" 0o%o\t", val);}
                if(tipo & 0x02){printf(" %c\t", (char)val);}
                if(tipo & 0x01){printf(" %d\t", val);}
                printf("\n");
            }
        }
    } else 
        mv -> error = 1;
}

void SYS1(maquinaV *mv){

    int posfisica, base, tope, n, bytes, val, i, j, inicio, tipo, seg;


    posfisica = traducePuntero(mv,mv->regs[EDX]);
    seg = (mv -> regs[EDX] >> 16) & 0xFFFF;
    base = mv -> tablaSeg[seg][0];
    tope = mv -> tablaSeg[seg][0] + mv -> tablaSeg[seg][1];
    tipo = mv->regs[EAX];
    n = mv -> regs[ECX] & 0xFFFF;
    bytes = (mv -> regs[ECX] >> 16) & 0xFFFF;


    if (posfisica >= base && posfisica + bytes * n < tope){

        if (n != 0 && bytes != 0){ 
            for (i = 0; i < n; i++)
            {
                inicio = posfisica;
                printf("[%04x]:",inicio);

                if (tipo & 0x10) 
                {
                    char bit;
                    val = 0;
                    j = 0;
                   
                    scanf(" %c", &bit); 
                    while (j < (bytes * 8) && (bit == '1' || bit == '0'))
                    {
                        val = (val << 1);
                        if (bit == '1') val = val | 1;
                        
                        scanf(" %c", &bit);
                        j++;
                    }                       
                } else {
                    if (tipo & 0x08) scanf("%x", &val); 
                    else if (tipo & 0x04) scanf("%o", &val); 
                    else if (tipo & 0x02) scanf(" %c", (char*)&val);
                    else if (tipo & 0x01) scanf("%d", &val); 
                    else {
                        mv->error = 3;
                        return;
                    }

                }

                escribeIntMem(mv,posfisica,val, OP2);
                posfisica += bytes; 
            }   
        }
    }
}

void SYS3(maquinaV *mv){

    int posfisica = traducePuntero(mv, mv -> regs[EDX]),i = 0;;
    char str[400],car;
    
    scanf("%s", str);
    car = str[i];
    while (car != '\0')
    {
        mv -> mem[posfisica] = car;
        posfisica++;
        i++;
        car = str[i];
    }
    mv -> mem[posfisica] = '\0'; // Agregar el carácter nulo al final

    printf("\n");

}

void SYS4(maquinaV *mv){

    int posfisica = traducePuntero(mv, mv -> regs[EDX]);
    char car;

    car = mv -> mem[posfisica];
    while (car != '\0')
    {
        printf("%c", mv -> mem[posfisica]);
        posfisica++;
        car = mv -> mem[posfisica];
    }
}

void SYSF(maquinaV *mv){
    creaVmi(mv);
    getchar();
}

void menuSYS(maquinaV *mv){
    int orden = mv -> regs[OP2];

    switch (orden){
        case 0x1: SYS1(mv); break; //lectura
        case 0x2: SYS2(mv); break; //escritura
        case 0x3: SYS3(mv); break; //lectura string
        case 0x4: SYS4(mv); break; //escritura string
        //case 0x7: clrscr(); break; //limpio pantalla  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        case 0xF: SYSF(mv); break; //creo vmi
        default: mv -> error = 3; break;
    }
}

void creaVmi(maquinaV *mv){

    char *textoHeader = "VMI25";
    unsigned short int auxShort;
    FILE *archVmi = fopen("breakpoint.vmi","wb");

    if(archVmi == NULL){
        printf("No se pudo crear el archivo .vmi.");
        return;
    }

    /*HEADER*/

    fwrite(textoHeader, 1, strlen(textoHeader), archVmi); //Escribe VMI25 en el header

    unsigned char temp = 0x01; //Escribe la version, siempre es 1
    fwrite(&temp,1,sizeof(temp),archVmi);

    /*TAMAÑO DE LA MEMORIA*/
    fwrite(&(mv->tamMem),1,sizeof(mv->tamMem),archVmi); //Escribe el tamaño de la memoria en el archivo

    /*VOLCADO DE REGISTROS*/

    for (int i = 0; i < REG_SIZE; i++)
        fwrite(&(mv->regs[i]),1,sizeof(mv->regs[i]),archVmi);

    /*VOLCADO DE TABLA DE SEGMENTOS*/

    for (int i = 0; i < 8; i++){
        auxShort = mv->tablaSeg[i][0];
        fwrite(&auxShort,1,sizeof(auxShort),archVmi);
        auxShort = mv->tablaSeg[i][1];
        fwrite(&auxShort,1,sizeof(auxShort),archVmi);
    }

    /*VOLCADO DE MEMORIA*/

    for (unsigned int i = 0; i < mv->tamMem; i++)
        fwrite(&(mv->mem[i]),1,sizeof(mv->mem[i]),archVmi);


    fclose(archVmi);

}

void JMP(maquinaV *mv,int opB){
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) | (opB & 0x0000FFFF);
}

void JZ(maquinaV *mv,int opB){
    if(NZ(*mv) == 0)
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) | (opB & 0x0000FFFF);
}

void JP(maquinaV *mv,int opB){
    if(NZ(*mv) > 0)
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) | (opB & 0x0000FFFF);
}

void JN(maquinaV *mv,int opB){
    if(NZ(*mv) < 0)
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) | (opB & 0x0000FFFF);
}

void JNZ(maquinaV *mv,int opB){
    if(NZ(*mv) > 0 || NZ(*mv) < 0)
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) | (opB & 0x0000FFFF);
}

void JNP(maquinaV *mv,int opB){
    if(NZ(*mv) <= 0)
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) | (opB & 0x0000FFFF);
}

void JNN(maquinaV *mv, int opB){
    if(NZ(*mv) >= 0)
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) | (opB & 0x0000FFFF);
}

void STOP(maquinaV *mv){
    mv->regs[IP] = 0xFF;
}

/********OPERACIONES DE LA PILA**********/


// Obtiene la dirección física del SP
static int spFisico(maquinaV *mv) {
    return traducePuntero(mv, mv->regs[SP]);
}


void PUSH(maquinaV *mv, char topB) {
    int valor;
    getValor(mv, OP2, &valor, topB);

    int spF = spFisico(mv) - 4; // decrecemos el SP físico (stack crece hacia abajo)

    if (spF < mv->tablaSeg[posSS][0]) {
        mv->error = 4; // STACK OVERFLOW
        return;
    }

    mv->regs[SP] -= 4; // decremento lógico del SP

    // Escribimos valor byte a byte
    for (int i = 0; i < 4; i++)
        mv->mem[spF + i] = (valor >> (8 * (3 - i))) & 0xFF;
}

void POP(maquinaV *mv, char topB) {
    int spF = spFisico(mv);

    if (spF > mv->tablaSeg[posSS][0] + mv->tablaSeg[posSS][1] - 4) {
        mv->error = 5; // STACK UNDERFLOW
        return;
    }

    int valor = 0;
    for (int i = 0; i < 4; i++)
        valor = (valor << 8) | mv->mem[spF + i];

    setValor(mv, OP2, valor, topB);
    mv->regs[SP] += 4; // incremento lógico del SP
}

void CALL(maquinaV *mv) {
    int spF = spFisico(mv) - 4;
    if (spF < mv->tablaSeg[posSS][0]) {
        mv->error = 4; // STACK OVERFLOW
        return;
    }

    mv->regs[SP] -= 4;

    int retorno = mv->regs[IP] + 1;
    for (int i = 0; i < 4; i++)
        mv->mem[spF + i] = (retorno >> (8 * (3 - i))) & 0xFF;

    int nuevaIP = mv->regs[OP2] + mv->tablaSeg[posCS][0];
    if (nuevaIP >= mv->tablaSeg[posCS][0] && nuevaIP < mv->tablaSeg[posCS][0] + mv->tablaSeg[posCS][1])
        mv->regs[IP] = (mv->regs[IP] & 0xFFFF0000) + mv->regs[OP2];
    else
        mv->error = 1; // SEGMENTATION FAULT
}

void RET(maquinaV *mv) {
    int spF = spFisico(mv);

    if (spF > mv->tablaSeg[posSS][0] + mv->tablaSeg[posSS][1] - 4) {
        mv->error = 5; // STACK UNDERFLOW
        return;
    }

    int retorno = 0;
    for (int i = 0; i < 4; i++)
        retorno = (retorno << 8) | mv->mem[spF + i];

    mv->regs[IP] = retorno;
    mv->regs[SP] += 4;
}
