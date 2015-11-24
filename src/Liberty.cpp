
#include "Liberty.h"

Liberty::Liberty(char * libFileName)
{
    default_wire_load = NULL;
    
    libFile = fopen(libFileName,"r");
    if(libFile)
    {
        char line_buffer[MAX_LINE];
        char * aux;
            while(fgets(line,MAX_LINE,libFile))
            {
                  aux = strtok(line," ");
                  if(strcmp(aux,"library") == 0)
                  {
                        aux = strtok(NULL,"(");
                        aux = strtok(aux,")");
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
									for(i=0,aux=strtok(NULL,"(),\"");aux!=NULL;i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
									}
									tlut.num_indices = i;
									tlut.ind1 = (float*) malloc(sizeof(float)*i);
									tlut.ind2 = (float*) malloc(sizeof(float)*i);
									strcpy(line,line_buffer);
									aux = strtok(line," \t");
									for(i=0,aux=strtok(NULL,"(),\"");aux!=NULL;i++,aux=strtok(NULL,",\")"))
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
									for(i=0,aux=strtok(NULL,"(),\"");(aux!=NULL)&&(i<tlut.num_indices);i++,aux=strtok(NULL,",\")"))
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
									for(i=0,aux=strtok(NULL,"()\",");aux!=NULL;i++,aux=strtok(NULL,",\")"))
									{
										if(aux[0]==';')
											break;
									}
									plut.num_indices = i;
									plut.ind1 = (float*) malloc(sizeof(float)*i);
									plut.ind2 = (float*) malloc(sizeof(float)*i);
									strcpy(line,line_buffer);
									aux = strtok(line," \t");
									for(i=0,aux=strtok(NULL,"(),\"");aux!=NULL;i++,aux=strtok(NULL,",\")"))
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
									for(i=0,aux=strtok(NULL,"(),\"");(aux!=NULL)&&(i<plut.num_indices);i++,aux=strtok(NULL,",\")"))
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
																ptp.cell_fall = get_timing_character();
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"cell_rise")==0)
																ptp.cell_rise = get_timing_character();
															
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"fall_transition")==0)
																ptp.fall_transition = get_timing_character();
															
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"rise_transition")==0)
																ptp.rise_transition = get_timing_character();
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
                                                                ppp.fall_power = get_power_character();
                                                                /*
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
                                                                */
															}
															fgets(line,MAX_LINE,libFile);
															aux = strtok(line," \t();:\"");
															if(strcmp(aux,"rise_power")==0)
															{
                                                                ppp.rise_power = get_power_character();
                                                                /*
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
                                                                */
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
TIMING_CHARACTER Liberty::get_timing_character()
{
	TIMING_CHARACTER tc;
	char line[MAX_LINE];
	char * aux;
	unsigned num_indices;
	unsigned i,j;
	aux = strtok(NULL," \t()\":;");
	for (std::list<TIMING_LUT>::iterator it=time_luts.begin(); it!=time_luts.end(); ++it)
	{
		if(strcmp(aux,it->name)==0)
		{
			tc.timing_lut = &(*it);
			break;
		}
	}

	num_indices = tc.timing_lut->num_indices;
	tc.index1 = (float*) malloc(sizeof(float)*num_indices);
	tc.index2 = (float*) malloc(sizeof(float)*num_indices);
	tc.values = (float*) malloc(sizeof(float)*num_indices*num_indices);
	
	fgets(line,MAX_LINE,libFile);
	aux = strtok(line," \t();:\"");
	if(strcmp(aux,"index_1")==0)
	{
		for(i=0,aux=strtok(NULL,"(),\"");(aux!=NULL)&&(i<num_indices);i++,aux=strtok(NULL,",\")"))
		{
			if(aux[0]==';')
				break;
			tc.index1[i] = strtof(aux,NULL);
		}
	}
	fgets(line,MAX_LINE,libFile);
	aux = strtok(line," \t();:\"");
	if(strcmp(aux,"index_2")==0)
	{
		for(i=0,aux=strtok(NULL,"(),\"");(aux!=NULL)&&(i<num_indices);i++,aux=strtok(NULL,",\")"))
		{
			if(aux[0]==';')
				break;
			tc.index2[i] = strtof(aux,NULL);
		}
	}
	fgets(line,MAX_LINE,libFile);
	aux = strtok(line," \t();:\"");
	if(strcmp(aux,"values")==0)
	{
		aux=strtok(NULL," \t(),\"");
		for(i=0;(aux!=NULL)&&(i<num_indices);i++)
		{
			for(j=0;(aux!=NULL)&&(j<num_indices);j++,aux=strtok(NULL," \t,\"()"))
			{
				if((aux[0]=='\\')||(aux[0]==';'))
					break;
				tc.values[i*num_indices+j] = strtof(aux,NULL);
			}
			fgets(line,MAX_LINE,libFile);
			aux = strtok(line," \t,();:\"");
			if((aux!=NULL)&&(aux[0]=='}'))
				return tc;
			
			if(aux[0]==';')
				break;
		}
	}
	fgets(line,MAX_LINE,libFile);
	return tc;
}
POWER_CHARACTER Liberty::get_power_character()
{
	POWER_CHARACTER pc;
	char line[MAX_LINE];
	char * aux;
	unsigned num_indices;
	unsigned i,j;
	aux = strtok(NULL," \t()\":;");
	for (std::list<POWER_LUT>::iterator it=power_luts.begin(); it!=power_luts.end(); ++it)
	{
		if(strcmp(aux,it->name)==0)
		{
			pc.power_lut = &(*it);
			break;
		}
	}

	num_indices = pc.power_lut->num_indices;
	pc.index1 = (float*) malloc(sizeof(float)*num_indices);
	pc.index2 = (float*) malloc(sizeof(float)*num_indices);
	pc.values = (float*) malloc(sizeof(float)*num_indices*num_indices);
	
	fgets(line,MAX_LINE,libFile);
	aux = strtok(line," \t();:\"");
	if(strcmp(aux,"index_1")==0)
	{
		for(i=0,aux=strtok(NULL,"(),\"");(aux!=NULL)&&(i<num_indices);i++,aux=strtok(NULL,",\")"))
		{
			if(aux[0]==';')
				break;
			pc.index1[i] = strtof(aux,NULL);
		}
	}
	fgets(line,MAX_LINE,libFile);
	aux = strtok(line," \t();:\"");
	if(strcmp(aux,"index_2")==0)
	{
		for(i=0,aux=strtok(NULL,"(),\"");(aux!=NULL)&&(i<num_indices);i++,aux=strtok(NULL,",\")"))
		{
			if(aux[0]==';')
				break;
			pc.index2[i] = strtof(aux,NULL);
		}
	}
	fgets(line,MAX_LINE,libFile);
	aux = strtok(line," \t();:\"");
	if(strcmp(aux,"values")==0)
	{
		aux=strtok(NULL," \t(),\"");
		for(i=0;(aux!=NULL)&&(i<num_indices);i++)
		{
			//printf("---");
			//puts(aux);
			for(j=0;(aux!=NULL)&&(j<num_indices);j++,aux=strtok(NULL," \t,\"()"))
			{
				//puts(aux);
				if((aux[0]=='\\')||(aux[0]==';'))
					break;
				pc.values[i*num_indices+j] = strtof(aux,NULL);
			}
			fgets(line,MAX_LINE,libFile);
			aux = strtok(line," \t,();:\"");
			if((aux!=NULL)&&(aux[0]=='}'))
				return pc;
			
			if(aux[0]==';')
				break;
		}
	}
	fgets(line,MAX_LINE,libFile);
	return pc;
}
	
void Liberty::print()
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
				printf("Cell Fall:\n");
				print_timing_character(&(tp->cell_fall));
				printf("Cell Rise:\n");
				print_timing_character(&(tp->cell_rise));
				printf("Fall Transition:\n");
				print_timing_character(&(tp->fall_transition));
				printf("Rise Transition:\n");
				print_timing_character(&(tp->rise_transition));
			}
			printf("\t\tPower Profiles:\n");
			for (std::list<PIN_POWER_PROFILE>::iterator pp=op->power_profiles.begin(); pp!=op->power_profiles.end(); ++pp)
			{
				printf("\t\t\t\t%s\n",pp->related_pin);
                printf("Fall Power:\n");
                print_power_character(&(pp->fall_power));
                printf("Rise Power:\n");
                print_power_character(&(pp->rise_power));
			}
        }
        
    }
        
}
CELL * Liberty::findCell(char * cellName)
{
	for (std::list<CELL>::iterator it=cells.begin(); it!=cells.end(); ++it)
	{
		if(strcmp(it->name,cellName)==0)
			return &(*it);
	}
	return NULL;
}
void Liberty::print_timing_character(TIMING_CHARACTER * tc)
{
	unsigned i,j;
	unsigned num_indices = tc->timing_lut->num_indices;
	printf("\tIndex1: ");
	for(i=0;i<num_indices;i++)
		printf("%.2f,",tc->index1[i]);
	printf("\n\tIndex2: ");
	for(i=0;i<num_indices;i++)
		printf("%.2f,",tc->index2[i]);
	printf("\n\tValues:");
	for(i=0;i<num_indices;i++)
	{
		printf("\n\t\t");
		for(j=0;j<num_indices;j++)
			printf("%.2f,",tc->values[i*num_indices+j]);
	}
	printf("\n");
}
void Liberty::print_power_character(POWER_CHARACTER * pc)
{
	unsigned i,j;
	unsigned num_indices = pc->power_lut->num_indices;
	printf("\tIndex1: ");
	for(i=0;i<num_indices;i++)
		printf("%.2f,",pc->index1[i]);
	printf("\n\tIndex2: ");
	for(i=0;i<num_indices;i++)
		printf("%.2f,",pc->index2[i]);
	printf("\n\tValues:");
	for(i=0;i<num_indices;i++)
	{
		printf("\n\t\t");
		for(j=0;j<num_indices;j++)
			printf("%.2f,",pc->values[i*num_indices+j]);
	}
	printf("\n");
}
	
Liberty::~Liberty()
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
	for (std::list<CELL>::iterator it=cells.begin(); it!=cells.end(); ++it)
	{
		for (std::list<OUTPUT_PIN>::iterator op=it->output_pins.begin(); op!=it->output_pins.end(); ++op)
		{
			for (std::list<PIN_TIMING_PROFILE>::iterator tp=op->timing_profiles.begin(); tp!=op->timing_profiles.end(); ++tp)
			{
				free(tp->cell_fall.index1);free(tp->cell_fall.index2);free(tp->cell_fall.values);
				free(tp->cell_rise.index1);free(tp->cell_rise.index2);free(tp->cell_rise.values);
				free(tp->fall_transition.index1);free(tp->fall_transition.index2);free(tp->fall_transition.values);
				free(tp->rise_transition.index1);free(tp->rise_transition.index2);free(tp->rise_transition.values);
			}
			for (std::list<PIN_POWER_PROFILE>::iterator pp=op->power_profiles.begin(); pp!=op->power_profiles.end(); ++pp)
			{
				free(pp->fall_power.index1);free(pp->fall_power.index2);free(pp->fall_power.values);
				free(pp->rise_power.index1);free(pp->rise_power.index2);free(pp->rise_power.values);
			}
		}
	}
}

