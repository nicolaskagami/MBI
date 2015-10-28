//Nicolas Silveira Kagami 194636

#include "aag.h"

void aagRead(char * aagFileName)
{
    aagFile = fopen(aagFileName,"r");

    if(aagFile)
    {
        aagHeader();
        if(M==(A+I))
        {
            allocateMem(M,O);
            aagInputs();
            aagOutputs(); 
            aagSignals();
            aagInputsNames();
            aagOutputsNames();
            fgets(aag_name,MAX_LINE,aagFile);
            fgets(aag_name,MAX_LINE,aagFile);
        }
        else
        {
            printf("Number of Signals doesn't match\n");
        }
    }
}

void allocateMem(int number_signals, int number_outs)
{
    signals = malloc(2*(number_signals+1)*sizeof(SIGNAL));
    outputs = malloc(number_outs*sizeof(SIGNAL));
}

void aagHeader ()
{
    char head[MAX_LINE];
    fgets(head,MAX_LINE,aagFile);
    char * aux = &head[4];
    if(strcmp("aag",strtok(head," ")) == 0)
    {
        M = strtol(aux,&aux,10);
        I = strtol(aux,&aux,10);
        L = strtol(aux,&aux,10);
        O = strtol(aux,&aux,10);
        A = strtol(aux,NULL,10);
    }
    else
    {
        printf("Header Syntax Error\n");
    }
}

void aagInputs()
{ 
    char line[MAX_LINE];
    int i,aux;
    for(i=0;i<I;i++)
    {
        fgets(line,MAX_LINE,aagFile);
        aux = strtol(line,NULL,10);
    }
}

void aagOutputs()
{ 
    char line[MAX_LINE];
    int i;
    for(i=0;i<O;i++)
    {
        fgets(line,MAX_LINE,aagFile);
        outputs[i].ID =  strtol(line,NULL,10);
    }
}

void aagSignals()
{
    char line[MAX_LINE];
    char * aux;
    int i;
    int ID;
    for(i=0;i<A;i++)
    {
        fgets(line,MAX_LINE,aagFile);
        ID =  strtol(line,&aux,10);
        signals[ID].srcX = strtol(aux,&aux,10);
        signals[ID].srcY = strtol(aux,NULL,10);
    }
    
}

void aagInputsNames()
{
    char line[MAX_LINE];
    char aux[MAX_LINE];
    int i;
    for(i=1;i<=I;i++)
    {
        fgets(line,MAX_LINE,aagFile);
        strtok(line," ");
        strcpy(aux,strtok(NULL," \n"));
        signals[2*i].name = malloc(strlen(aux));
        strcpy(signals[2*i].name,aux);
    }
}

void aagOutputsNames()
{
    char line[MAX_LINE];
    char aux[MAX_LINE];
    int i;
    for(i=0;i<O;i++)
    {
        fgets(line,MAX_LINE,aagFile);
        strtok(line," ");
        strcpy(aux,strtok(NULL," \n"));
        outputs[i].name = malloc(strlen(aux));
        strcpy(outputs[i].name,aux);
    }
}

void aagStatsPrint()
{
    int i;
    printf("Name: %s",aag_name);
    printf("Stats:\n");
    printf("M: %d\n",M);
    printf("I: %d\n",I);
    printf("L: %d\n",L);
    printf("O: %d\n",O);
    printf("A: %d\n",A);
    aagSignalsPrint();
}

void aagSignalsPrint()
{
    int i;
    for(i=2;i<=M*2;i+=2)
    {
        printf("Signal %d ", i);
        if(signals[i].name)
        {
            printf("Input");
            if(signals[i].name)
            {
                printf(" %s\n",signals[i].name);
                printf("Position: %d %d\n", signals[i].srcX, signals[i].srcY);
                printf("Delay: %f", signals[i].delay);
            }
        }
        else
        {
            printf("Sources: %d %d",signals[i].srcX, signals[i].srcY);
        }
        printf("\n");
    }
    for(i=0;i<O;i++)
    {
        printf("Output %d", outputs[i].ID);
        if(outputs[i].name)
        {
            printf(" %s\n",outputs[i].name);
            printf("Position: %d %d\n",outputs[i].x, outputs[i].y);
            printf("Max Delay: %f\n", outputs[i].maxdelay);
        }
    }
}

int aagFindSignal(char * name)
{
    int i;
    
    for(i=1;i<=I;i++)
    {
        if(strcmp(signals[2*i].name,name) == 0)
        {
            return 2*i;
        }
    }
    return 0;
}

int aagFindOutput(char * name)
{
    int i;

    for(i=0;i<O;i++)
    {
        if(strcmp(outputs[i].name,name) == 0)
        {
            return i;
        }
    }
    return -1;
}
