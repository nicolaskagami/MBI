
#include "Topology.h"


//
Topology::Topology()
{
	vertices = NULL;
	edges = NULL;
	inputs = NULL;
	outputs = NULL;
}
int Topology::allocate_memory(unsigned v, unsigned e,unsigned I, unsigned O)
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
	inputs = (INPUT*) malloc(sizeof(INPUT)*I);
	outputs = (OUTPUT*) malloc(sizeof(OUTPUT)*O);

	num_inputs = I;
	num_outputs = O;
}
void Topology::preallocate(unsigned src,unsigned tgt,bool signal)
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
void Topology::indexify()
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
void Topology::add_edge(unsigned src,unsigned tgt,bool signal)
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
void Topology::set_position(unsigned vert,float x,float y)
{
    if(vert<num_vertices)
    {
        vertices[vert].position.x = x;
        vertices[vert].position.y = y;
    }
}




//PAAG
Paag::Paag(char * paagFileName)
{
	FILE * paagFile;
    paagFile = fopen(paagFileName,"r");
	topology = new Topology();
	char line[MAX_LINE];
	
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
            unsigned input;
            char * aux;
            unsigned i;
            float x,y;	
            topology->allocate_memory(M+1,A*2,I,O);
            //printf("M %d, I %d, L %d, O %d, A %d, X %d, Y %d\n",M,I,L,O,A,X,Y);
            //Inputs
            //Add input i as vertex number i/2 (vertex 0 being reserved for FALSE)
            //Position vertex according to X and Y

			//fgets(line,255,paagFile);puts(line);exit(1);
			
            for(i=0;i<2;i++)
			{
				fgets(line,MAX_LINE,paagFile);
                input = strtoul(line,&aux,10);
                aux = strtok(aux,"(,");
                aux = strtok(NULL,"(,");
                x = strtof(aux,NULL);
                aux = strtok(NULL,")");
                y = strtof(aux,NULL);
				
                topology->inputs[i].name[0] = '\0';
                topology->inputs[i].index = input/2;
                topology->inputs[i].delay = 0;
                topology->set_position(input/2,x,y);
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
                if(output > 2*topology->num_vertices -1)
                {
                    printf("PAAG Error: Floating Output\n");
                    exit(1);
                }
                aux = strtok(aux,"(,");
                aux = strtok(NULL,"(,");
                x = strtof(aux,NULL);
                aux = strtok(NULL,")");
				y = strtof(aux,NULL);

                topology->outputs[i].name[0] = '\0';
                topology->outputs[i].index = output/2;
                topology->outputs[i].max_delay = 0;
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

                topology->preallocate(srca/2,signal/2,srca%2);
                topology->preallocate(srcb/2,signal/2,srcb%2);
                //printf("Signal: %u %u %u (%u,%u)\n",signal,srca,srcb,x,y);
            }
            fsetpos(paagFile,&file_position);
            topology->indexify();
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

                topology->set_position(signal/2,x,y);
                topology->add_edge(srca/2,signal/2,srca%2);
                topology->add_edge(srcb/2,signal/2,srcb%2);
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
                                        strcpy(topology->inputs[index].name,name);
                                }
                                //printf("Input label: %u, %s\n",index,name);
                                break;
                    case 'o':
                                index = strtoul(&line[1],&aux,10);
                                name = strtok(aux," \r\n");
                                if(index<O)
                                {
                                        strcpy(topology->outputs[index].name,name);
                                }
                                //printf("Output label: %u, %s\n",index,name);
                                break;
                }
            }
            /*
            for(i=0;i<I;i++)
                    printf("IN %d %s: %d\n",i,inputs[i].name,inputs[i].index);
            for(i=0;i<O;i++)
                    printf("OUT %d %s: %d\n",i,outputs[i].name,outputs[i].index);
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
Paag::~Paag()
{
	delete topology;
}