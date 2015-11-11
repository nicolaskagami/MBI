
#include "MBI.h"


MBI::MBI(char *paagFileName,char * sdcFileName,char * libFileName)
{
    parse_paag(paagFileName);
    parse_sdc(sdcFileName);
    set_clock();
    lib = new Liberty(libFileName);
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
            vertices[i].num_srcs = 0;
            vertices[i].pre_delay = 0;
            vertices[i].post_delay = 0;
            vertices[i].positive_targets = 0;
            vertices[i].negative_targets = 0;
            vertices[i].position.x = -1;
            vertices[i].position.y = -1;
            vertices[i].num_positive_critical = 0;
            vertices[i].num_negative_critical = 0;
			vertices[i].inverter_tree = NULL;
    }
}
MBI::~MBI()
{
	for(unsigned i=0;i<num_vertices;i++)
    {
        if(vertices[i].inverter_tree)
            delete(vertices[i].inverter_tree);
    }
	
    free(vertices);
    free(edges);
    free(paag_inputs);
    free(paag_outputs);
    clean_sdc();
    clean_paag();
    if(lib)
        delete(lib);
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
    if((src>=num_vertices)||(tgt>=num_vertices))
    {
        printf("Invalid vertex\n");
        exit(1);
    }
    unsigned ind,srcs;
    srcs = vertices[tgt].num_srcs++;
    if(srcs<MAX_SOURCES)
    {
        vertices[tgt].srcs[srcs] = src;
    }
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
void MBI::set_position(unsigned vert,float x,float y)
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
    printf("----------------------------------------------------------------------\n");
    printf("Inputs\n");
    for(i=0;i<I;i++)
        printf("IN %d %s: %d Delay: %f\n",i,paag_inputs[i].name,paag_inputs[i].index,paag_inputs[i].delay);
    printf("Outputs\n");
    for(i=0;i<O;i++)
            printf("OUT %d %s: %d Max Delay: %f\n",i,paag_outputs[i].name,paag_outputs[i].index,paag_outputs[i].max_delay);
    printf("----------------------------------------------------------------------\n");
    printf("Vertices: %d\n",num_vertices);
    printf("Edges: %d\n",num_edges);
    for(i = 0;i<num_vertices;i++)
    {
        if((vertices[i].positive_targets+vertices[i].negative_targets)>0)
        {
            printf("Vert %d (%.2f,%.2f)\n",i,vertices[i].position.x,vertices[i].position.y);
            printf("Sources: ");
            for(unsigned srcs = 0; srcs < vertices[i].num_srcs;srcs++)
                printf("%u ",vertices[i].srcs[srcs]);
            printf("\nEstimated Delay: %f %f\n",vertices[i].pre_delay,vertices[i].post_delay);
            printf("Positive Consumers: ");
            for(unsigned b = vertices[i].pindex,j=0;j<vertices[i].positive_targets;j++)
            {
                if(j<vertices[i].num_positive_critical)
                    printf("[%d] ",edges[b+j].target);
                else
                    printf("%d ",edges[b+j].target);
            }
            printf("\nNegative Consumers: ");
            for(unsigned b = vertices[i].nindex,j=0;j<vertices[i].negative_targets;j++)
            {
                if(j<vertices[i].num_negative_critical)
                    printf("[%d] ",edges[b+j].target);
                else
                    printf("%d ",edges[b+j].target);
            }
            printf("\n");
			if(vertices[i].inverter_tree)
			vertices[i].inverter_tree->print();
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
        if(M==(A+I+L))
        {
            char line[MAX_LINE];
            char * aux;
            unsigned i;
            float x,y;
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
                x = strtof(aux,NULL);
                aux = strtok(NULL,")");
                y = strtof(aux,NULL);

                paag_inputs[i].name[0] = '\0';
                paag_inputs[i].index = input/2;
                paag_inputs[i].delay = 0;
				
                set_position(input/2,x,y);
                //Add vertex
                //printf("IN: %u (%.2f,%.2f)\n",input,x,y);
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
                if(output > 2*num_vertices -1)
                {
                    printf("PAAG Error: Floating Output\n");
                    exit(1);
                }
                aux = strtok(aux,"(,");
                aux = strtok(NULL,"(,");
                x = strtof(aux,NULL);
                aux = strtok(NULL,")");
				y = strtof(aux,NULL);

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
                x = strtof(aux,NULL);
                aux = strtok(NULL,")");
                y = strtof(aux,NULL);

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
                                name = strtok(aux," \r\n");
                                if(index<I)
                                {
                                        strcpy(paag_inputs[index].name,name);
                                }
                                //printf("Input label: %u, %s\n",index,name);
                                break;
                    case 'o':
                                index = strtoul(&line[1],&aux,10);
                                name = strtok(aux," \r\n");
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
          exit(1);
        }
    }

}
void MBI::clean_paag()
{
    
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
            aux = strtok(line," \t");
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
                clocks.push_back(clk); 
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
                for(aux=strtok(NULL," \t\n");aux!=NULL;aux=strtok(NULL," \t\n"))
                {
                    if(strcmp(aux,"-to")==0)
                    {
                        aux=strtok(NULL," \t\n");
                        if(aux)
                        {
                            unsigned i;
                            strcpy(output_name,aux);
                            for(i=0;i<O;i++)
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
void MBI::clean_sdc()
{
    
}
void MBI::set_clock()
{
    if(clocks.size()!=0)
    {
        current_clock = clocks.front();
    }
    else
    {
        printf("SDC Error: No Clocks Set\n");
    }
}
//
void MBI::estimate_delay()
{
    unsigned i,vert;
    std::queue<unsigned> q;
    float delay;
    //Input Propagation
    for(i=0;i<I;i++)
    {
        vert = paag_inputs[i].index;
        if(vertices[vert].pre_delay<paag_inputs[i].delay)
            vertices[vert].pre_delay = paag_inputs[i].delay;
        q.push(vert);
    }
    while(!q.empty())
    {
        vert = q.front();
        q.pop();
        unsigned ind,base,tgt;
        for(base = vertices[vert].pindex,ind=0;ind<(vertices[vert].positive_targets+vertices[vert].negative_targets);ind++)
        {
            tgt = edges[base+ind].target;
            delay = vertices[vert].pre_delay + nodal_delay;
            if(vertices[tgt].pre_delay<delay)
                vertices[tgt].pre_delay = delay;
            q.push(tgt);
        }
    }
    //Output Propagation
    for(i=0;i<O;i++)
    {
        vert = paag_outputs[i].index;
        if(paag_outputs[i].max_delay>current_clock.period)
        {
            printf("SDC Warning: Main Clock Period lower than an output's maximum delay\n");
            printf("\tClock: %s \tPeriod: %f -> %f from output %s\n",current_clock.name, current_clock.period,paag_outputs[i].max_delay,paag_outputs[i].name);
            current_clock.period = paag_outputs[i].max_delay;
        }
        delay = current_clock.period - paag_outputs[i].max_delay;
        if(vertices[vert].post_delay<delay)
            vertices[vert].post_delay = delay;
        q.push(vert);
    }
    while(!q.empty())
    {
        vert = q.front();
        q.pop();
        unsigned i,src;
        for(i=0;i<vertices[vert].num_srcs;i++)
        {
            src = vertices[vert].srcs[i];
            delay = vertices[vert].post_delay + nodal_delay;
            if(vertices[src].post_delay<delay)
                vertices[src].post_delay = delay;
            q.push(src);
        }
    }
}
void MBI::insert_buffers()
{
    //Ways to run over all vertices:
    //1: Iterate over the array
    //2: Propagate from inputs 
    //3: Propagate from outputs 
    //
    //sort the edges
    for(unsigned i=0;i<num_vertices;i++)
    {
        //Sort the targets
        //sort_vert(i);
        sort_vert(vertices[i]);
        //Determine the critical ones (a number of how many of the first positive and negative are critical)
        select_criticals(i);
        //Allocate
        vertices[i].inverter_tree = new InverterTree(vertices[i].positive_targets,vertices[i].negative_targets,max_cell_fanout,max_inv_fanout,inv_delay,vertices[i].position);
		//
		add_criticals(i);
		vertices[i].inverter_tree->expand();
		add_non_criticals(i);
		vertices[i].inverter_tree->connect_positioned_targets();
	}
}
void MBI::sort_vert(VERT vert)
{
    EDGE * paux;
    EDGE * naux;

    naux = (EDGE*) malloc(vert.negative_targets*sizeof(EDGE));
    paux = (EDGE*) malloc(vert.positive_targets*sizeof(EDGE));
    
    //Ready to parallelize
    mSort(&(edges[vert.pindex]),paux,0,vert.positive_targets-1);
    mSort(&(edges[vert.nindex]),naux,0,vert.negative_targets-1);
    
    free(naux);
    free(paux);
}
void MBI::sort_vert(unsigned vert)
{
    EDGE * paux;
    EDGE * naux;

    naux = (EDGE*) malloc(vertices[vert].negative_targets*sizeof(EDGE));
    paux = (EDGE*) malloc(vertices[vert].positive_targets*sizeof(EDGE));
    //Ready to parallelize
    mSort(&(edges[vertices[vert].pindex]),paux,0,vertices[vert].positive_targets-1);
    mSort(&(edges[vertices[vert].nindex]),naux,0,vertices[vert].negative_targets-1);
    
    free(naux);
    free(paux);
}
void MBI::select_criticals(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;
    float highest_delay = 0;
    
    if(vertices[vert].positive_targets!=0)
        highest_delay = vertices[edges[pbase].target].post_delay;
    if((vertices[vert].negative_targets!=0)&&(vertices[edges[nbase].target].post_delay>highest_delay))
        highest_delay = vertices[edges[nbase].target].post_delay;
        
    if(highest_delay==0)
        return;

    //Paralellizable
    vertices[vert].num_positive_critical = vertices[vert].positive_targets;//In case all are critical...
    for( unsigned i=0;i<vertices[vert].positive_targets;i++)
    {
        if(vertices[edges[pbase+i].target].post_delay<highest_delay*CRITICAL_THRESHOLD)
        {
            vertices[vert].num_positive_critical = i;
            break;
        }
    }
    
    vertices[vert].num_negative_critical = vertices[vert].negative_targets;//In case all are critical...
    for( unsigned i=0;i<vertices[vert].negative_targets;i++)
    {
        if(vertices[edges[nbase+i].target].post_delay<highest_delay*CRITICAL_THRESHOLD)
        {
            vertices[vert].num_negative_critical = i;
            break;
        }
    }
}
void MBI::add_criticals(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;

    //Paralellizable
	
    for( unsigned i=0;i<vertices[vert].num_positive_critical;i++)
    {
		vertices[vert].inverter_tree->add_critical_target(edges[pbase+i].target,false,vertices[edges[pbase+i].target].post_delay);
    }
    
    for( unsigned i=0;i<vertices[vert].num_negative_critical;i++)
    {
		vertices[vert].inverter_tree->add_critical_target(edges[nbase+i].target,true,vertices[edges[nbase+i].target].post_delay);
    }
}
void MBI::add_non_criticals(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;

    //Paralellizable
	
    for( unsigned i=vertices[vert].num_positive_critical;i<vertices[vert].positive_targets;i++)
    {
		vertices[vert].inverter_tree->add_non_critical_target(edges[pbase+i].target,false,vertices[edges[pbase+i].target].post_delay,vertices[edges[pbase+i].target].position);
    }
    
    for( unsigned i=vertices[vert].num_negative_critical;i<vertices[vert].negative_targets;i++)
    {
		vertices[vert].inverter_tree->add_non_critical_target(edges[nbase+i].target,true,vertices[edges[nbase+i].target].post_delay,vertices[edges[nbase+i].target].position);
    }
}
void MBI::set_nodal_delay(char * cellName,char * invName)
{
    for (std::list<CELL>::iterator it=lib->cells.begin(); it!=lib->cells.end(); ++it)
    {
        if(strcmp(cellName,it->name)==0)
        {
            nodal_delay = 0.005;
        }else
        if(strcmp(invName,it->name)==0)
        {
            inv_delay = 0.001;
        }
    }
}

void MBI::merge(EDGE * a,EDGE *aux,int left,int right,int rightEnd)
{
    int i,num,temp,leftEnd;
    leftEnd = right - 1;
    temp = left;
    num = rightEnd - left + 1;
    while((left<=leftEnd)&&(right<=rightEnd))
    {
        if(vertices[a[left].target].post_delay > vertices[a[right].target].post_delay)
           aux[temp++]=a[left++];
        else
            aux[temp++]=a[right++];
    }
    while(left <= leftEnd)
        aux[temp++]=a[left++];

    while(right <= rightEnd)
        aux[temp++]=a[right++];

    for(i=1;i<=num;i++,rightEnd--)
        a[rightEnd] = aux[rightEnd];
}
void MBI::mSort(EDGE * a,EDGE *aux,int left,int right)
{
    int center;
    if(left<right)
    {
        center=(left+right)/2;
        mSort(a,aux,left,center);
        mSort(a,aux,center+1,right);
        merge(a,aux,left,center+1,right);
    }
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
/*
void MBI::option1(unsigned vert)
{
    max_inv_fanout = 2;
    max_cell_fanout = 2;
    unsigned height = min_height(vertices[vert].positive_targets,vertices[vert].negative_targets,max_inv_fanout);
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
}*/

//Option 2:
//Start with all consumers in the min height and trade your way to optimality
//Init: All with height = min height to represent all
//Trades: Trade F positions in Nth layer for 1 position in (N-1)th layer
int main(int argc, char ** argv)
{
    MBI nets("./input/example4.paag","./input/example4.sdc","./input/simple-cells.lib");
	nets.max_inv_fanout = 2;
	nets.max_cell_fanout = 2;
    nets.set_nodal_delay("AND2_X1","INV_X1");
    //nets.print();
    nets.estimate_delay();
    nets.insert_buffers();
    nets.print();
    //nets.lib->print();
}
