#include "operations.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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
    switch (mv.regs[CC]){
        case -1: return -1; break;
        case 0: return 0; break;
        case 1: return 1; break;
    }
    return 0;
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
   int offset,reg,espacio, bytes;


    if (top == 1){ // registro
        reg = mv->regs[iOP] & 0x1F;
        bytes = (mv->regs[iOP] >> 6) & 0b11;

        // aseguramos que OP tenga solo los bits válidos según bytes
        unsigned int op_val = 0;
        switch(bytes){
            case 0: op_val = OP; break;            // todo el registro
            case 1: op_val = OP & 0xFF; break;     // 1er byte (LSB)
            case 2: op_val = OP & 0xFF; break;     // 2do byte (byte alto, AH/DH)
            case 3: op_val = OP & 0xFFFF; break;   // 16 bits
            default: 
                printf("\nHUBO UN ERROR EN OP"); 
                return;
        }

        // escritura segura en el registro según bytes
        switch (bytes){
            case 0:
                mv->regs[reg] = op_val;
                break;
            case 1:
                mv->regs[reg] = (mv->regs[reg] & 0xFFFFFF00) | op_val;
                break;
            case 2:
                mv->regs[reg] = (mv->regs[reg] & 0xFFFF00FF) | (op_val << 8);
                break;
            case 3:
                mv->regs[reg] = (mv->regs[reg] & 0xFFFF0000) | op_val;
                break;
        }
    } else {
        if(top == 3){ //memoria

            reg = mv -> regs[iOP] >> 16;//cargo el registro
                
                if (reg >= 0 && reg <= 31){ // si es un registro valido

                    offset = mv -> regs[iOP] & 0x00FF; //cargo el offset
                    espacio = mv -> regs[reg] + offset; // cargo el espacio en memoria
                
                    if ((espacio + 3>= mv -> tablaSeg[posDS][0]) && (espacio + 3 < mv -> tablaSeg[posDS][0] + mv -> tablaSeg[posDS][1])) // si el espacio en memoria es valido
                        escribeIntMem(mv,espacio,OP, iOP); // guardo el valor
                    else{
                        mv -> error = 1; // si no error 1
                        return;
                    }
                } else{
                    mv -> error = 1;// si no es un registro valido error 1
                    return;
                }
        }
    } 
}

void getValor(maquinaV *mv,int iOP, int *OP, char top) {
    int offset, reg, bytes;

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
        offset = mv->regs[iOP] & 0x00FF;
        reg = mv->regs[iOP] >> 16;
        int dir = mv->regs[reg] + offset;

        /*if (dir < mv->tablaSeg[posDS][0] || dir + 3 >= mv->tablaSeg[posDS][0] + mv->tablaSeg[posDS][1]) {
            mv->error = 1;
        } else {
            leeIntMem(mv, dir, OP, iOP);
        }*/
       leeIntMem(mv,dir,OP,iOP); //CREO que está bien no verificar. Si no me equivoco yo puedo acceder a los datos de toda la memoria?
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

void SYS2(maquinaV *mv){
    int pos, tipo, n, bytes, val, inicio;

    pos = mv->regs[EDX];
    tipo = mv->regs[EAX];
    n = mv->regs[ECX] & 0xFFFF;
    bytes = (mv->regs[ECX] >> 16) & 0xFFFF;
    inicio = pos;

    if (pos >= mv -> regs[CS] && pos + n * bytes <= mv -> regs[SS] + mv -> tablaSeg[posSS][1]){
        if (n != 0 && bytes != 0)
        {
            for (int i = 0; i < n; i++)
            {
                inicio = pos;
                val = 0;  
                for (int j = 0; j < bytes; j++)
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
        }
    } else {
        mv -> error = 1;
    }
}

void SYS1(maquinaV *mv){
    int pos, base, tope, n, bytes, val, i, j, inicio, tipo, seg;


    pos = mv -> regs[EDX];
    seg = (mv -> regs[EDX] >> 16) & 0xFFFF;
    base = mv -> tablaSeg[seg][0];
    tope = mv -> tablaSeg[seg][0] + mv -> tablaSeg[seg][1];
    tipo = mv->regs[EAX];
    n = mv -> regs[ECX] & 0xFFFF;
    bytes = (mv -> regs[ECX] >> 16) & 0xFFFF;


    if (pos >= base && pos + bytes * n < tope){ 
        if (n != 0 && bytes != 0){ 
            for (i = 0; i < n; i++)
            {
                inicio = pos;
                printf("\n[%04X]: ", inicio);

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

                escribeIntMem(mv,pos,val, OP2);
                pos += bytes; 
            }   
        }
    }
}

void SYS3(maquinaV *mv){
    int pos, seg, base, tope, n, i = 0;
    char str[400];
    
    pos = mv -> regs[EDX];
    seg = (mv -> regs[EDX] >> 16) & 0xFFFF;
    base = mv -> tablaSeg[seg][0];
    tope = mv -> tablaSeg[seg][0] + mv -> tablaSeg[seg][1];
    n = mv -> regs[ECX];

    if (pos >= base && pos < tope){
        if (n > 0){
            scanf("%s", str);
            while (i < n && str[i] != '\0'){
                mv -> mem[pos] = str[i];
                pos++;

                if (pos >= tope){
                    mv -> error = 1;
                    return;
                }

                i++;
                
            }
        }

    } else {
        mv -> error = 1;
    }
}

void SYS4(maquinaV *mv){
    int pos, seg, base, tope;
   // int n;
    char car;

    
    pos = mv -> regs[EDX];
    seg = (mv -> regs[EDX] >> 16) & 0xFFFF;
    base = mv -> tablaSeg[seg][0];
    tope = mv -> tablaSeg[seg][0] + mv -> tablaSeg[seg][1];
    //n = mv -> regs[ECX];

    if (pos >= base && pos < tope){
        car = mv->mem[pos];
        while (pos < tope && car != '\0'){
            if (car == '\n')
                printf("\n");
            else if (car == '\t')
                printf("\t");
            else
                printf("%c", mv->mem[pos]);

            pos++;
            if(pos < tope)
                car = mv->mem[pos];
        }

        if(pos >= tope)
            mv->error = 1;

    } else {
        mv->error = 1;
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
        //case 0x7: clrscr(); break; //limpio pantalla
        case 0xF: SYSF(mv); break; //creo vmi
        default: mv -> error = 3; break;
    }
}

void creaVmi(maquinaV *mv){
    //unsigned char byteAct;
    char *textoHeader = "VMI25";
    //char letraAct;
    unsigned short int auxShort;
    FILE *archVmi = fopen("breakpoint.vmi","wb"); //Se tiene que llamar igual que el .vmx?

    if(archVmi == NULL){
        printf("No se pudo crear el archivo .vmi.");
        return;
    }

    /*HEADER*/

    fwrite(textoHeader, 1, strlen(textoHeader), archVmi); //Escribe VMI25 en el header

    unsigned char temp = 0x01; //Escribe la version, siempre es 1, se puede implementar en el tipo maquinaV con un campo version sino.
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

   /* printf("\nTabla de segmentos: \n");
    for(int i = 0; i < 8; i++){
        for (int j = 0; j <= 1; j++){
            printf("%d ",mv->tablaSeg[i][j]);
        }
        printf("\n");
    }
    */

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


void PUSH(maquinaV *mv, char topB){

    int aux;
        //printf("\nsp inicial en el push: %d", mv -> regs[SP]);


    if (mv->regs[SP] - 4 > mv->tablaSeg[posSS][0]) { // si hay lugar
        getValor(mv, OP2, &aux, topB);

        mv->regs[SP] -= 4;

        for (int i = 0; i < 4; i++) {
            unsigned char byte = (aux >> (8 * (3 - i))) & 0xFF;
            mv->mem[mv->regs[SP] + i] = byte;
        }
    } else
        mv->error = 4; // overflow

    /*printf("\nLos 4 bytes del SP en este push son %02X %02X %02X %02X",
       mv->mem[mv->regs[SP]],
       mv->mem[mv->regs[SP]+1],
       mv->mem[mv->regs[SP]+2],
       mv->mem[mv->regs[SP]+3]);
    */
    //printf(", sp final en el push: %d", mv -> regs[SP]);

}

void POP(maquinaV *mv, char topB){
    int aux;
    //printf("\nsp inicial en el POP: %d", mv -> regs[SP]);

    if (mv -> regs[SP] + 4 <= mv -> tablaSeg[posSS][0] + mv -> tablaSeg[posSS][1]){ // si la pila no esta vacia
        leeIntMem(mv, mv -> regs[SP], &aux, OP2);
        setValor(mv,OP2,aux,topB);
        mv -> regs[SP] += 4;
    } else 
        mv -> error = 5; //underflow
    //printf(", sp final en el pop: %d; El dato que saqeúe de la pila es %08X (hexa)", mv -> regs[SP],aux);

}


void CALL(maquinaV *mv){
    int retorno;
    char byte;

    //printf("\nsp inicial en el CALL: %d", mv -> regs[SP]);

    if (mv -> regs[SP] - 4 >= mv -> regs[SS]){ //HAY ESPACIO PARA AGREGAR
        
        mv -> regs[SP] -= 4; //RESTO AL SP
        
        //GUARDO LA DIRECCION DE RETORNO
        retorno = mv -> regs[IP] + 1;
        //printf("\n");
        for (int i = 0; i < 4; i++) {
            byte = (retorno >> (8 * (3 - i))) & 0xFF;
            mv->mem[mv->regs[SP] + i] = byte;
        }

        int nuevaip = mv -> regs[OP2] + mv -> regs[CS]; 

        if (nuevaip >= mv -> tablaSeg[posCS][0] && nuevaip < mv -> tablaSeg[posCS][0] + mv -> tablaSeg[posCS][1]){
            mv -> regs[IP] = (mv -> regs[IP] & 0xFFFF0000) + mv -> regs[OP2];
        
        } else {
            mv -> error = 1; //SEGMENTATION FAULT
        }

    } else {
        mv -> error = 4; //STACK OVERFLOW
    }

    //printf("; sp final en el CALL: %d", mv -> regs[SP]);

}

void RET(maquinaV *mv) {
    int retorno = 0;
    if (mv -> regs[SP] + 4 <= mv->tablaSeg[posSS][0] + mv->tablaSeg[posSS][1]){
        //CARGO RETORNO BYTE A BYTE
        for (int i = 0; i < 4; i++) 
            retorno = (retorno << 8) | mv->mem[mv->regs[SP] + i];

        //AVANZO EN LA PILA Y GUARDO EL IP
        mv -> regs[SP] += 4;
        mv -> regs[IP] = retorno;


    } else {
        mv -> error = 5; //STACK UNDERFLOW
    }

}
