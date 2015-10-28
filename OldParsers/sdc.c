//Nicolas Silveira Kagami

#include "sdc.h"

void sdcRead(char * sdcFileName)
{
    sdcFile = fopen(sdcFileName,"r");
    char line[MAX_LINE];
    char linecp[MAX_LINE];
    char * aux;
            
    if(sdcFile)
    {
        numberClocks = 0;
        while(!feof(sdcFile))
        {
            if(fgets(line,MAX_LINE,sdcFile))
            { 
                strcpy(linecp,line);
                aux = strtok(line," \n");
                if(aux)
                {
                    if(strcmp(aux,"create_clock") == 0)
                    {
                        sdcCreateClk(linecp);
                    }
                    if(strcmp(aux,"set_input_delay") == 0)
                    {
                        sdcSetInputDelay(linecp);
                    } 
                    if(strcmp(aux,"set_max_delay") == 0)
                    {
                        sdcSetMaxDelay(linecp);
                    }

                }
            }
        }
        sdcClksPrint();
    }
}

void sdcCreateClk(char * line)
{ 
    char * aux;
    char * aux2;

    aux2 = strtok(NULL," \n");
    aux2 = strtok(NULL," \n");
    clocks[numberClocks].period = strtof(aux2,NULL);
    aux2 = strtok(NULL," \n");
    aux2 = strtok(NULL," \n");
    strcpy(clocks[numberClocks].name, aux2);
    numberClocks++;
}

void sdcSetInputDelay(char * line)
{
    char * aux;
    char * aux2;
    float delay;
    int clock;
    int input;
    fpos_t position;
    
    aux2 = strtok(NULL," \n");
    delay = strtof(aux2,NULL);
    aux2 = strtok(NULL," \n");
    aux2 = strtok(NULL," \n");
    clock = sdcClkSearch(aux2);
    if(clock != -1)
    {
        aux2 = strtok(NULL," \n");
        input = aagFindSignal(aux2);
        if(input)
        {
            signals[input].delay = delay;
        }
    }
}

void sdcSetMaxDelay(char * line)
{
    char * aux;
    char * aux2;
    float delay;
    int output;
    fpos_t position;

    
    aux2 = strtok(NULL," \n");
    delay = strtof(aux2,NULL);
    aux2 = strtok(NULL," \n");
    aux2 = strtok(NULL," \n");
    output = aagFindOutput(aux2);
    if(output != -1)
    {
        outputs[output].maxdelay = delay;
        printf("B");
    }
}

void sdcClksPrint()
{
    int i;
    for(i=0;i<numberClocks;i++)
    {
        printf("Clock:");
        puts(clocks[i].name);
        printf("%f\n",clocks[i].period);
    }
}

int sdcClkSearch(char * name)
{
    int i;
    for(i=0;i<numberClocks;i++)
    {
        if(strcmp(name, clocks[i].name) == 0)
        {
            return i;
        }
    }
    return -1;
}
