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

/***************TAMAÑOS**************/

#define MEM_SIZE 16384 //16384 bytes == 16 KiB
#define REG_SIZE 32 //32 registros en el procesador de la VM.
#define HEADER_SIZE 7 //el encabezado ocupa del byte 0 al 7 de un archivo

/***************MAQUINA VIRTUAL**************/
typedef struct maquinaV{
    unsigned char mem[MEM_SIZE]; //vector de memoria
    unsigned char regs[REG_SIZE]; //vector de registros
    int tablaSeg[2][2]; // tabla de segmentos: matriz de 2x2
    int N;
    int Z;
    int error;
} maquinaV;

void setValor(maquinaV *mv, int iOP, int OP, char top);

void getValor(maquinaV *mv,int iOP, int *OP, char top);

void readMem(maquinaV *mv);

void writeMem(maquinaV *mv);

void MOV(maquinaV *MV, char topA, char topB);

void ADD(maquinaV *MV, char topA, char topB);

void SUB(maquinaV *MV, char topA, char topB);

void MUL(maquinaV *MV, char topA, char topB);

void DIV(maquinaV *MV, char topA, char topB);

void CMP(maquinaV *MV, char topA, char topB);

void SHL(maquinaV *MV, char topA, char topB);

void SHR(maquinaV *MV, char topA, char topB);

void SAR(maquinaV *MV, char topA, char topB);

void AND(maquinaV *MV, char topA, char topB);

void OR(maquinaV *MV, char topA, char topB);

void XOR(maquinaV *MV, char topA, char topB);

void SWAP(maquinaV *MV, char topA, char topB);

void LDL(maquinaV *MV, char topA, char topB);

void LDH(maquinaV *MV, char topA, char topB);

void RND(maquinaV *MV, char topA, char topB);

void STOP(maquinaV *mv);

void SYS(maquinaV *mv);

void JZ(maquinaV *mv);

void JP(maquinaV *mv);

void JN(maquinaV *mv);

void JNZ(maquinaV *mv);

void JNP(maquinaV *mv);