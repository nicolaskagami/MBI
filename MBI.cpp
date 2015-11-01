
#include "MBI.h"

bool Point::operator== (Point b)
{ 
    if((x == b.x)&&(y == b.y))
        return true; 
    else 
        return false;
}
MBI::MBI(char *paagFileName,char * sdcFileName)
{
    parse_paag(paagFileName);
    parse_sdc(sdcFileName);
}
MBI::MBI(unsigned v, unsigned e)
{
    edges = (EDGE*) malloc(sizeof(EDGE)*e);
    vertices = (VERT*) malloc(sizeof(VERT)*v);
    num_edges = e;
    num_vertices = v;
    if(edges==NULL || vertices==NULL)
    {
        std::cerr << "Memory Allocation Error\n" ;
        exit(1);
    }
    for(unsigned i=0;i<num_vertices;i++)
    {
            vertices[i].pre_delay = 0;
            vertices[i].post_delay = 0;
            vertices[i].positive_targets = 0;
            vertices[i].negative_targets = 0;
            vertices[i].position.x = -1;
            vertices[i].position.y = -1;
    }
}
int MBI::allocate_memory(unsigned v, unsigned e)
{

    edges = (EDGE*) malloc(sizeof(EDGE)*e);
    vertices = (VERT*) malloc(sizeof(VERT)*v);
    num_edges = e;
    num_vertices = v;
    if(edges==NULL || vertices==NULL)
    {
        std::cerr << "Memory Allocation Error\n" ;
        exit(1);
    }
    for(unsigned i=0;i<num_vertices;i++)
    {
            vertices[i].pre_delay = 0;
            vertices[i].post_delay = 0;
            vertices[i].positive_targets = 0;
            vertices[i].negative_targets = 0;
    }
}
MBI::~MBI()
{
    free(vertices);
    free(edges);
}
void MBI::preallocate(unsigned src,unsigned tgt,bool signal)
{
    if(src>=num_vertices)
    {
        printf("Invalid vertex\n");
        exit(1);
    }
    if(signal)
        vertices[src].negative_targets++;
    else
        vertices[src].positive_targets++;
}
void MBI::indexify()
{
    vertices[0].pindex = 0;
    vertices[0].nindex = vertices[0].positive_targets;
    for(unsigned i = 0; i<(num_vertices-1);i++)
    {
        vertices[i+1].pindex = vertices[i].nindex + vertices[i].negative_targets;
        vertices[i+1].nindex = vertices[i+1].pindex + vertices[i+1].positive_targets;

        vertices[i].positive_targets =0;
        vertices[i].negative_targets =0;
    }
    vertices[num_vertices-1].positive_targets=0; 
    vertices[num_vertices-1].negative_targets=0; 
}
void MBI::add_edge(unsigned src,unsigned tgt,bool signal)
{
    //printf("Adding Edge %d %d \n",src, tgt);
    if(src>=num_vertices)
    {
        printf("Invalid vertex\n");
        exit(1);
    }
    unsigned ind;
    if(signal)
    {
        ind = vertices[src].nindex + vertices[src].negative_targets;
        vertices[src].negative_targets++;
    }
    else
    {
        ind = vertices[src].pindex + vertices[src].positive_targets;
        vertices[src].positive_targets++;
    }
    if(ind>=num_edges)
    {
        printf("Invalid Edge : %u/%d\n",ind, num_edges);
        exit(1);
    }
    edges[ind].target = tgt;    
}
/*void MBI::set_delay(unsigned vert,float delay)
{
    if(vert>=num_vertices)
    {
        printf("Invalid vertex\n");
        exit(1);
    }
    vertices[vert].delay = delay;
}*/
void MBI::set_position(unsigned vert,unsigned x,unsigned y)
{
    if(vert<num_vertices)
    {
        vertices[vert].position.x = x;
        vertices[vert].position.y = y;
    }
}
void MBI::print()
{
    unsigned i;
    printf("Clocks:\n");
    for (std::list<CLOCK>::iterator it=clocks.begin(); it!=clocks.end(); ++it)
       printf("%s: Period: %f\n",it->name,it->period); 
    printf("Inputs\n");
    for(i=0;i<I;i++)
        printf("IN %d %s: %d Delay: %f\n",i,paag_inputs[i].name,paag_inputs[i].index,paag_inputs[i].delay);
    printf("Outputs\n");
    for(i=0;i<O;i++)
            printf("OUT %d %s: %d Max Delay: %f\n",i,paag_outputs[i].name,paag_outputs[i].index,paag_outputs[i].max_delay);
    printf("Vertices: %d\n",num_vertices);
    printf("Edges: %d\n",num_edges);
    for(i = 0;i<num_vertices;i++)
    {
        //if((vertices[i].positive_targets+vertices[i].negative_targets)>0)
        {
            printf("Vert %d (%u,%u)\n",i,vertices[i].position.x,vertices[i].position.y);
            printf("Estimated Delay: %f %f\n",vertices[i].pre_delay,vertices[i].post_delay);
            printf("Positive Consumers: ");
            for(unsigned b = vertices[i].pindex,j=0;j<vertices[i].positive_targets;j++)
                printf("%d ",edges[b+j].target);
            printf("\nNegative Consumers: ");
            for(unsigned b = vertices[i].nindex,j=0;j<vertices[i].negative_targets;j++)
                printf("%d ",edges[b+j].target);
            printf("\n");
        }
    }
}
//Parsers
//Zeroth vertex is true, negated is false
//Add a procedure to propagate delay from output to nearest vertex (possibly adding?
void MBI::parse_paag(char * paagFileName)
{
    FILE * paagFile;

    paagFile = fopen(paagFileName,"r");
    if(paagFile)
    {
        //Header
        {
            char head[MAX_LINE];
            fgets(head,MAX_LINE,paagFile);
            char * aux = &head[5];
            if(strcmp("paag",strtok(head," ")) == 0)
            {
                M = strtol(aux,&aux,10);
                I = strtol(aux,&aux,10);
                L = strtol(aux,&aux,10);
                O = strtol(aux,&aux,10);
                A = strtol(aux,&aux,10);
                X = strtol(aux,&aux,10);
                Y = strtol(aux,NULL,10);
            }
            else
            {
                printf("Header Syntax Error\n");
                exit(1);
            }
        }
        if(M==(A+I))
        {
            char line[MAX_LINE];
            char * aux;
            unsigned i;
            unsigned x,y;
            allocate_memory(M+1,A*2);
            //printf("M %d, I %d, L %d, O %d, A %d, X %d, Y %d\n",M,I,L,O,A,X,Y);
            //Inputs
            //Add input i as vertex number i/2 (vertex 0 being reserved for FALSE)
            //Position vertex according to X and Y
            paag_inputs = (INPUT*) malloc(sizeof(INPUT)*I);
            paag_outputs = (OUTPUT*) malloc(sizeof(OUTPUT)*O);
            for(i=0;i<I;i++)
            {
                unsigned input;
                fgets(line,MAX_LINE,paagFile);
                input = strtoul(line,&aux,10);
                aux = strtok(aux,"(,");
                aux = strtok(NULL,"(,");
                x = strtoul(aux,&aux,10);
                aux = strtok(NULL,")");
                y = strtoul (aux,&aux,10);

                paag_inputs[i].name[0] = '\0';
                paag_inputs[i].index = input/2;
                paag_inputs[i].delay = 0;
                set_position(input/2,x,y);
                //Add vertex
                //printf("IN: %u (%u,%u)\n",input,x,y);
            }
            //Latches
            for(i=0;i<L;i++)
            {
                fgets(line,MAX_LINE,paagFile);
            }
            //Outputs
            //Add to a list of outputs or to vertex (needs to be positioned!)
            for(i=0;i<O;i++)
            {
                unsigned output;
                fgets(line,MAX_LINE,paagFile);
                //outputs[i].ID =  strtol(line,NULL,10);
                output = strtoul(line,&aux,10);
                aux = strtok(aux,"(,");
                aux = strtok(NULL,"(,");
                x = strtoul(aux,&aux,10);
                aux = strtok(NULL,")");
                y = strtoul (aux,&aux,10);

                paag_outputs[i].name[0] = '\0';
                paag_outputs[i].index = output/2;
                paag_outputs[i].max_delay = 0;
                //printf("OUT: %d (%u,%u)\n",output,x,y);
            }
            //Signals
            fpos_t file_position;
            fgetpos(paagFile,&file_position);
            for(i=0;i<A;i++)
            {
                unsigned signal,srca,srcb;
                fgets(line,MAX_LINE,paagFile);
                signal = strtoul(line,&aux,10);
                srca = strtoul(aux,&aux,10);
                srcb = strtoul(aux,&aux,10);

                preallocate(srca/2,signal/2,srca%2);
                preallocate(srcb/2,signal/2,srcb%2);
                //printf("Signal: %u %u %u (%u,%u)\n",signal,srca,srcb,x,y);
            }
            fsetpos(paagFile,&file_position);
            indexify();
            for(i=0;i<A;i++)
            {
                unsigned signal,srca,srcb;
                fgets(line,MAX_LINE,paagFile);
                signal = strtoul(line,&aux,10);
                srca = strtoul(aux,&aux,10);
                srcb = strtoul(aux,&aux,10);
                aux = strtok(aux,"(,");
                aux = strtok(NULL,"(,");
                x = strtoul(aux,&aux,10);
                aux = strtok(NULL,")");
                y = strtoul (aux,&aux,10);

                set_position(signal/2,x,y);
                add_edge(srca/2,signal/2,srca%2);
                add_edge(srcb/2,signal/2,srcb%2);
                //printf("Signal: %u %u %u (%u,%u)\n",signal,srca,srcb,x,y);
            }

            while(fgets(line,MAX_LINE,paagFile))
            {
                char * name;
                unsigned index;
                if(line[0] == 'c')   
                    break;
                switch(line[0])
                {
                    case 'i':
                                index = strtoul(&line[1],&aux,10);
                                name = strtok(aux," \n");
                                if(index<I)
                                {
                                        strcpy(paag_inputs[index].name,name);
                                }
                                //printf("Input label: %u, %s\n",index,name);
                                break;
                    case 'o':
                                index = strtoul(&line[1],&aux,10);
                                name = strtok(aux," \n");
                                if(index<O)
                                {
                                        strcpy(paag_outputs[index].name,name);
                                }
                                //printf("Output label: %u, %s\n",index,name);
                                break;
                }
            }
            /*
            for(i=0;i<I;i++)
                    printf("IN %d %s: %d\n",i,paag_inputs[i].name,paag_inputs[i].index);
            for(i=0;i<O;i++)
                    printf("OUT %d %s: %d\n",i,paag_outputs[i].name,paag_outputs[i].index);
                    */
            fclose(paagFile);
        }
        else
        {
            printf("Number of Signals doesn't match\n");
        }
    }

}
void MBI::parse_sdc(char * sdcFileName)
{
    FILE * sdcFile;
    
    sdcFile = fopen(sdcFileName,"r");
    if(sdcFile)
    {
        char line[MAX_LINE];
        char * aux;
        //Header
        while(fgets(line,MAX_LINE,sdcFile))
        {
            aux = strtok(line," ");
            if(strcmp(aux,"create_clock") == 0)
            {
                CLOCK clk;
                for(aux=strtok(NULL," \n");aux!=NULL;aux=strtok(NULL," \n"))
                {
                    if(strcmp(aux,"-period")==0)
                    {
                        aux=strtok(NULL," \n");
                        if(aux)
                        {
                            clk.period = strtof(aux,NULL);
                            //printf("Period: %f\n",clk.period);
                        }
                    }
                    if(strcmp(aux,"-name")==0)
                    {
                        aux=strtok(NULL," \n");
                        if(aux)
                        {
                            strcpy(clk.name,aux);
                            //printf("NAME: %s\n",clk.name);
                        }
                    }
                }
                     clocks.push_front(clk); 
            }
            else
            if(strcmp(aux,"set_input_delay") == 0)
            {
                float delay;
                bool delay_parsed = false;
                char input_name[MAX_LINE];
                for(aux=strtok(NULL," \n");aux!=NULL;aux=strtok(NULL," \n"))
                {
                    if(strcmp(aux,"-clock")==0)
                    {
                        aux=strtok(NULL," \n");
                        if(aux)
                        {
                            //printf("For clock: %s\n",aux);
                        }
                    } else
                    if(delay_parsed==false)
                    {
                        delay = strtof(aux,&aux);
                        delay_parsed = true;
                    } else {
                        if(aux)
                        {
                            unsigned i;
                            strcpy(input_name,aux);
                            for(i=0;i<I;i++)
                            {
                                if(strcmp(input_name,paag_inputs[i].name)==0)
				{
				    paag_inputs[i].delay = delay;
                                    //set_delay(paag_inputs[i].index,delay);        
				}
                            }
                        }
                    }                        
                }
            }
            else
            if(strcmp(aux,"set_max_delay") == 0)
            {
                float delay;
                bool delay_parsed = false;
                char output_name[MAX_LINE];
                for(aux=strtok(NULL," \n");aux!=NULL;aux=strtok(NULL," \n"))
                {
                    if(strcmp(aux,"-to")==0)
                    {
                        aux=strtok(NULL," \n");
                        if(aux)
                        {
                            unsigned i;
                            strcpy(output_name,aux);
                            for(i=0;i<I;i++)
                            {
                                if(strcmp(output_name,paag_outputs[i].name)==0)
				{
				    paag_outputs[i].max_delay = delay;
                                    //set_delay(paag_outputs[i].index,delay);        
				}
                            }
                        }
                    } else
                    if(delay_parsed==false)
                    {
                        delay = strtof(aux,&aux);
                        delay_parsed = true;
                    }
                }
            }
        }
        fclose(sdcFile);
    }

}
void MBI::estimate_delay()
{
    unsigned i,vert;
    for(i=0;i<I;i++)
    {
        vert = paag_inputs[i].index;
        if(vertices[vert].pre_delay<paag_inputs[i].delay)
            vertices[vert].pre_delay = paag_inputs[i].delay;
    }
    for(i=0;i<O;i++)
    {
        vert = paag_outputs[i].index;
        if(vertices[vert].post_delay<paag_outputs[i].max_delay)
            vertices[vert].post_delay = paag_outputs[i].max_delay;
    }
}

unsigned minHeight(unsigned posConsumers,unsigned negConsumers,unsigned fanout)
{
    unsigned posAvailable = fanout;
    unsigned negAvailable = 0;
    unsigned height = 0;
    if((negConsumers == 0)&&(posConsumers<=fanout)) 
    {
        return height; 
    }
    while((negConsumers<negAvailable)&&(posConsumers<posAvailable)) 
    {
        height++;
        if(height%2)
        {
            //New layer is even (positive)
            posConsumers=negAvailable*fanout;
        }
        else
        {
            //New layer is odd (negative)
            negConsumers=posAvailable*fanout;
        }
    }
    return height;
}

//Fanout limitation algorithms:
//Option 1:
//Add delays one by one, maintain optimality (possibly ordered high to low)
//Init: All with height 0 (invalid)
//Insertion:  
//  If there is space in current layer, 
//      add it there
//  else
//      add it in the next layer
void MBI::option1(unsigned vert)
{
    max_inv_fanout = 2;
    max_cell_fanout = 2;
    unsigned height = minHeight(vertices[vert].positive_targets,vertices[vert].negative_targets,max_inv_fanout);
    unsigned maximum_delay = 0;
    LEVEL * levels = (LEVEL*) malloc(sizeof(LEVEL)*height);
    for(unsigned i=1;i<height;i++)
    {
            levels[i].vacant=0;
            levels[i].inv_taken=0;
            levels[i].signal_taken=0;
    }
    levels[0].vacant = max_cell_fanout;
    unsigned base = vertices[vert].pindex;
    for(unsigned i=0;i<vertices[vert].positive_targets+vertices[vert].negative_targets;i++)
    {
        //Add each target
        for(unsigned h=0;h<height;h++)
        {
            if(levels[h].vacant>0)
            {
                edges[base+i].level = h;
                levels[h].vacant--;
                break;
            }
        }        
    }
}

//Option 2:
//Start with all consumers in the min height and trade your way to optimality
//Init: All with height = min height to represent all
//Trades: Trade F positions in Nth layer for 1 position in (N-1)th layer

int main(int argc, char ** argv)
{
    MBI nets("./input/example.paag","./input/example.sdc");
    nets.unit_delay = 0.1;
    nets.estimate_delay();
    nets.print();
    /*
    MBI nets(10,10);
    nets.preallocate(0,1,0);
    nets.preallocate(0,2,1);
    nets.preallocate(0,3,0);
    nets.preallocate(0,4,1);
    nets.preallocate(0,5,0);
    nets.preallocate(0,6,1);
    nets.preallocate(0,7,0);
    nets.preallocate(0,8,1);
    nets.preallocate(0,9,1);
    nets.indexify();
    nets.add_edge(0,1,0); 
    nets.add_edge(0,2,1);
    nets.add_edge(0,3,0);
    nets.add_edge(0,4,1);
    nets.add_edge(0,5,0);
    nets.add_edge(0,6,1);
    nets.add_edge(0,7,0);
    nets.add_edge(0,8,1);
    nets.add_edge(0,9,1);
    nets.set_delay(1,1);
    nets.set_delay(2,2);
    nets.set_delay(3,3);
    nets.set_delay(4,4);
    nets.set_delay(5,5);
    nets.set_delay(6,6);
    nets.set_delay(7,7);
    nets.set_delay(8,8);
    nets.set_delay(9,9);
    nets.print();
    */
}
