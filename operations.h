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
    unsigned char mem[MEM_SIZE]; //vector de memoria        //la memoria tiene que ser unsigned?????
    int regs[REG_SIZE]; //vector de registros
    int tablaSeg[2][2]; // tabla de segmentos: matriz de 2x2
    int error;
} maquinaV;

void actNZ(maquinaV *mv,int valor);

int NZ(maquinaV mv);

void escribeIntMem(maquinaV *mv,int dir,int valor);

void leeIntMem(maquinaV *mv,int dir, int *valor);

void setValor(maquinaV *mv, int iOP, int OP, char top);

void getValor(maquinaV *mv,int iOP, int *OP, char top);

void readMem(maquinaV *mv);

void writeMem(maquinaV *mv);

void MOV(maquinaV *mv, char tOpA, char tOpB);

void ADD(maquinaV *mv, char tOpA, char tOpB);

void SUB(maquinaV *mv, char tOpA, char tOpB);

void MUL(maquinaV *mv, char tOpA, char tOpB);

void DIV(maquinaV *mv, char tOpA, char tOpB);

void CMP(maquinaV *mv, char tOpA, char tOpB);

void SHL(maquinaV *mv, char tOpA, char tOpB);

void SHR(maquinaV *mv, char tOpA, char tOpB);

void SAR(maquinaV *mv, char tOpA, char tOpB);

void AND(maquinaV *mv, char tOpA, char tOpB);


void OR(maquinaV *mv, char tOpA, char tOpB);

void XOR(maquinaV *mv, char tOpA, char tOpB);

void SWAP(maquinaV *mv, char tOpA, char tOpB);

void LDL(maquinaV *mv, char tOpA, char tOpB);

void LDH(maquinaV *mv, char tOpA, char tOpB);

void RND(maquinaV *mv, char tOpA, char tOpB);

void NOT(maquinaV *mv, char tOpA);

void STOP(maquinaV *mv);

void SYS1(maquinaV *mv);

void SYS2(maquinaV *mv);

void SYS3(maquinaV *mv);

void SYS4(maquinaV *mv);

void menuSYS(maquinaV *mv);

void binario(int val);

void JMP(maquinaV *mv, int opB);

void JZ(maquinaV *mv, int opB);

void JP(maquinaV *mv, int opB);

void JN(maquinaV *mv, int opB);

void JNZ(maquinaV *mv, int opB);

void JNP(maquinaV *mv, int opB);

void JNN(maquinaV *mv, int opB);

void PUSH(maquinaV *mv, char topB);

void POP(maquinaV *mv, char topB);

void RET(maquinaV *mv);
