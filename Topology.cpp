
#include "Topology.h"
//
Topology::Topology()
{
	vertices = NULL;
	edges = NULL;
	inputs = NULL;
	outputs = NULL;
}
Topology::~Topology()
{
    free(vertices);
    free(edges);
    free(inputs);
    free(outputs);
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
    for(unsigned i=0;i<num_edges;i++)
    {
        edges[i].path_delay = 0;
    }
	inputs = (INPUT*) malloc(sizeof(INPUT)*I);
	outputs = (OUTPUT*) malloc(sizeof(OUTPUT)*O);

	num_inputs = 0;
	num_outputs = 0;
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

//DEF
Def::Def(char * defFileName,Liberty *liberty)
{
	FILE * defFile;
    lib = liberty;
    defFile = fopen(defFileName,"r");
	topology = new Topology();
	char line[MAX_LINE];
    char * aux;
    if(defFile)
    {
        while(fgets(line,MAX_LINE,defFile)&&!feof(defFile))
        {
            aux = strtok(line," ;\n");
            if(aux)
            {
                if(strcmp("VERSION",aux)==0)
                {//Version
                    aux = strtok(NULL," ;\n") ;
                } else if(strcmp("DIVIDERCHAR", aux)==0)
                {//Divider Char
                    aux = strtok(NULL,"\" ;\n") ;
                } else if(strcmp("BUSBITCHARS", aux)==0)
                {//Bus Bit Chars
                    aux = strtok(NULL,"\" ;\n") ;
                    //printf("Bus Bit Chars: %s\n",aux);
                } else if(strcmp("DESIGN", aux)==0)
                {//Design
                    aux = strtok(NULL,"\" ;\n") ;
                    //printf("Design: %s\n",aux);
                    while(fgets(line,MAX_LINE,defFile)&&!feof(defFile))
                    {
                        aux = strtok(line,";\n");
                        if((aux)&&(strcmp("END DESIGN",line)==0))
                        {
                            aux = strtok(NULL,"\" ;\n") ;
                            //printf("Design End\n");
                            break;
                        } 
                        aux = strtok(line," ;\n");
                        if(aux)
                        {
                            if(strcmp("UNITS",aux)==0)
                            {
                            } else if(strcmp("PROPERTYDEFINITIONS",aux)==0)
                            {
                            } else if(strcmp("DIEAREA",aux)==0)
                            {//DIEAREA
                                aux = strtok(NULL," \n;()");
                                float u = strtof(aux,NULL);
                                aux = strtok(NULL," \n;()");
                                u = strtof(aux,NULL);
                                aux = strtok(NULL," \n;()");
                                u = strtof(aux,NULL);
                                //MAX X = u
                                aux = strtok(NULL," \n;()");
                                u = strtof(aux,NULL);
                                //MAX Y = u
                            } else if(strcmp("ROW",aux)==0)
                            {
                            } else if(strcmp("TRACKS",aux)==0)
                            {
                            } else if(strcmp("GCELLGRID",aux)==0)
                            {
                            } else if(strcmp("COMPONENTS",aux)==0)
                            {//COMPONENTS
                                aux = strtok(NULL," ;\n");
                                unsigned numComponents = strtoul(aux,NULL,10);
                                //printf("Num of Components: %u\n",numComponents);
                                components.reserve(numComponents);
                                while(fgets(line,MAX_LINE,defFile)&&!feof(defFile))
                                {
                                    aux = strtok(line,"\n");
                                    if((aux)&&(strcmp("END COMPONENTS",line)==0))
                                    {
                                        aux = strtok(NULL,"\" \n");
                                        break;
                                    }
                                    aux = strtok(line," \n");
                                    if(aux)
                                    {
                                        if(strcmp("-",aux)==0)
                                        {
                                            COMPONENT component;
                                            aux = strtok(NULL," \t\n");
                                            strcpy(component.name,aux);
                                            aux = strtok(NULL," \t\n");
                                            if(aux)
                                                component.cell = lib->findCell(aux);
                                            
                                            aux = strtok(NULL," ()\t\n");
                                            if(aux[0] == '+')
                                            {
                                                aux = strtok(NULL," ();\t\n");
                                                if(strcmp("PLACED",aux)==0)
                                                {
                                                    aux = strtok(NULL," ();\t\n");
                                                    component.position.x = strtof(aux,NULL);
                                                    aux = strtok(NULL," ();\t\n");
                                                    component.position.y = strtof(aux,NULL);
                                                }
                                                    
                                            }
                                            fgets(line,MAX_LINE,defFile);
                                            components.push_back(component);
                                        }
                                    }
                                } 

                            } else if(strcmp("PINS",aux)==0)
                            {//PINS
                                aux = strtok(NULL," ;\n");
                                unsigned numPins = strtoul(aux,NULL,10);
                                //printf("Num of Pins: %u\n",numPins);
                                pins.reserve(numPins);
                                while(fgets(line,MAX_LINE,defFile)&&!feof(defFile))
                                {
                                    aux = strtok(line,";\n");
                                    if((aux)&&(strcmp("END PINS",line)==0))
                                    {
                                        aux = strtok(NULL,"\" ;\n");
                                        break;
                                    }
                                    aux = strtok(line," ;\n");
                                    if(aux)
                                    {
                                        if(strcmp("-",aux)==0)
                                        {
                                            PIN pin;
                                            aux = strtok(NULL," \n");
                                            strcpy(pin.name,aux);

                                            aux = strtok(NULL," ();\t\n");
                                            aux = strtok(NULL," ();\t\n");
                                            aux = strtok(NULL," ();\t\n");
                                            aux = strtok(NULL," ();\t\n");
                                            if(strcmp("+",aux)==0)
                                            {
                                                aux = strtok(NULL," ();\t\n");
                                                if(strcmp("DIRECTION",aux)==0)
                                                {
                                                    aux = strtok(NULL," ();\t\n");
                                                    if(strcmp("INPUT",aux)==0)
                                                    {
                                                        pin.direction = true;
                                                    }
                                                    else
                                                    if(strcmp("OUTPUT",aux)==0)
                                                    {
                                                        pin.direction = false;
                                                    }
                                                }
                                            }
                                            fgets(line,MAX_LINE,defFile);
                                            fgets(line,MAX_LINE,defFile);
                                            aux = strtok(line," ();\t\n");
                                            if(strcmp("+",aux)==0)
                                            {
                                                aux = strtok(NULL," ();\t\n");
                                                if(strcmp("PLACED",aux)==0)
                                                {
                                                    aux = strtok(NULL," ();\t\n");
                                                    pin.position.x = strtof(aux,NULL);
                                                    aux = strtok(NULL," ();\t\n");
                                                    pin.position.y = strtof(aux,NULL);
                                                }
                                            }
                                            pins.push_back(pin);
                                        }
                                    }
                                } 

                           } else if(strcmp("NETS",aux)==0)
                            {//NETS
                                aux = strtok(NULL," ;\n");
                                unsigned numNets = strtoul(aux,NULL,10);
                                //printf("Num of Pins: %u\n",numNets);
                                nets.reserve(numNets);
                                while(fgets(line,MAX_LINE,defFile)&&!feof(defFile))
                                {
                                    aux = strtok(line,";\t\n");
                                    if((aux)&&(strcmp("END NETS",line)==0))
                                    {
                                        aux = strtok(NULL,"\" ;\n");
                                        break;
                                    }
                                    aux = strtok(line," ;\t\n");
                                    if(aux)
                                    {
                                        if(strcmp("-",aux)==0)
                                        {
                                            NET net;
                                            NET_POINT np;

                                            aux = strtok(NULL," ;\t\n");
                                            strcpy(net.name,aux);
                                            //First pin
                                            fgets(line,MAX_LINE,defFile);

                                            aux = strtok(line," ;)(\t\n");
                                            strcpy(np.name,aux);
                                            aux = strtok(NULL," ;)(\t\n");
                                            strcpy(np.pin,aux);

                                            while((strcmp("PIN",np.name)==0)&&(isPinOutput(np.pin)))
                                            {
                                                //np is only target, next one is source...
                                                net.targets.push_back(np);
                                                aux = strtok(NULL," ;)(\t\n");
                                                strcpy(np.name,aux);
                                                aux = strtok(NULL," ;)(\t\n");
                                                strcpy(np.pin,aux);
                                            }
                                            net.source = np;
                                            //Other pins
                                            aux = strtok(NULL," )(\t\n");
                                            do
                                            {
                                                while(aux)
                                                {
                                                    strcpy(np.name,aux);
                                                    aux = strtok(NULL," )(\t\n");
                                                    if(aux)
                                                    {
                                                        strcpy(np.pin,aux);
                                                        aux = strtok(NULL," )(\t\n");
                                                        net.targets.push_back(np);
                                                    }
                                                }
                                                fgets(line,MAX_LINE,defFile);
                                                aux = strtok(line," )(\t\n");
                                                
                                            } while((aux)&&(aux[0]!=';'));

                                            nets.push_back(net);
                                        }

                                    }
                                }
                            }
                        }
                    }
                }
            
            }
        }

        toTopology();
    }
    else
    {
        printf("Def Error: Can't open file: %s\n", defFileName);exit(1);
    }
    
    //print();

}
Def::~Def()
{
    delete topology;
}
void Def::print()
{
    printf("Def\n");
    printf("Components:\n");
    for(unsigned i = 0; i < components.size();i++)
    {
        printf("%s: (%.2f, %.2f) %s\n",components[i].name,components[i].position.x,components[i].position.y,components[i].cell->name);
        if(!components[i].ready)
        {
            printf("Def Error: Component not ready!\n");exit(1);
        }
    }
    printf("Pins:\n");
    for(unsigned i = 0; i < pins.size();i++)
    {
        printf("%s: (%.2f, %.2f) %s\n",pins[i].name,pins[i].position.x,pins[i].position.y,(pins[i].direction)?"Input":"Output");
    }
    printf("Nets:\n");
    for(unsigned i = 0; i < nets.size();i++)
    {
        printf("%s: (%s %s)\n",nets[i].name,nets[i].source.name,nets[i].source.pin);
        printf("\tTargets: ");
        for(unsigned j=0;j<nets[i].targets.size();j++)
            printf("(%s %s) ",nets[i].targets[j].name,nets[i].targets[j].pin);
        printf("\n");    getchar();
    }
}
bool Def::isPinOutput(char * pinName)
{
    for(unsigned i=0;i<pins.size();i++)
    {
        if(strcmp(pins[i].name,pinName)==0)
        {
            if(pins[i].direction)
                return false;
            else
                return true;
        }
    }
}
PIN * Def::findPin(char * pinName)
{
    for(unsigned i=0;i<pins.size();i++)
    {
        if(strcmp(pins[i].name,pinName)==0)
        {
            return&(pins[i]);
        }
    }
    return NULL;
}
void Def::toTopology()
{
    topology = new Topology();
    //Preparation
    numEdges = 0;
    numVertices = 0;
    numInputs = 0;
    numOutputs = 0;

    for(unsigned i = 0; i < pins.size();i++)
    {
        if(!pins[i].direction)
        {
            numEdges++;
            numOutputs++;
        }
        else
        {
            numInputs++; 
        }
        pins[i].vertex = numVertices++;
    }
    for(unsigned i = 0; i < components.size();i++)
    {
        if(strcmp(components[i].cell->name,"AND2_X1")==0)
        {
            //And
            components[i].vertex =  numVertices++;
            components[i].ready = true;
            numEdges+=2;
        }
        else
        {
            components[i].ready = false;
        }
        //else is INV, which does not contribute to vertex
    }

    //Determine the source component for each net point
    //printf("Determining Components\n");
    for(unsigned i = 0; i < nets.size();i++)
    {
        nets[i].source.component = NULL;
        if(strcmp(nets[i].source.name,"PIN"))
        {
            nets[i].source.component = findComponent(nets[i].source.name);
            if(nets[i].source.component == NULL)
            {
                printf("Def Error: Unmapped component: %s in net %s\n",nets[i].source.name, nets[i].name);exit(1);
            }
        }
        for(unsigned j=0;j<nets[i].targets.size();j++)
        {
            nets[i].targets[j].component = NULL;
            if(strcmp(nets[i].targets[j].name,"PIN"))
            {
                nets[i].targets[j].component = findComponent(nets[i].targets[j].name);
                if(nets[i].targets[j].component == NULL)
                {
                    printf("Def Error: Unmapped component: %s in net %s\n",nets[i].targets[j].name, nets[i].name);exit(1);
                }
            }
        }
    }
    //printf("Allocating Memory\n");
    topology->allocate_memory(numVertices,numEdges,numInputs,numOutputs);
    //printf("Toughest Part\n");
    bool netsReady;
    do
    { //This could be done much more efficiently
        netsReady = true;
        for(unsigned i = 0; i < nets.size();i++)
        {
            if(nets[i].source.component)
            {
                if(nets[i].source.component->ready)
                {
                    for(unsigned j=0;j<nets[i].targets.size();j++)
                    {
                        if(nets[i].targets[j].component)
                        {
                            if(nets[i].targets[j].component->ready)
                            {
                                //topology->preallocate(nets[i].source.component->vertex,nets[i].targets[j].component->vertex,nets[i].source.component->vertexSignal);
                            }
                            else
                            {
                                netsReady = false;
                                //now we know the source of this target, we can set its stuff up 
                                nets[i].targets[j].component->ready = true;
                                nets[i].targets[j].component->vertex = nets[i].source.component->vertex;
                                nets[i].targets[j].component->vertexSignal = !nets[i].source.component->vertexSignal;
                                //Push to a queue?
                            }
                        }
                    }
                }
                else
                {   //printf("Net: %u: (%s %s) with %u\n",i,nets[i].source.name,nets[i].source.pin,nets[i].targets.size());
                    netsReady = false;
                }
            }
            else
            {
                //It's a PIN source
                PIN * pin = findPin(nets[i].source.pin);
                if(pin == NULL)
                {
                    printf("Def Error: Pin %s not found\n",nets[i].source.pin);exit(1);
                }
                for(unsigned j=0;j<nets[i].targets.size();j++)
                {
                    if(nets[i].targets[j].component)
                    {
                        if(nets[i].targets[j].component->ready)
                        {
                            //maybe don't preallocate now, first let's make sure all nets are done
                            //topology->preallocate(nets[i].source.component->vertex,nets[i].targets[j].component->vertex,nets[i].source.component->vertexSignal);
                        }
                        else
                        {
                            netsReady = false;
                            //now we know the source of this target, we can set its stuff up 
                            nets[i].targets[j].component->ready = true;
                            nets[i].targets[j].component->vertex = pin->vertex;
                            nets[i].targets[j].component->vertexSignal = true;
                            //Push to a queue?
                        }
                    }
                }
            }
        }
    }while(!netsReady);
    //printf("Preallocation\n");
    //Preallocation
    for(unsigned i = 0; i < nets.size();i++)
    {
        if((nets[i].source.component)&&(nets[i].source.component->ready))
        {
            for(unsigned j=0;j<nets[i].targets.size();j++)
            {
                if(nets[i].targets[j].component)
                {
                    if(nets[i].targets[j].component->ready)
                    {
                        if(strcmp(nets[i].targets[j].component->cell->name,"AND2_X1")==0)
                            topology->preallocate(nets[i].source.component->vertex,nets[i].targets[j].component->vertex,nets[i].source.component->vertexSignal);
                    }
                }
                else
                {
                    PIN * outputPin = findPin(nets[i].targets[j].pin);
                    if(outputPin == NULL)
                    {
                        printf("Def Error: Pin %s not found\n",nets[i].source.pin);exit(1);
                    }
                    topology->preallocate(nets[i].source.component->vertex,outputPin->vertex,nets[i].source.component->vertexSignal);
                }
            }
        }
        else
        {
            //It's a PIN source
            PIN * pin = findPin(nets[i].source.pin);
            if(pin == NULL)
            {
                printf("Def Error: Pin %s not found\n",nets[i].source.pin);exit(1);
            }
            for(unsigned j=0;j<nets[i].targets.size();j++)
            {
                if((nets[i].targets[j].component)&&(nets[i].targets[j].component->ready))
                {
                    if(strcmp(nets[i].targets[j].component->cell->name,"AND2_X1")==0)
                        topology->preallocate(pin->vertex,nets[i].targets[j].component->vertex,false);
                }
                else
                {
                    PIN * outputPin = findPin(nets[i].targets[j].pin);
                    if(outputPin == NULL)
                    {
                        printf("Def Error: Pin %s not found\n",nets[i].source.pin);exit(1);
                    }
                    topology->preallocate(pin->vertex,outputPin->vertex,false);
                }
            }
        }
    }
    //Indexify
    //printf("Indexify\n");
    topology->indexify();
    //Add edges
    for(unsigned i = 0; i < nets.size();i++)
    {
        if((nets[i].source.component)&&(nets[i].source.component->ready))
        {
            for(unsigned j=0;j<nets[i].targets.size();j++)
                if((nets[i].targets[j].component)&&(nets[i].targets[j].component->ready))
                {
                    if(strcmp(nets[i].targets[j].component->cell->name,"AND2_X1")==0)
                        topology->add_edge(nets[i].source.component->vertex,nets[i].targets[j].component->vertex,nets[i].source.component->vertexSignal);
                }
                else
                {
                    PIN  * outputPin = findPin(nets[i].targets[j].pin);
                    if(outputPin == NULL)
                    {
                        printf("Def Error: Pin not found\n");exit(1);
                    }
                    topology->add_edge(nets[i].source.component->vertex,outputPin->vertex,nets[i].source.component->vertexSignal);
                }

        }
        else
        {
            //It's a PIN source
            PIN * pin = findPin(nets[i].source.pin);
            if(pin == NULL)
            {
                printf("Def Error: Pin %s not found\n",nets[i].source.pin);exit(1);
            }
            for(unsigned j=0;j<nets[i].targets.size();j++)
                if((nets[i].targets[j].component)&&(nets[i].targets[j].component->ready))
                {
                    if(strcmp(nets[i].targets[j].component->cell->name,"AND2_X1")==0)
                        topology->add_edge(pin->vertex,nets[i].targets[j].component->vertex,false);
                }
                else
                {
                    
                    PIN  * outputPin = findPin(nets[i].targets[j].pin);
                    if(outputPin == NULL)
                    {
                        printf("Def Error: Pin %s not found\n",nets[i].source.pin);exit(1);
                    }
                    printf("Def Warning: Connecting Pin %s to Pin %s\n",pin->name,outputPin->name);
                    topology->add_edge(pin->vertex,outputPin->vertex,false);
                }
        }
    }

    //Position Vertices
    for(unsigned i = 0; i < pins.size();i++)
    {
        topology->set_position(pins[i].vertex,pins[i].position.x,pins[i].position.y);
        if(!pins[i].direction)
        {
            OUTPUT o;
            o.index = pins[i].vertex;
            strcpy(o.name,pins[i].name);
            o.max_delay = 0;
            topology->outputs[topology->num_outputs++] = o;
        }
        else
        {
            INPUT in;
            in.index = pins[i].vertex;
            strcpy(in.name,pins[i].name);
            in.delay = 0;
            topology->inputs[topology->num_inputs++] = in;
        }
    }
    for(unsigned i = 0; i < components.size();i++)
    {
        if(strcmp(components[i].cell->name,"AND2_X1")==0)
        {
            topology->set_position(components[i].vertex,components[i].position.x,components[i].position.y);
        }
        //else is INV, which does not contribute to vertex
    }

}
COMPONENT * Def::findComponent(char * compName)
{
    for(unsigned j=0;j<components.size();j++)
    {
        if(strcmp(compName,components[j].name)==0)
        {
            return &components[j];
        }
    }
    return NULL;
}
