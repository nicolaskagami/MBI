
#include "MBI.h"

bool Point::operator== (Point b)
{ 
    if((x == b.x)&&(y == b.y))
        return true; 
    else 
        return false;
}
MBI::MBI(char *paagFileName,char * sdcFileName,char * libFileName)
{
    parse_paag(paagFileName);
    parse_sdc(sdcFileName);
    parse_lib(libFileName);
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
            vertices[i].num_srcs = 0;
            vertices[i].pre_delay = 0;
            vertices[i].post_delay = -1;
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
            vertices[i].num_srcs = 0;
            vertices[i].pre_delay = 0;
            vertices[i].post_delay = -1;
            vertices[i].positive_targets = 0;
            vertices[i].negative_targets = 0;
            vertices[i].position.x = -1;
            vertices[i].position.y = -1;
    }
}
MBI::~MBI()
{
    free(vertices);
    free(edges);
    free(paag_inputs);
    free(paag_outputs);
    clean_lib();
    clean_sdc();
    clean_paag();
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
          printf("Sources: ");
          for(unsigned srcs = 0; srcs < vertices[i].num_srcs;srcs++)
                printf("%u ",vertices[i].srcs[srcs]);
            printf("\nEstimated Delay: %f %f\n",vertices[i].pre_delay,vertices[i].post_delay);
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
        if(M==(A+I+L))
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
void MBI::clean_sdc()
{
	
}
void MBI::parse_lib(char * libFileName)
{
    FILE * libFile;
    default_wire_load = NULL;
    
    libFile = fopen(libFileName,"r");
    if(libFile)
    {
        char line[MAX_LINE];
        char line_buffer[MAX_LINE];
        char * aux;
        //Header
            while(fgets(line,MAX_LINE,libFile))
            {
                  aux = strtok(line," ");
                  if(strcmp(aux,"library") == 0)
                  {
                        aux = strtok(NULL,"(");
                        aux = strtok(aux,")");
                        puts(aux);
                        while(fgets(line,MAX_LINE,libFile))
                        {
                            if(line[0] == '}')
                            {
								break;
							}
							aux = strtok(line," \t(");
							if(strcmp(aux,"lu_table_template") == 0) // TIMING LUT
							{
								aux = strtok(NULL,"(");
								aux = strtok(aux,")");
								TIMING_LUT tlut;
								strcpy(tlut.name,aux);

								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"variable_1") == 0)
								{
									aux = strtok(NULL," :;");
									strcpy(tlut.var1,aux);
								}
								
								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"variable_2") == 0)
								{
									aux = strtok(NULL," :;");
									strcpy(tlut.var2,aux);
								}
								
								fgets(line,MAX_LINE,libFile);		
								unsigned i;
								strcpy(line_buffer,line);
								aux = strtok(line," \t");
								if(strcmp(aux,"index_1") == 0)
								{									
									for(i=0,aux=strtok(NULL,"()\"");aux!=NULL;i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
									}
									tlut.num_indices = i;
									tlut.ind1 = (float*) malloc(sizeof(float)*i);
									tlut.ind2 = (float*) malloc(sizeof(float)*i);
									strcpy(line,line_buffer);
									aux = strtok(line," \t");
									for(i=0,aux=strtok(NULL,"()\"");aux!=NULL;i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
										tlut.ind1[i] = strtof(aux,NULL);
									}
								}
								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"index_2") == 0)
								{
									for(i=0,aux=strtok(NULL,"()\"");(aux!=NULL)&&(i<tlut.num_indices);i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
										tlut.ind2[i] = strtof(aux,NULL);
									}
									if(i!=tlut.num_indices)
									{
										printf("Inconsistent number of indices in Timing Lut: %s\n",tlut.name);
										exit(1);
									}
								}
								time_luts.push_back(tlut);
								/*
								puts(tlut.name);
								puts(tlut.var1);
								for(i=0;i<tlut.num_indices;i++)
									printf("%f, ",tlut.ind1[i]);
								puts(tlut.var2);
								
								for(i=0;i<tlut.num_indices;i++)
									printf("%f, ",tlut.ind2[i]);*/
								
							} else
							if(strcmp(aux,"power_lut_template") == 0) // POWER LUT
							{
								POWER_LUT plut;
								
								aux = strtok(NULL,"(");
								aux = strtok(aux,")");
								strcpy(plut.name,aux);

								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"variable_1") == 0)
								{
									aux = strtok(NULL," :;");
									strcpy(plut.var1,aux);
								}
								
								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"variable_2") == 0)
								{
									aux = strtok(NULL," :;");
									strcpy(plut.var2,aux);
								}
								
								fgets(line,MAX_LINE,libFile);		
								char line_buffer[MAX_LINE];
								unsigned i;
								strcpy(line_buffer,line);
								aux = strtok(line," \t");
								if(strcmp(aux,"index_1") == 0)
								{									
									for(i=0,aux=strtok(NULL,"()\"");aux!=NULL;i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
									}
									plut.num_indices = i;
									plut.ind1 = (float*) malloc(sizeof(float)*i);
									plut.ind2 = (float*) malloc(sizeof(float)*i);
									strcpy(line,line_buffer);
									aux = strtok(line," \t");
									for(i=0,aux=strtok(NULL,"()\"");aux!=NULL;i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
										plut.ind1[i] = strtof(aux,NULL);
									}
								}
								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"index_2") == 0)
								{
									for(i=0,aux=strtok(NULL,"()\"");(aux!=NULL)&&(i<plut.num_indices);i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
										plut.ind2[i] = strtof(aux,NULL);
									}
									if(i!=plut.num_indices)
									{
										printf("Inconsistent number of indices in Power Lut: %s\n",plut.name);
										exit(1);
									}
								}
								power_luts.push_back(plut);
							} else
							if(strcmp(aux,"voltage_map") == 0) //name (a,b)
							{
								VOLT_MAP volt_map;
								aux = strtok(NULL,"(,");
								strcpy(volt_map.name,aux);
								aux = strtok(NULL,",)");
								volt_map.voltage = strtof(aux,NULL);
								voltage_maps.push_back(volt_map);
							} else
							if(strcmp(aux,"time_unit") == 0) // name : "value";
							{
								aux = strtok(NULL,":");
								aux = strtok(NULL,"\"");
								aux = strtok(NULL,";\"");
								//puts(aux);
							} else
							if(strcmp(aux,"nom_process") == 0) //name : value;
							{
								aux = strtok(NULL," :");
								aux = strtok(NULL," ;\"");
								nom_process = strtof(aux,NULL);
								//puts(aux);
							} else
							if(strcmp(aux,"nom_temperature") == 0) //name : value;
							{
								aux = strtok(NULL," :");
								aux = strtok(NULL," ;\"");
								nom_temperature = strtof(aux,NULL);
								//puts(aux);
							} else
							if(strcmp(aux,"nom_voltage") == 0) //name : value;
							{
								aux = strtok(NULL," :");
								aux = strtok(NULL," ;\"");
								nom_voltage = strtof(aux,NULL);
								//puts(aux);
							} else
							if(strcmp(aux,"technology") == 0) //name (value);
							{
								aux = strtok(NULL,"()");
								//puts(aux);
							} else
							if(strcmp(aux,"wire_load") == 0) //name (value);
							{
								fpos_t position;
								unsigned i;
								WIRE_LOAD wl;
								aux = strtok(NULL,"(\")");
								strcpy(wl.name,aux);
								//Capacitance
								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"capacitance") == 0)
								{
									aux = strtok(NULL," :;");
									wl.capacitance = strtof(aux,NULL);
									//printf("%f\n",wl.capacitance);
								}
								//Resistance
								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"resistance") == 0)
								{
									aux = strtok(NULL," :;");
									wl.resistance = strtof(aux,NULL);
									//printf("%f\n",wl.resistance);
								}
								//Slope
								fgets(line,MAX_LINE,libFile);		
								aux = strtok(line," \t");
								if(strcmp(aux,"slope") == 0)
								{
									aux = strtok(NULL," :;");
									wl.slope = strtof(aux,NULL);
									//printf("%f\n",wl.slope);
								}
								fgetpos(libFile,&position);
								i=0;
								while(fgets(line,MAX_LINE,libFile))
								{
									aux = strtok(line," \t()");
									if(strcmp(aux,"fanout_length"))
										break;
									i++;
								}
								fsetpos(libFile,&position);
								wl.fanout = (unsigned *) malloc(sizeof(unsigned)*i);
								wl.length = (float *) malloc(sizeof(float)*i);
								wl.num_indices = i;
								i=0;
								while(fgets(line,MAX_LINE,libFile))
								{
									aux = strtok(line," \t()");
									if(strcmp(aux,"fanout_length"))
										break;
									aux = strtok(NULL," \t(),");
									wl.fanout[i] = strtoul(aux,NULL,10);
									aux = strtok(NULL," \t(),");
									wl.length[i] = strtof(aux,NULL);
									i++;
								}
								wire_loads.push_back(wl);
							} else
							if(strcmp(aux,"default_wire_load") == 0) 
                            {
								aux = strtok(NULL,"\"");
								aux = strtok(NULL,"\"");

                                for (std::list<WIRE_LOAD>::iterator it=wire_loads.begin(); it!=wire_loads.end(); ++it)
                                {
                                   if(strcmp(aux,it->name)==0) 
                                   {
                                       default_wire_load = &(*it);
                                       break;
                                   }
                                }    
                                if(default_wire_load == NULL)
                                {
                                    printf("LIB Warning: Suggested default wire load not found\n");
                                }
                            }else
							if(strcmp(aux,"cell") == 0)
							{
								CELL cell;
								aux = strtok(NULL,"(");
								aux = strtok(aux,")");
								
								strcpy(cell.name,aux);
								while(fgets(line,MAX_LINE,libFile))
								{
									aux = strtok(line," \t(");
                                    if(aux)
                                    {
                                        if(strcmp(aux,"drive_strength") == 0)
                                        {
                                            aux = strtok(NULL," :");
                                            aux = strtok(NULL," ;\"");
                                            cell.drive_strength = strtoul(aux,NULL,10);
                                            //printf("%u\n",cell.drive_strength);
                                        }else
                                        if(strcmp(aux,"area") == 0)
                                        {
                                            aux = strtok(NULL," :");
                                            aux = strtok(NULL," ;\"");
                                            cell.area = strtof(aux,NULL);
                                            //printf("%f\n",cell.area);
                                        }else
                                        if(strcmp(aux,"pg_pin") == 0)
                                        {
                                            PG_PIN pgpin;
                                            aux = strtok(NULL," \t()");
                                            strcpy(pgpin.name,aux);

                                            fgets(line,MAX_LINE,libFile);
                                            aux = strtok(line," \t:;");
                                            if(strcmp(aux,"voltage_name")==0)
                                            {
                                                aux = strtok(NULL," \t:;");
                                                for (std::list<VOLT_MAP>::iterator it=voltage_maps.begin(); it!=voltage_maps.end(); ++it)
                                                {
                                                    if(strcmp(aux,it->name)==0)
                                                    {
                                                       pgpin.volt_map = &(*it); 
                                                    }
                                                }
                                            }
                                            fgets(line,MAX_LINE,libFile);
                                            aux = strtok(line," \t:;");
                                            if(strcmp(aux,"pg_type")==0)
											{
												aux = strtok(NULL," \t:;");
                                                strcpy(pgpin.type,aux);
                                            }
											fgets(line,MAX_LINE,libFile);
                                            cell.pg_pins.push_back(pgpin);
                                        }else
                                        if(strcmp(aux,"cell_leakage_power") == 0)
                                        {
                                            aux = strtok(NULL," :");
                                            aux = strtok(NULL," ;\"");
                                            cell.cell_leakage_power = strtof(aux,NULL);
                                            //printf("%f\n",cell.cell_leakage_power);
                                        }else
                                        if(strcmp(aux,"leakage_power") == 0)
                                        {
                                            LEAKAGE_POWER lp;

                                            fgets(line,MAX_LINE,libFile);
                                            strcpy(line_buffer,line);
                                            aux = strtok(line," \t:\";");
                                            if(strcmp(aux,"when")==0)
                                            {
                                                aux = strtok(line_buffer,"\"");
                                                aux = strtok(NULL,"\"");
                                                strcpy(lp.when,aux);
                                            }

                                            fgets(line,MAX_LINE,libFile);
                                            aux = strtok(line," \t:;");
                                            if(strcmp(aux,"value")==0)
                                            {
                                                aux = strtok(NULL," \t:;\"");
                                                lp.value = strtof(aux,NULL);
                                            }
                                            fgets(line,MAX_LINE,libFile);
                                            cell.leakage_powers.push_back(lp);
                                        }else
										if(strcmp(aux,"pin") == 0)
                                        {
											char name[MAX_NAME];
											aux = strtok(NULL," \t()");
											strcpy(name,aux);
											fgets(line,MAX_LINE,libFile);
											fgets(line,MAX_LINE,libFile);
											aux = strtok(line," \t:;");
											if(strcmp(aux,"direction")==0)
											{
												aux = strtok(NULL," \t:;");
												if(strcmp(aux,"input")==0)
												{
													int i;
													INPUT_PIN input_pin;
													input_pin.related_ground_pin = -1;
													input_pin.related_power_pin = -1;
													strcpy(input_pin.name,name);
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:\"");
													if(strcmp(aux,"related_power_pin")==0)
													{
														aux = strtok(NULL," \t;:\"");
														i=0;
														for (std::list<PG_PIN>::iterator pg=cell.pg_pins.begin(); pg!=cell.pg_pins.end(); ++pg,i++)
														{
															if(strcmp(pg->name,aux)==0)
																input_pin.related_power_pin = i;
															
														}
														//printf("cell: %d\n",&cell.input_pins);
														//printf("power pin:%d\n",input_pin.related_power_pin);
													}
													
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:\"");
													if(strcmp(aux,"related_ground_pin")==0)
													{
														aux = strtok(NULL," \t;:\"");
														i=0;
														for (std::list<PG_PIN>::iterator pg=cell.pg_pins.begin(); pg!=cell.pg_pins.end(); ++pg,i++)
														{
															if(strcmp(pg->name,aux)==0)
																input_pin.related_ground_pin = i;
														}
													}
													
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:");
													if(strcmp(aux,"capacitance")==0)
													{
														aux = strtok(NULL," \t;:");
														input_pin.capacitance = strtof(aux,NULL);
													}
													
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:");
													if(strcmp(aux,"fall_capacitance")==0)
													{
														aux = strtok(NULL," \t;:");
														input_pin.fall_capacitance = strtof(aux,NULL);
													}
													
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:");
													if(strcmp(aux,"rise_capacitance")==0)
													{
														aux = strtok(NULL," \t;:");
														input_pin.rise_capacitance = strtof(aux,NULL);
													}
													fgets(line,MAX_LINE,libFile);
													cell.input_pins.push_back(input_pin);
												}else
												if(strcmp(aux,"output")==0)
												{
													OUTPUT_PIN output_pin;
													int i;
													output_pin.related_ground_pin = -1;
													output_pin.related_power_pin = -1;
													strcpy(output_pin.name,name);
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:\"");
													if(strcmp(aux,"related_power_pin")==0)
													{
														aux = strtok(NULL," \t;:\"");
														i=0;
														for (std::list<PG_PIN>::iterator pg=cell.pg_pins.begin(); pg!=cell.pg_pins.end(); ++pg,i++)
														{
															if(strcmp(pg->name,aux)==0)
																output_pin.related_power_pin = i;
															
														}
													}
													
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:\"");
													if(strcmp(aux,"related_ground_pin")==0)
													{
														aux = strtok(NULL," \t;:\"");
														i=0;
														for (std::list<PG_PIN>::iterator pg=cell.pg_pins.begin(); pg!=cell.pg_pins.end(); ++pg,i++)
														{
															if(strcmp(pg->name,aux)==0)
																output_pin.related_ground_pin = i;
														}
													}
													
													fgets(line,MAX_LINE,libFile);
													aux = strtok(line," \t;:");
													if(strcmp(aux,"max_capacitance")==0)
													{
														aux = strtok(NULL," \t;:");
														output_pin.max_capacitance = strtof(aux,NULL);
													}
													
													fgets(line,MAX_LINE,libFile);
													strcpy(line_buffer,line);
													aux = strtok(line," \t;:\"");
                                                
													if(strcmp(aux,"function")==0)
													{
														aux = strtok(line_buffer,"\"");
														aux = strtok(NULL,"\"");
														strcpy(output_pin.function,aux);
													}
													
													//fgets(line,MAX_LINE,libFile);
													
													while(fgets(line,MAX_LINE,libFile))
													{
														aux = strtok(line," \t;:\"");
														if(strcmp(aux,"timing") == 0)
														{
															PIN_TIMING_PROFILE ptp;
															fgets(line,MAX_LINE,libFile);
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"related_pin")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ptp.related_pin,aux);
															}
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t()\":;");
															if(strcmp(aux,"timing_sense")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ptp.timing_sense,aux);
															}
															fgets(line,MAX_LINE,libFile);
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"cell_fall")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ptp.cell_fall.timing_lut,aux);
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_1")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.cell_fall.index1 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_2")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.cell_fall.index2 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"values")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.cell_fall.values = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
															}
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"cell_rise")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ptp.cell_rise.timing_lut,aux);
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_1")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.cell_rise.index1 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_2")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.cell_rise.index2 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"values")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.cell_rise.values = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
															}
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"fall_transition")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ptp.fall_transition.timing_lut,aux);
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_1")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.fall_transition.index1 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_2")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.fall_transition.index2 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"values")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.fall_transition.values = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
															}
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"rise_transition")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ptp.rise_transition.timing_lut,aux);
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_1")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.rise_transition.index1 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_2")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.rise_transition.index2 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"values")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ptp.rise_transition.values = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
															}
															fgets(line,MAX_LINE,libFile);
															output_pin.timing_profiles.push_back(ptp);
														}else
														if(strcmp(aux,"internal_power") == 0)
														{
															PIN_POWER_PROFILE ppp;
															fgets(line,MAX_LINE,libFile);
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"related_pin")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ppp.related_pin,aux);
															}
															
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"fall_power")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ppp.fall_power.power_lut,aux);
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_1")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ppp.fall_power.index1 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_2")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ppp.fall_power.index2 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"values")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ppp.fall_power.values = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
															}
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"rise_power")==0)
															{
																aux = strtok(NULL," \t()\":;");
																strcpy(ppp.rise_power.power_lut,aux);
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_1")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ppp.rise_power.index1 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"index_2")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ppp.rise_power.index2 = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
																aux = strtok(line," \t();:\"");
																if(strcmp(aux,"values")==0)
																{
																	aux = strtok(NULL," \t();:\"");
																	ppp.rise_power.values = strtof(aux,NULL);
																}
																fgets(line,MAX_LINE,libFile);
															}
															output_pin.power_profiles.push_back(ppp);
														}else
														if(aux[0]=='}')
															break;
													}
													cell.output_pins.push_back(output_pin);
												}
											}
											
										}else
										if(aux[0]=='}')
                                            break;
                                    }

								}
                                cells.push_back(cell);
							}
                        }
                  }
            }
        fclose(libFile);
    }
}
void MBI::print_lib()
{
	unsigned i;
	printf("Voltage Maps:\n");
	for (std::list<VOLT_MAP>::iterator it=voltage_maps.begin(); it!=voltage_maps.end(); ++it)
	{
		printf("\t%s: %.2f\n",it->name,it->voltage);
	}
	
	printf("Wire Loads:\n");
	for (std::list<WIRE_LOAD>::iterator it=wire_loads.begin(); it!=wire_loads.end(); ++it)
	{
		printf("%s: \n",it->name);
		printf("\tCapacitance: %f\n",it->capacitance);
		printf("\tResistance: %f\n",it->resistance);
		printf("\tSlope: %f\n",it->slope);
		printf("\tFanout \tLength\n");
		for(i=0;i<it->num_indices;i++)
		{
			printf("\t%u \t%f\n",it->fanout[i],it->length[i]);
		}
	}

	printf("Time Luts:\n");
	for (std::list<TIMING_LUT>::iterator it=time_luts.begin(); it!=time_luts.end(); ++it)
	{
		printf("%s: \n",it->name);
		printf("\t%s: ",it->var1);
		for(i=0;i<it->num_indices;i++)
			printf("%f, ",it->ind1[i]);
		printf("\n\t%s: ",it->var2);
		for(i=0;i<it->num_indices;i++)
			printf("%f, ",it->ind2[i]);
		printf("\n");
	}

	printf("Power Luts:\n");
	for (std::list<POWER_LUT>::iterator it=power_luts.begin(); it!=power_luts.end(); ++it)
	{
		printf("%s: \n",it->name);
		printf("\t%s: ",it->var1);
		for(i=0;i<it->num_indices;i++)
			printf("%.2f, ",it->ind1[i]);
		printf("\n\t%s: ",it->var2);
		for(i=0;i<it->num_indices;i++)
			printf("%.2f, ",it->ind2[i]);
		printf("\n");
	}

    printf("Cells:\n");
	for (std::list<CELL>::iterator it=cells.begin(); it!=cells.end(); ++it)
    {
		printf("\t%s: \n",it->name);
        printf("\tDrive Strength: %u\n",it->drive_strength);
        printf("\tArea: %f\n",it->area);
		printf("\tPG_Pins\n");
        for (std::list<PG_PIN>::iterator pg=it->pg_pins.begin(); pg!=it->pg_pins.end(); ++pg)
		{
			printf("\t\t%s:\t%s\t%f\n",pg->name,pg->type,pg->volt_map->voltage);
		}
        printf("\tCell Leakage Power: %f\n",it->cell_leakage_power);
        printf("\t\tWhen\tValue\n");
        for (std::list<LEAKAGE_POWER>::iterator lp=it->leakage_powers.begin(); lp!=it->leakage_powers.end(); ++lp)
        {
            printf("\t\t%s\t%f\n",lp->when,lp->value);
        }
		printf("\t\tInput Pins:\n");
		for (std::list<INPUT_PIN>::iterator ip=it->input_pins.begin(); ip!=it->input_pins.end(); ++ip)
        {
			printf("\t\t%s\n",ip->name);
            printf("\t\t\tCapacitance: %.2f Rise: %.2f Fall: %.2f\t\n",ip->capacitance,ip->rise_capacitance,ip->fall_capacitance);
			printf("\t\t");
			if(ip->related_power_pin>=0)
				printf("\tPower Pin: %d",ip->related_power_pin);
			if(ip->related_ground_pin>=0)
				printf("\tGround Pin: %d",ip->related_ground_pin);
			printf("\n");
        }
		printf("\t\tOutput Pins:\n");
		for (std::list<OUTPUT_PIN>::iterator op=it->output_pins.begin(); op!=it->output_pins.end(); ++op)
        {
			printf("\t\t%s\n",op->name);
            printf("\t\t\tMax Capacitance: %.2f\n",op->max_capacitance);
			printf("\t\t\tFunction: %s\n",op->function);
			printf("\t\t");
			if(op->related_power_pin>=0)
				printf("\tPower Pin: %d",op->related_power_pin);
			if(op->related_ground_pin>=0)
				printf("\tGround Pin: %d",op->related_ground_pin);
			printf("\n");
			printf("\t\tTiming Profiles:\n");
			for (std::list<PIN_TIMING_PROFILE>::iterator tp=op->timing_profiles.begin(); tp!=op->timing_profiles.end(); ++tp)
			{
				printf("\t\t\t\t%s: %s\n",tp->related_pin,tp->timing_sense);
				printf("\t\t\t\tCell Fall: %s: %.2f %.2f %.2f\n",tp->cell_fall.timing_lut,tp->cell_fall.index1,tp->cell_fall.index2,tp->cell_fall.values);
				printf("\t\t\t\tCell Rise: %s: %.2f %.2f %.2f\n",tp->cell_rise.timing_lut,tp->cell_rise.index1,tp->cell_rise.index2,tp->cell_rise.values);
				printf("\t\t\t\tFall Transition: %s: %.2f %.2f %.2f\n",tp->fall_transition.timing_lut,tp->fall_transition.index1,tp->fall_transition.index2,tp->fall_transition.values);
				printf("\t\t\t\tRise Transition: %s: %.2f %.2f %.2f\n",tp->rise_transition.timing_lut,tp->rise_transition.index1,tp->rise_transition.index2,tp->rise_transition.values);
			}
			printf("\t\tPower Profiles:\n");
			for (std::list<PIN_POWER_PROFILE>::iterator pp=op->power_profiles.begin(); pp!=op->power_profiles.end(); ++pp)
			{
				printf("\t\t\t\t%s\n",pp->related_pin);
				printf("\t\t\t\tFall Power: %s: %.2f %.2f %.2f\n",pp->fall_power.power_lut,pp->fall_power.index1,pp->fall_power.index2,pp->fall_power.values);
				printf("\t\t\t\tRise Power: %s: %.2f %.2f %.2f\n",pp->rise_power.power_lut,pp->rise_power.index1,pp->rise_power.index2,pp->rise_power.values);
			}
        }
        
    }
        
}
void MBI::clean_lib()
{
	for (std::list<TIMING_LUT>::iterator it=time_luts.begin(); it!=time_luts.end(); ++it)
	{
		free(it->ind1);
		free(it->ind2);
	}
	for (std::list<POWER_LUT>::iterator it=power_luts.begin(); it!=power_luts.end(); ++it)
	{
		free(it->ind1);
		free(it->ind2);
	}
	for (std::list<WIRE_LOAD>::iterator it=wire_loads.begin(); it!=wire_loads.end(); ++it)
	{	
		free(it->fanout);
		free(it->length);
	}
}
void MBI::estimate_delay()
{
    unsigned i,vert;
    std::queue<unsigned> q;
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
            float delay = vertices[vert].pre_delay + unit_delay;
            if(vertices[tgt].pre_delay<delay)
                vertices[tgt].pre_delay = delay;
            q.push(tgt);
        }
    }
    for(i=0;i<O;i++)
    {
        vert = paag_outputs[i].index;
        if(vertices[vert].post_delay<paag_outputs[i].max_delay)
            vertices[vert].post_delay = paag_outputs[i].max_delay;
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
            float delay = vertices[vert].post_delay - unit_delay;// Minus if max delay, should be changed to plus if the value means the delay until the end 
            if((vertices[src].post_delay>delay)||(vertices[src].post_delay<0))
                vertices[src].post_delay = delay;
            q.push(src);
        }
    }
}
void MBI::insert_buffers()
{
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
    MBI nets("./input/example3.paag","./input/example.sdc","./input/simple-cells.lib");
    nets.unit_delay = 0.001;
    nets.estimate_delay();
    nets.insert_buffers();
	//nets.print_lib();
    //nets.print();
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
