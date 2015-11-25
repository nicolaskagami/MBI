
#include "MBI.h"

MBI::MBI(int argc,char ** argv)
{
	positional_input_source = 0;
	char * positional_input_file_name = NULL; //PAAG or DEF
	char * timing_input_file_name 	  = NULL; //SDC Input Output Timing
	char * cell_input_file_name 	  = NULL; //LIB Cell Library
    def = NULL;
    paag = NULL;
    lib = NULL;
    critical_path_delay = 0;
	for(unsigned i=1;i<argc;i++)
	{
		if(strcmp("--paag",argv[i])==0)
		{
			positional_input_file_name = argv[++i];
			positional_input_source = 1;
		}else
		if(strcmp("--def",argv[i])==0)
		{
			positional_input_file_name = argv[++i];
			positional_input_source = 2;
		}else
		if(strcmp("--sdc",argv[i])==0)
		{
			timing_input_file_name = argv[++i];
		}else
		if(strcmp("--lib",argv[i])==0)
		{
			cell_input_file_name = argv[++i];
		}
	}
	if((positional_input_file_name)&&(timing_input_file_name)&&(cell_input_file_name))
	{
		//printf("Selected positional input file:   \t%s\n",positional_input_file_name);
		//printf("Selected timing input file:       \t%s\n",timing_input_file_name);
		//printf("Selected cell library input file: \t%s\n",cell_input_file_name);
        parse_lib(cell_input_file_name);
        switch(positional_input_source)
		{
			case 1: parse_paag(positional_input_file_name);break;
			case 2: parse_def(positional_input_file_name);break;//def parser
			default:exit(1);
		}

		parse_sdc(timing_input_file_name);
		set_clock();
        
	}
	else
	{
		if(!positional_input_file_name)
		{
			printf("MBI Error: No positional input file\n");
			printf("\tDefine positional file as --def defFileName.def or --paag paagFileName.paag\n");
		}
		if(!timing_input_file_name)
		{
			printf("MBI Error: No timing input file\n");
			printf("\tDefine timing file as --sdc sdcFileName.sdc\n");
		}
		if(!cell_input_file_name)
		{
			printf("MBI Error: No Cell library input file\n");
			printf("\tDefine cell library file as --lib libFileName.lib\n");
		}
		exit(1);
	}
}
MBI::~MBI()
{
	for(unsigned i=0;i<num_vertices;i++)
    {
        if(vertices[i].inverter_tree)
            delete(vertices[i].inverter_tree);
    }
	
    //free(vertices);
    //free(edges);
    //free(inputs);
    //free(outputs);
    clean_sdc();
    clean_paag();
    clean_def();
    clean_lib();
}
void MBI::print()
{
    unsigned i;
    printf("Clocks:\n");
    for (std::list<CLOCK>::iterator it=clocks.begin(); it!=clocks.end(); ++it)
       printf("%s: Period: %f\n",it->name,it->period); 
    printf("----------------------------------------------------------------------\n");
    printf("Inputs\n");
    for(i=0;i<num_inputs;i++)
        printf("IN %d %s: %d Delay: %f\n",i,inputs[i].name,inputs[i].index,inputs[i].delay);
    printf("Outputs\n");
    for(i=0;i<num_outputs;i++)
            printf("OUT %d %s: %d Max Delay: %f\n",i,outputs[i].name,outputs[i].index,outputs[i].max_delay);
    printf("----------------------------------------------------------------------\n");
    printf("Vertices: %d\n",num_vertices);
    printf("Edges: %d\n",num_edges);
    for(i = 0;i<num_vertices;i++)
    {
        //if((vertices[i].positive_targets+vertices[i].negative_targets)>0)
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
			//if(vertices[i].inverter_tree)
				//vertices[i].inverter_tree->print();
        }
    }
    printf("Critical Delay: %.4f\n",critical_path_delay);
}
void MBI::print_configuration()
{
    printf("%d ",NET_ORDER);
    printf("%d ",CRIT_ALG);
    printf("%.2f ",CRITICAL_THRESHOLD);
    printf("%d ",NON_CRIT_ALG);
    printf("%d ",INV_POS);
}

//
void MBI::set_initial_delay()
{
    unsigned i, vert;
    float delay;
    for(i=0;i<num_inputs;i++)
    {
        vert = inputs[i].index;
        if(vertices[vert].pre_delay<inputs[i].delay)
            vertices[vert].pre_delay = inputs[i].delay;
    }
    for(i=0;i<num_outputs;i++)
    {
        vert = outputs[i].index;
        if(outputs[i].max_delay>current_clock.period)
        {
            printf("SDC Warning: Main Clock Period lower than an output's maximum delay\n");
            printf("\tClock: %s \tPeriod: %f -> %f from output %s\n",current_clock.name, current_clock.period,outputs[i].max_delay,outputs[i].name);
            current_clock.period = outputs[i].max_delay;
        }
        delay = current_clock.period - outputs[i].max_delay;
        if(vertices[vert].post_delay<delay)
            vertices[vert].post_delay = delay;
    }
}
void MBI::estimate_delay()
{
    unsigned i,vert;
    std::queue<unsigned> q;
    float delay;
    //Input Propagation
    for(i=0;i<num_inputs;i++)
        q.push(inputs[i].index);

    while(!q.empty())
    {
        vert = q.front();
        q.pop();
        //printf("Vert: %u\n",vert);
        unsigned ind,base,tgt;
        for(base = vertices[vert].pindex,ind=0;ind<(vertices[vert].positive_targets+vertices[vert].negative_targets);ind++)
        {
            tgt = edges[base+ind].target;
            delay = vertices[vert].pre_delay + nodal_delay + edges[base+ind].path_delay;
            if(vertices[tgt].pre_delay<delay)
            {
                vertices[tgt].pre_delay = delay;
                q.push(tgt);
            }
            
        }
    }
    //Output Propagation
    for(i=0;i<num_outputs;i++)
        q.push(outputs[i].index);

    while(!q.empty())
    {
        vert = q.front();
        q.pop();
        unsigned i,src;
        for(i=0;i<vertices[vert].num_srcs;i++)
        {
            src = vertices[vert].srcs[i];
            unsigned ind,base,tgt;
            delay = vertices[vert].post_delay + nodal_delay;
            for(base = vertices[src].pindex,ind=0;ind<(vertices[src].positive_targets+vertices[src].negative_targets);ind++)
                if(edges[base+ind].target == vert)
                {
                    delay += edges[base+ind].path_delay;break;
                }
            if(vertices[src].post_delay<delay)
            {
                q.push(src);
                vertices[src].post_delay = delay;
            }
            
        }
    }
}
void MBI::insert_buffers()
{
    //Ways to run over all vertices:
    //1: Iterate over the array
    //2: Propagate from inputs 
    //3: Propagate from outputs 
    std::queue<unsigned> q;
    unsigned vert;

    bool * expanded_vertices = (bool*) malloc(sizeof(bool)*num_vertices);

    for(unsigned i=0;i<num_vertices;i++)
        expanded_vertices[i] = false;

    switch(NET_ORDER)
    {
        default:
        case 0: //Simple iteration 
                for(vert=0;vert<num_vertices;vert++)
                    insert_buffer(vert);
                break;
        case 1: //Input -> Output propagation
                for(unsigned i=0;i<num_inputs;i++)
                    q.push(inputs[i].index);

                while(!q.empty())
                {
                    vert = q.front();
                    q.pop();
                    insert_buffer(vert);
                    expanded_vertices[vert] = true;
                    unsigned ind,base,tgt;
                    for(base = vertices[vert].pindex,ind=0;ind<(vertices[vert].positive_targets+vertices[vert].negative_targets);ind++)
                    {
                        tgt = edges[base+ind].target;
                        bool ready_to_expand = true;
                        for(unsigned sources = 0;sources<vertices[tgt].num_srcs;sources++)
                        {
                            if(expanded_vertices[vertices[tgt].srcs[sources]] == false) 
                                ready_to_expand = false;
                        }
                        if(ready_to_expand)
                            q.push(tgt);
                    }
                }
                break; 
        case 2: //Output -> Input propagation
                for(unsigned o=0;o<num_outputs;o++)
                {
                    //Here we sort out the outputs who supply other vertices
                    //This may not work for sequential circuits
                    unsigned ind,base,tgt;
                    bool ready_to_expand = true;
                    unsigned src = outputs[o].index;
                    for(base = vertices[src].pindex,ind=0;ind<(vertices[src].positive_targets+vertices[src].negative_targets);ind++)
                    {
                        tgt = edges[base+ind].target;
                        if(expanded_vertices[tgt] == false) 
                            ready_to_expand = false;
                    }
                    if(ready_to_expand)
                        q.push(src);
                }

                while(!q.empty())
                {
                    vert = q.front();
                    q.pop();
                    insert_buffer(vert);
                    expanded_vertices[vert] = true;
                    unsigned ind,base,tgt;
                    for(unsigned sources = 0;sources<vertices[vert].num_srcs;sources++)
                    {
                        bool ready_to_expand = true;
                        unsigned src = vertices[vert].srcs[sources];
                        for(base = vertices[src].pindex,ind=0;ind<(vertices[src].positive_targets+vertices[src].negative_targets);ind++)
                        {
                            tgt = edges[base+ind].target;
                            if(expanded_vertices[tgt] == false) 
                                ready_to_expand = false;
                        }
                        if(ready_to_expand)
                            q.push(src);
                    }
                }
                for(unsigned i=0;i<num_vertices;i++)
                {
                    if(!expanded_vertices[i]) //This should only bee needed in circularly dependant sequential circuits
                        insert_buffer(i);
                }
                break; 
    }
    free(expanded_vertices);
}
void MBI::insert_buffer(unsigned vert)
{
    //printf("Inserting Vert: %u\n",vert);
    if(min_height(vertices[vert].positive_targets,vertices[vert].negative_targets)>0)
    {
        sort_vert(vertices[vert]);
        vertices[vert].inverter_tree = new InverterTree(vertices[vert].positive_targets,vertices[vert].negative_targets,
														max_cell_fanout,max_inv_fanout,inv_delay,vertices[vert].position);
        //
        add_targets(vert);
		vertices[vert].inverter_tree->connect();
        vertices[vert].post_delay = vertices[vert].inverter_tree->maxDelay;
        calculate_path_delay(vert);
        //vertices[vert].inverter_tree->print_inverters();
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
void MBI::add_targets(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;

    //Paralellizable
	
    for( unsigned i=0;i<vertices[vert].positive_targets;i++)
    {
		vertices[vert].inverter_tree->add_positive_target(edges[pbase+i].target,true,vertices[edges[pbase+i].target].post_delay,vertices[edges[pbase+i].target].position);
    }
    
    for( unsigned i=0;i<vertices[vert].negative_targets;i++)
    {
		vertices[vert].inverter_tree->add_negative_target(edges[nbase+i].target,true,vertices[edges[nbase+i].target].post_delay,vertices[edges[nbase+i].target].position);
    }
}
void MBI::calculate_critical_delay()
{
 
    for(unsigned vert=0;vert<num_vertices;vert++)
        if((vertices[vert].pre_delay+vertices[vert].post_delay) > critical_path_delay)
        {
            critical_path_delay = (vertices[vert].pre_delay+vertices[vert].post_delay);
            if(critical_path_delay> 1)
            {
                printf("WTF vert: %u %.4f\n",vert,critical_path_delay);getchar();
            }
        }
}
void MBI::calculate_path_delay(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;

    //Paralellizable
    
    for( unsigned i=0;i<vertices[vert].positive_targets;i++)
    {
        edges[pbase+i].path_delay = vertices[vert].inverter_tree->positiveLevels[i]*inv_delay;
    }
    
    for( unsigned i=0;i<vertices[vert].negative_targets;i++)
    {
        edges[nbase+i].path_delay = vertices[vert].inverter_tree->negativeLevels[i]*inv_delay;
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
unsigned MBI::min_height(unsigned posConsumers,unsigned negConsumers)
{
    unsigned posAvailable = max_cell_fanout;
    unsigned negAvailable = 0;
    unsigned minHeight = 0;
    unsigned leavesAvailable, leaves;
    unsigned height1_branches;
    
    if((negConsumers == 0)&&(posConsumers<=max_cell_fanout)) 
    {
        return 0; 
    }
    do
    {
        minHeight++;
        if(minHeight%2)
        {
            //New layer is odd (negative)
            negAvailable=posAvailable*max_inv_fanout;
            leaves = negConsumers;
            height1_branches = posConsumers;
            leavesAvailable = negAvailable;
        }
        else
        {
            //New layer is even (positive)
            posAvailable=negAvailable*max_inv_fanout;
            leaves = posConsumers;
            height1_branches = negConsumers;
            leavesAvailable = posAvailable;
        }
        //printf("Height:%d Available P %d, N %d \n",minHeight,posAvailable,negAvailable);
    }
    while((leaves > leavesAvailable)||(height1_branches > ((leavesAvailable-leaves)/max_inv_fanout)));
	
    return minHeight;
}

//Parsers
//Zeroth vertex is true, negated is false
void MBI::parse_paag(char * paagFileName)
{
	paag = new Paag(paagFileName);

	vertices = paag->topology->vertices;
	num_vertices = paag->topology->num_vertices;
	
	edges = paag->topology->edges;
	num_edges = paag->topology->num_edges;
	
	inputs = paag->topology->inputs;
	num_inputs = paag->topology->num_inputs;
	outputs = paag->topology->outputs;
	num_outputs = paag->topology->num_outputs;
}
void MBI::clean_paag()
{
    if(paag)
		delete(paag);
}
void MBI::parse_def(char * defFileName)
{
	def = new Def(defFileName,lib);

	vertices = def->topology->vertices;
	num_vertices = def->topology->num_vertices;
	
	edges = def->topology->edges;
	num_edges = def->topology->num_edges;
	
	inputs = def->topology->inputs;
	num_inputs = def->topology->num_inputs;
	outputs = def->topology->outputs;
	num_outputs = def->topology->num_outputs;
}
void MBI::clean_def()
{
	if(def)
        delete(def);
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
                            for(i=0;i<num_inputs;i++)
                            {
                                if(strcmp(input_name,inputs[i].name)==0)
                                {
                                    inputs[i].delay = delay;
                                    //set_delay(inputs[i].index,delay);        
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
                            for(i=0;i<num_outputs;i++)
                            {
                                if(strcmp(output_name,outputs[i].name)==0)
                                {
                                    outputs[i].max_delay = delay;
                                    //set_delay(outputs[i].index,delay);        
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
void MBI::parse_lib(char * libFileName)
{
    lib = new Liberty(libFileName);
}
void MBI::clean_lib()
{
    if(lib)
        delete(lib);
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

int main(int argc, char ** argv)
{
    MBI nets(argc,argv);
	nets.max_inv_fanout = 2;
	nets.max_cell_fanout = 2;

    nets.set_nodal_delay("AND2_X1","INV_X1");
    nets.set_initial_delay();
    nets.estimate_delay();
    nets.calculate_critical_delay();

    nets.insert_buffers();
    nets.estimate_delay();
    nets.calculate_critical_delay();
    //nets.print();
    nets.print_configuration();
    printf("%.4f\n",nets.critical_path_delay);
    //nets.lib->print();
}
