//Nicolas Silveira Kagami 194636

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE 50

typedef struct 
{
    char * name;
    int srcX;
    int srcY;
    float delay;
} SIGNAL;

typedef struct 
{
    char * name;
    int ID;
    int x,y;
    float maxdelay;
} OUTPUT;

FILE * aagFile;
SIGNAL * signals;
OUTPUT * outputs;
char aag_name[MAX_LINE];
int M;
int I;
int L;
int O;
int A;

void allocateMem(int number_signals, int number_outs);
void aagRead(char * aagFileName);
void aagHeader();
void aagInputs();
void aagOutputs();
void aagSignals();
void aagInputsNames();
void aagOutputsNames();
void aagAnds();
void aagStatsPrint();
void aagSignalsPrint();
int aagFindSignal(char * name);
int aagFindOutput(char *name);
