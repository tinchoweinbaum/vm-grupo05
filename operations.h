//verificar la implementación de NZ, conviene llevar como parametros?

void MOV(char opB, char *opA, int *N, int *Z); //hacen falta los parametros N y Z? es MOV

void ADD(char opB, char *opA, int *N, int *Z);

void SUB(char opB, char *opA, int *N, int *Z);

void MUL(char opB, char *opA, int *N, int *Z);

void DIV(char opB, char *opA, int *N, int *Z);

void CMP(char opB, char opA, int *N, int *Z);

void SHL(char opB, char *opA); //los shifts afectan a NZ?

void SHR(char opB, char *opA);

void SAR(char opB, char *opA);

void AND(char opB, char *opA, int *N, int *Z);

void OR(char opB, char *opA, int *N, int *Z);

void XOR(char opB, char *opA, int *N, int *Z);

void SWAP(char *opB, char *opA); //NZ?

void LDL(char opB, char *opA);

void LDH(char opB, char *opA);

void RND(char opB, char *opA);



