#include "InverterTree.h"

InverterTree::InverterTree(unsigned posTargets,unsigned negTargets,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition)
{
    Debug = false;

	sourcePosition = srcPosition;
	inverterDelay = invDelay;
	maxDelay = 0;
	
	degree = maxInvFanout;
	maximumCellFanout = maxCellFanout;
	
	positiveTargets = (TARGET*) malloc(posTargets*sizeof(TARGET));
	negativeTargets = (TARGET*) malloc(negTargets*sizeof(TARGET));
	numNegativeTargets = 0;
	numPositiveTargets = 0;
	
	numPositiveCriticals = 0;
	numNegativeCriticals = 0;
	
	positiveLevels = (unsigned*) malloc(posTargets*sizeof(unsigned));
	negativeLevels = (unsigned*) malloc(negTargets*sizeof(unsigned));
	
    for(unsigned i =0;i<posTargets;i++)
    {
        positiveTargets[i].target = 0; 
        positiveTargets[i].post_delay = 0; 
        positiveTargets[i].isVertex = true; 
        positiveLevels[i] = 0; 
    }
    for(unsigned i =0;i<negTargets;i++)
    {
        negativeTargets[i].target = 0; 
        negativeTargets[i].post_delay = 0; 
        negativeTargets[i].isVertex = true; 
        negativeLevels[i] = 0; 
    }
	positiveConsumersLeft = posTargets;
	negativeConsumersLeft = negTargets;
	
	currentPositiveTargets = (TARGET*) malloc((posTargets+negTargets)*sizeof(TARGET));//Careful here, it may be bigger than positiveConsumers...
	currentNegativeTargets = (TARGET*) malloc((posTargets+negTargets)*sizeof(TARGET));
	numCurrentPositiveTargets = 0;
	numCurrentNegativeTargets = 0;
	
	height = min_height(posTargets,negTargets)+1;

	levels = (LEVEL*) malloc(height*sizeof(LEVEL));
    levels[0].vacant = maxCellFanout - 1;
    levels[0].inv_taken = 1;
    levels[0].signal_taken = 0;
	for(unsigned i=1;i<height;i++)
	{
		levels[i].vacant = degree - 1;
		levels[i].inv_taken = 1;
		levels[i].signal_taken = 0;
	}
}
InverterTree::~InverterTree()
{
	for (std::vector<INVERTER>::iterator it = positionedInverters.begin() ; it != positionedInverters.end(); ++it)
	{
		if(it->targets)
			free(it->targets);
	}
	free(levels);
    free(currentPositiveTargets);
    free(currentNegativeTargets);
	free(positiveTargets);
	free(negativeTargets);
	free(positiveLevels);
	free(negativeLevels);
}

//Main Functions
void InverterTree::connect()
{
	//printf("place_criticals\n");
	place_criticals();
	//printf("Expand\n");
	expand();
	//printf("Prune\n");
	prune();
	//printf("place_non_criticals\n");
	place_non_criticals();
	//printf("Connect\n");
	connect_targets();
	//printf("Determining max delay\n");
	determine_max_delay();
    //print();
}
void InverterTree::expand()
{
	//Expand until there is space for the rest, as determined by the targetLeft variables
	int positiveAvailable =0;
	unsigned positiveCriticalsCovered = 0;
	int negativeAvailable =0;
	unsigned negativeCriticalsCovered = 0;
	unsigned expanded;
	unsigned i;
	
	//Expand to provide for non criticals
	for(i=0;(positiveAvailable<positiveConsumersLeft)||(negativeAvailable<negativeConsumersLeft);i++)
	{
		if((i+1)>=height)
			add_levels(1);

		if(i%2)
			negativeCriticalsCovered+=levels[i].signal_taken;
		else
			positiveCriticalsCovered+=levels[i].signal_taken;

		expanded = levels[i].vacant;
		if(expanded > 0)
		{
			if(i % 2)
			{
                //Current Level (i) is negative
				//Next level (i+1) is positive
                //Remember that we allocated an extra slot for an inverter at the last level
				positiveAvailable = levels[i+1].vacant + (expanded*degree) + 1; //+1 for the inverter that was alloted
				if((positiveAvailable >= positiveConsumersLeft)&&(positiveCriticalsCovered==numPositiveCriticals))
				{
					negativeAvailable = (positiveAvailable-positiveConsumersLeft)/degree;
					positiveAvailable=positiveConsumersLeft;
					//printf("Height: %u/%u, Pos: %u/%u Neg %u/%u\n",i,height,positiveAvailable,positiveConsumersLeft,negativeAvailable,negativeConsumersLeft);
				    if((negativeAvailable >= negativeConsumersLeft)&&(negativeCriticalsCovered==numNegativeCriticals)) //If this is not enough, these won't be the final layers and we should expand all
                    {
                        levels[i+1].inv_taken--;
                        levels[i+1].vacant++;
                        if(negativeAvailable < expanded)
                            expanded -= negativeAvailable;
                        else
                            expanded = 0;
                    }
				}
			}
			else
			{
                //Current Level (i) is positive
				//Next level (i+1) is negative
                //Remember that we allocated an extra slot for an inverter at the last level
				negativeAvailable = levels[i+1].vacant + (expanded*degree) + 1;
				if((negativeAvailable >= negativeConsumersLeft)&&(negativeCriticalsCovered==numNegativeCriticals))
				{
					positiveAvailable = (negativeAvailable-negativeConsumersLeft)/degree;
					negativeAvailable=negativeConsumersLeft;
					//printf("Height: %u/%u, Pos: %u/%u Neg %u/%u\n",i,height,positiveAvailable,positiveConsumersLeft,negativeAvailable,negativeConsumersLeft);

				    if((positiveAvailable >= positiveConsumersLeft)&&(positiveCriticalsCovered==numPositiveCriticals)) //If this is not enough, these won't be the final layers and we should expand all
                    {
                        levels[i+1].inv_taken--;
                        levels[i+1].vacant++;
                        if(positiveAvailable < expanded)//Just in case there are several unpropagated available at the next layer
                            expanded -= positiveAvailable;
                        else
                            expanded = 0;
                    }
				}
			}
			//printf("Expanded: %u\n",expanded);
			levels[i].vacant -= expanded;
			levels[i].inv_taken += expanded;
			levels[i+1].vacant += degree*expanded;
		}
	}
	
	//printf("\tEnd of expansion\n");
	//printf("\tLeft/Available: (+): %u/%u (-) %u/%u\n",positiveConsumersLeft,positiveAvailable,negativeConsumersLeft,negativeAvailable);

}
void InverterTree::prune()
{
	//This function needs some cleaning up
	int simulatedPosConsumersLeft = numPositiveTargets;
	int simulatedNegConsumersLeft = numNegativeTargets;
	
	unsigned inv_saved;
	int pruned;
	//for(unsigned i=0;i<height;i++)
	//	printf("Vacant: %u Inverters Fed: %u Signals Fed: %u\n",levels[i].vacant,levels[i].inv_taken,levels[i].signal_taken);	
	do
	{
		inv_saved = 0;
		for(unsigned h=height-1;h>0;h--)
		{
			//printf("Height: %u\n",h);
			//printf("\tP Left: %d\n",simulatedPosConsumersLeft);
			//printf("\tN Left: %d\n",simulatedNegConsumersLeft);
			if(h%2)
			{
				pruned = degree*(((int)levels[h].vacant - simulatedNegConsumersLeft)/degree);
				if(pruned>0)
				{
					//printf("Height %u:Pruning: %u, saving %d invs\n",height,pruned,pruned/degree);
					
					levels[h].vacant-=pruned;
					simulatedNegConsumersLeft-=pruned;
					if(simulatedNegConsumersLeft<0)
						simulatedNegConsumersLeft=0;
					levels[h-1].vacant+=pruned/degree;
					levels[h-1].inv_taken-=pruned/degree;
					inv_saved+=pruned/degree;
					//simulatedPosConsumersLeft+=pruned/degree;
				}
				else
					pruned = 0;
			}
			else
			{
				pruned = degree*(((int)levels[h].vacant - simulatedPosConsumersLeft)/degree);
				if(pruned>0)
				{
					//printf("Height %u:Pruning: %u, saving %d invs\n",height,pruned,pruned/degree);
					levels[h].vacant-=pruned;
					simulatedPosConsumersLeft-=pruned;
					if(simulatedPosConsumersLeft<0)
						simulatedPosConsumersLeft=0;
					levels[h-1].vacant+=pruned/degree;
					levels[h-1].inv_taken-=pruned/degree;
					inv_saved+=pruned/degree;
					//simulatedNegConsumersLeft+=pruned/degree;
				}
				else
					pruned = 0;
			}
		}
		//Remember to test everytime if amount of inverters in earlier layer is necessary (we may be able to prune more if we take 3(1) and 3(1) we should save 3 invs) 
		//distribute signals
		simulatedPosConsumersLeft = numPositiveTargets;
		simulatedNegConsumersLeft = numNegativeTargets;

		for(unsigned h = 0;h<height;h++)
		{
			//Fills the levels with signals, preferably from the first levels
			unsigned signals = levels[h].vacant;
			
				if(h%2)
				{
					if(levels[h].signal_taken>simulatedNegConsumersLeft)
					{
						levels[h].vacant += levels[h].signal_taken - simulatedNegConsumersLeft;
						levels[h].signal_taken = simulatedNegConsumersLeft;
					}
					else
						simulatedNegConsumersLeft-=levels[h].signal_taken;
					
					if(signals>simulatedNegConsumersLeft)
						signals = simulatedNegConsumersLeft;
					simulatedNegConsumersLeft-=signals;
					levels[h].vacant-=signals;
					levels[h].signal_taken+=signals;
				}
				else
				{
					if(levels[h].signal_taken>simulatedPosConsumersLeft)
					{
						levels[h].vacant += levels[h].signal_taken - simulatedPosConsumersLeft;
						levels[h].signal_taken = simulatedPosConsumersLeft;
					}
					else
						simulatedPosConsumersLeft-=levels[h].signal_taken;
					
					if(signals>simulatedPosConsumersLeft)
						signals = simulatedPosConsumersLeft;
					simulatedPosConsumersLeft-=signals;
					levels[h].vacant-=signals;
					levels[h].signal_taken+=signals;
				}
		}
	}
	while(inv_saved>0);
	//for(unsigned i=0;i<height;i++)
	//	printf("Vacant: %u Inverters Fed: %u Signals Fed: %u\n",levels[i].vacant,levels[i].inv_taken,levels[i].signal_taken);	
}
void InverterTree::connect_targets()
{
    sizeInvertersArray = 0;
	//Determine the maximum size for Inverters Array
	for(unsigned h=0;h<height;h++)
		if(sizeInvertersArray < levels[h].inv_taken)
			sizeInvertersArray = levels[h].inv_taken;
	
	//Allocated reusable space
	inverters = (TEMP_INVERTER*) malloc(sizeInvertersArray*sizeof(TEMP_INVERTER));
	for(unsigned i=0;i<sizeInvertersArray;i++)
    {
		inverters[i].targets_indexes = (unsigned*) malloc(degree*sizeof(unsigned));
        inverters[i].num_targets = 0;
        inverters[i].position.x =0;
        inverters[i].position.y =0;
        for(unsigned j = 0;j<degree;j++)
        {
            inverters[i].targets_indexes[j]=0;
        }
    }
	
	for(currentLayer = height-1;currentLayer>0;currentLayer--)
	{
		//numInverters = ((numTargets-1) / degree)+1;
		numInverters = levels[currentLayer-1].inv_taken;
		if(numInverters>0)
		{
			collect_targets(currentLayer);
			if(currentLayer%2)
			{
				targets = currentNegativeTargets;
				numTargets = numCurrentNegativeTargets;
			}
			else
			{
				targets = currentPositiveTargets;
				numTargets = numCurrentPositiveTargets;
			}
        }
		if((numInverters>0)&&(numTargets>0))
        {
			
			//printf("Layer: %u, Num inv: %u/%u Num Targets: %u/%u/%u\n",currentLayer,numInverters,levels[currentLayer-1].inv_taken,numTargets,degree*numInverters,degree*levels[currentLayer-1].inv_taken);
			if(numInverters>sizeInvertersArray)
			{
				printf("More inverters than calculated %u > %u\n",numInverters,sizeInvertersArray);//This should never happen
				print();
				exit(1);
			}
			//printf("Current Layer is %u (%c) %u targets, %u inverters\n",currentLayer,(currentLayer%2)? '-' : '+',numTargets,numInverters);

	        //Introduce here the Non-Critical allocation
			
	        non_critical_allocation(); 
			
	        float totalDistance = 0;
	        for(unsigned i=0;i<numInverters;i++)
	            for(unsigned t=0;t<inverters[i].num_targets;t++)
	                totalDistance+=inverters[i].position.distance(targets[inverters[i].targets_indexes[t]].position);

	        //printf("Total Distance: %.2f\n",totalDistance);
	        //Temporary Inverters are ready, let's consolidate them
			for(unsigned i=0;i<numInverters;i++)
			{
				unsigned inv_index = consolidate_inverter(targets,inverters[i]);
				if(currentLayer%2)
				{
					//Current layer is negative, these inverters will be targets in the positive layer
					add_current_positive_target(inv_index,false,positionedInverters[inv_index].post_delay,inverters[i].position);
				}
				else
				{
					//Current layer is positive, these inverters will be targets in the negative layer
					add_current_negative_target(inv_index,false,positionedInverters[inv_index].post_delay,inverters[i].position);
				}
			}
			//Zero out current layer occupation
			if(currentLayer%2)
				numCurrentNegativeTargets = 0;
			else
				numCurrentPositiveTargets = 0;
		}
	}
	
	//Deallocate reused space
	for(unsigned i=0;i<sizeInvertersArray;i++)
		free(inverters[i].targets_indexes);
	free(inverters);

}
void InverterTree::determine_max_delay()
{
	maxDelay=0;

	for(unsigned i=0;i<numPositiveTargets;i++)
		if((positiveLevels[i] == 0)&&(positiveTargets[i].post_delay>maxDelay))
			maxDelay = positiveTargets[i].post_delay;
	for(unsigned i=0;i<numCurrentPositiveTargets;i++)
		if(positionedInverters[currentPositiveTargets[i].target].post_delay > maxDelay)
			maxDelay = positionedInverters[currentPositiveTargets[i].target].post_delay;
}

//Critical targets algorithms
void InverterTree::place_criticals()
{
	switch(CRIT_ALG)
	{
		default:
		case 0: place_criticals_FlatPercent();
			break;
		case 1: place_criticals_RelativePercent();
			break;
		case 2: place_criticals_InvDifference();
			break;
        case 3: numNegativeCriticals = 0;numPositiveCriticals=0;
            break;        
	}
}
void InverterTree::place_criticals_FlatPercent()
{
	float highest_delay = 0;
	if(numPositiveTargets>0)
        highest_delay = positiveTargets[0].post_delay;
    if((numNegativeTargets>0)&&(negativeTargets[0].post_delay>highest_delay))
        highest_delay = negativeTargets[0].post_delay;

	for(unsigned p=0;p<numPositiveTargets;p++)
	{
		if(positiveTargets[p].post_delay>=highest_delay*CRITICAL_THRESHOLD)
			add_critical_target(p,false);
	}
	for(unsigned n=0;n<numNegativeTargets;n++)
	{
		if(negativeTargets[n].post_delay>=highest_delay*CRITICAL_THRESHOLD)
			add_critical_target(n,true);
	}
}
void InverterTree::place_criticals_RelativePercent()
{
    float average_delay = 0;

	for(unsigned p=0;p<numPositiveTargets;p++)
    {
        average_delay+=positiveTargets[p].post_delay;
    }
	for(unsigned n=0;n<numNegativeTargets;n++)
    {
        average_delay+=negativeTargets[n].post_delay;
    }
    average_delay /= (numNegativeTargets+numPositiveTargets);

	for(unsigned p=0;p<numPositiveTargets;p++)
	{
		if(positiveTargets[p].post_delay>=average_delay*(2-CRITICAL_THRESHOLD))
			add_critical_target(p,false);
	}
	for(unsigned n=0;n<numNegativeTargets;n++)
	{
		if(negativeTargets[n].post_delay>=average_delay*(2-CRITICAL_THRESHOLD))
			add_critical_target(n,true);
	}
}
void InverterTree::place_criticals_InvDifference()
{
	for(unsigned h=0;h<height;h++)
	{
		unsigned groupSize = levels[h].vacant;
		if(h%2)
		{
            if((2+(int)numNegativeCriticals+groupSize)>((int)numNegativeTargets))
                groupSize = (numNegativeTargets - numNegativeCriticals -2);
			for(int n = numNegativeCriticals + groupSize-1;n>=(int)numNegativeCriticals;n--)
			{
				//printf("n: %d [%d]/%d ",n,numNegativeCriticals,numNegativeTargets);
				//Find the greatest group that is substantially (2*inverterDelay) more critical than the rest
				//printf("n: %d %.4f vs %.4f\n",n,negativeTargets[n].post_delay,(2*inverterDelay)+negativeTargets[n+1].post_delay);
				if(negativeTargets[n].post_delay>(2*inverterDelay)+negativeTargets[n+1].post_delay)
				{
					for(unsigned i=numNegativeCriticals;i<n;i++)
					{
						levels[h].signal_taken++;
						levels[h].vacant--;
						determine_level(i,h);
					}
					numNegativeCriticals=n+1;
					break;
				}
			}
		}
		else
		{
            if((2+(int)numPositiveCriticals+groupSize)>((int)numPositiveTargets))
                groupSize = (numPositiveTargets - numPositiveCriticals -2);
			for(int p = numPositiveCriticals + groupSize-1;p>=(int)numPositiveCriticals;p--)
			{
				//Find the greatest group that is substantially (2*inverterDelay) more critical than the rest
				//printf("p: %d %.4f vs %.4f\n",p,positiveTargets[p].post_delay,(2*inverterDelay)+positiveTargets[p+1].post_delay);
				if(positiveTargets[p].post_delay>(2*inverterDelay)+positiveTargets[p+1].post_delay)
				{
					for(unsigned i=numPositiveCriticals;i<=p;i++)
					{
						levels[h].signal_taken++;
						levels[h].vacant--;
						determine_level(i,h);
					}
					numPositiveCriticals=p+1;
					break;
				}
			}
		}
		if(h+1<height)
		{
			levels[h].inv_taken+=levels[h].vacant;
			levels[h+1].vacant+=degree*levels[h].vacant;
			levels[h].vacant = 0;
		}
	}
}

//Non critical targets algorithms
void InverterTree::place_non_criticals()
{
    switch(NON_CRIT_PLACE)
    {
        default:
        case 0: place_non_criticals_Ordered();
                break;
        case 1: place_non_criticals_Random();
                break;
    }
}
void InverterTree::place_non_criticals_Ordered()
{
	//Prepare levels for non_critical allocation:
	//Take away the criticals from signal_taken so that we know how many non_critical are required
	unsigned p;
	unsigned n;
	for(p=0;p<numPositiveCriticals;p++)
	{
		if(levels[positiveLevels[p]].signal_taken!=0)
		{
			levels[positiveLevels[p]].signal_taken--;
		}
	}
	for(n=0;n<numNegativeCriticals;n++)
	{
		if(levels[negativeLevels[n]].signal_taken!=0)
		{
			levels[negativeLevels[n]].signal_taken--;
		}
	}
	for(unsigned h = 0;h<height;h++)
	{
        //n and p start at the non criticals
        //This code may look weird, but it is correct
		unsigned signals_cap = levels[h].signal_taken;
        if(h%2)
        {
            signals_cap+=n;
            if(signals_cap>numNegativeTargets)
                signals_cap = numNegativeTargets;
            for(;n<signals_cap;n++)
                determine_level(n,h);
        }
        else
        {
            signals_cap+=p;
            if(signals_cap>numPositiveTargets)
                signals_cap = numPositiveTargets;
            for(;p<signals_cap;p++)
                determine_level(p,h);
        }
	}
	//Revalidate value of signal_taken
	for(unsigned p=0;p<numPositiveCriticals;p++)
		levels[positiveLevels[p]].signal_taken++;
	for(unsigned n=0;n<numNegativeCriticals;n++)
		levels[negativeLevels[n]].signal_taken++;
}
void InverterTree::place_non_criticals_Random()
{
    bool * pAssignedTargets = (bool*) malloc(numPositiveTargets*sizeof(bool));
    bool * nAssignedTargets = (bool*) malloc(numNegativeTargets*sizeof(bool));
    for(unsigned i=0;i<numPositiveTargets;i++)
        pAssignedTargets[i] = false;
    for(unsigned i=0;i<numNegativeTargets;i++)
        nAssignedTargets[i] = false;

	unsigned p=0;
	unsigned n=0;
	for(p=0;p<numPositiveCriticals;p++)
    {
        pAssignedTargets[p] = true;
		if(levels[positiveLevels[p]].signal_taken!=0)
			levels[positiveLevels[p]].signal_taken--;
    }
	for(n=0;n<numNegativeCriticals;n++)
    {
        nAssignedTargets[n] = true;
		if(levels[negativeLevels[n]].signal_taken!=0)
			levels[negativeLevels[n]].signal_taken--;
    }

	srand(time(NULL));
/*
	for(unsigned h = 0;h<height;h++)
	{
        for(unsigned i = 0;i<levels[h].signal_taken;i++)
        {
            if(h%2)
            {
                unsigned rando = (rand() % (numNegativeTargets-numNegativeCriticals)) + numNegativeCriticals;
                for(unsigned j = 0;j<numNegativeTargets;j++)
                {
                    if(!nAssignedTargets[rando])
                    {
                        nAssignedTargets[rando] = true;
                        determine_level(rando,h);
                        break;
                    }
                    else
                    {
                        rando = ((rando +1) % (numNegativeTargets-numNegativeCriticals)) + numNegativeCriticals;
                    }
                }
            }
            else
            {
                unsigned rando = (rand() % (numPositiveTargets-numPositiveCriticals)) + numPositiveCriticals;
                for(unsigned j = 0;j<numPositiveTargets;j++)
                {
                    if(!pAssignedTargets[rando])
                    {
                        pAssignedTargets[rando] = true;
                        determine_level(rando,h);
                        break;
                    }
                    else
                    {
                        rando = ((rando +1) % (numPositiveTargets-numPositiveCriticals)) + numPositiveCriticals;
                    }
                }
            }
        }
	}
*/
	for(unsigned h = 0;h<height;h++)
	{
        //n and p start at the non criticals
        //This code may look weird, but it is correct
		unsigned signals_cap = levels[h].signal_taken;
        if(h%2)
        {
            signals_cap+=n;
            if(signals_cap>numNegativeTargets)
                signals_cap = numNegativeTargets;
            for(;n<signals_cap;n++)
                determine_level(numNegativeTargets-n-1+numNegativeCriticals,h);
        }
        else
        {
            signals_cap+=p;
            if(signals_cap>numPositiveTargets)
                signals_cap = numPositiveTargets;
            for(;p<signals_cap;p++)
                determine_level(numPositiveTargets-p-1+numPositiveCriticals,h);
        }
	}
	//Revalidate value of signal_taken
	for(unsigned p=0;p<numPositiveCriticals;p++)
		levels[positiveLevels[p]].signal_taken++;
	for(unsigned n=0;n<numNegativeCriticals;n++)
		levels[negativeLevels[n]].signal_taken++;

    free(pAssignedTargets);
    free(nAssignedTargets);
}
float InverterTree::non_critical_allocation() 
{
    switch(NON_CRIT_ALG)
    {
        default:
        case 0: 
            return non_critical_allocation_kmeans();
        case 1: 
            return non_critical_allocation_worstFirst();
    }
}
float InverterTree::non_critical_allocation_worstFirst()
{
    float furthest_distance;
    float worstDelay = 0;
	Point furthestPoint;
	unsigned furthest;
    if(numTargets==0)	
        return 0;
	bool * supplied_targets = (bool*) malloc(numTargets*sizeof(bool));
	float * distances = (float*) malloc(degree*sizeof(float));
	
	for(unsigned i =0;i<numTargets;i++)
		supplied_targets[i] = false;
	
	for(unsigned inv=0;inv<numInverters;inv++)
	{
		furthest = 0;
		furthest_distance = -1;
		for(unsigned i =0;i<numTargets;i++)
		{
			if((!supplied_targets[i])&&(targets[i].position.distance(sourcePosition)>furthest_distance))
			{
				furthest_distance = targets[i].position.distance(sourcePosition);
				furthestPoint = targets[i].position;
				furthest = i;
			}
		}
		inverters[inv].targets_indexes[0] = furthest;
		inverters[inv].num_targets=1;
		supplied_targets[furthest] = true;
		
		if(furthest_distance>0)
		{
			for(unsigned i =0;i<numTargets;i++)
			{
				float worst_distance = 0;//worst distance from the inverter to it's targets
				unsigned worstIndex = 0;
				if(!supplied_targets[i])
				{	
					float distance = targets[i].position.distance(furthestPoint);
					if(inverters[inv].num_targets < degree)
					{
						
						distances[inverters[inv].num_targets] = distance;
						inverters[inv].targets_indexes[inverters[inv].num_targets++] = i;
						if(distance>worst_distance)
						{
							worstIndex = i;
							worst_distance = distance;
						}
					}
					else
					{
						if(distance < worst_distance)//If we found a good one
						{
							distances[worstIndex] = distance;
							inverters[inv].targets_indexes[worstIndex] = i;
                            worst_distance = distance;
							for(unsigned j=0;j<degree;j++) //Rediscover worst distance
							{
								if(distances[j]>worst_distance)
								{
									worstIndex = j;
									worst_distance = distances[j];
								}
							}
						}
					}
				}
			}
			inverters[inv].position = furthestPoint;
			for(unsigned j=0;j<inverters[inv].num_targets;j++)
				supplied_targets[inverters[inv].targets_indexes[j]] = true;
		}
	}
	
	free(supplied_targets);
	free(distances);
    return worstDelay;
}
float InverterTree::non_critical_allocation_kmeans() 
{
	srand(time(NULL));
	//Randomly seed inverters
    if(numTargets == 0)
        return 0;
	for(unsigned i=0;i<numInverters;i++) // Starting position
		inverters[i].position = targets[rand()%numTargets].position;
    //K Means
    float worstDelay = 0;
    for(unsigned iterations=0;iterations<5;iterations++)
    {
        for(unsigned i=0;i<numInverters;i++)
            inverters[i].num_targets = 0;
        
        for(unsigned t=0;t<numTargets;t++)
        {
            //For each target, choose the closest inverter
            int closest_inverter=-1;//Cant start with the first because it may be full
            float closest_distance = 0;//targets[t].position.distance(inverters[0].position); //Go home valgrind, you're drunk
            for(unsigned i=0;i<numInverters;i++)
            {
                if(inverters[i].num_targets<degree)
                {
                    float current_distance = targets[t].position.distance(inverters[i].position);
                    if((current_distance<closest_distance)||(closest_inverter<0))
                    {
                        closest_distance = current_distance;
                        closest_inverter = i;
                    }
                }
            }
            //if(inverters[closest_inverter].num_targets<degree)
            inverters[closest_inverter].targets_indexes[inverters[closest_inverter].num_targets++] = t;
        }
        
        for(unsigned i=0;i<numInverters;i++)
        {
            //Reposition each inverter according to its targets
            position_inverter(&inverters[i]);
        }
        //For tests
        float totalDistance = 0;
        for(unsigned i=0;i<numInverters;i++)
            for(unsigned t=0;t<inverters[i].num_targets;t++)
            {
                totalDistance+=inverters[i].position.distance(targets[inverters[i].targets_indexes[t]].position);
            }
        //printf("Total Distance: %.2f\n",totalDistance);
    }

    return worstDelay;
}

//Inverter Positioning algorithms
void InverterTree::position_inverter(TEMP_INVERTER * inv)
{
    switch(INV_POS)
	{
		default:
		case 0: position_inverter_centroid(inv);
				break;
		case 1: position_inverter_ponderateCentroid(inv);
				break;
	}
}
void InverterTree::position_inverter_centroid(TEMP_INVERTER * inv)
{
	float x = 0;
    float y = 0;
    for(unsigned i=0;i<inv->num_targets;i++)
    {
        x += targets[inv->targets_indexes[i]].position.x;
        y += targets[inv->targets_indexes[i]].position.y;
    }
    inv->position.x = x/inv->num_targets;
    inv->position.y = y/inv->num_targets;
}
void InverterTree::position_inverter_ponderateCentroid(TEMP_INVERTER * inv)
{
	float x = 0;
    float y = 0;
	float totalWeight = 0;
	float weight;
    for(unsigned i=0;i<inv->num_targets;i++)
    {
		weight = targets[inv->targets_indexes[i]].post_delay;
		totalWeight+=weight;
        x += (targets[inv->targets_indexes[i]].position.x)*weight;
        y += (targets[inv->targets_indexes[i]].position.y)*weight;
    }
    inv->position.x = x/(totalWeight);
    inv->position.y = y/(totalWeight);
}

void InverterTree::add_levels(unsigned newLevels)
{
	height+=newLevels;
	levels=(LEVEL*) realloc(levels,height*sizeof(LEVEL));
    
	for(unsigned i=height-newLevels;i<height;i++)
	{
		levels[i].vacant = degree - 1;
		levels[i].inv_taken = 1;
		levels[i].signal_taken = 0;
	}
}
void InverterTree::add_critical_target(unsigned target_index,bool signal)
{
	//Allocate as close to the source as possible, always leaving a space to deliver the rest of the signals
	unsigned i;
	
	if(signal)
	{
		i=1; //Negative levels
		negativeConsumersLeft--;
		numNegativeCriticals++;
	}
	else
	{
		i=0;// Positive levels
		positiveConsumersLeft--;
		numPositiveCriticals++;
	}
	//printf("Height: %u\n",height);
	for(;i<height;i+=2)
	{
		//printf("Trying Target: %u at %u, vacant: %u\n",target_index,i,levels[i].vacant);
		if(levels[i].vacant>0)
		{
			//printf("1Added Target %u to level %u\n",target_index,i);
			determine_level(target_index,i);
			levels[i].vacant--;
			levels[i].signal_taken++;
			return;
		}
		else if ((i>0)&&(levels[i-1].vacant>0))
		{
			levels[i-1].vacant--;
			levels[i-1].inv_taken++;
			levels[i].vacant +=degree-1;
			levels[i].signal_taken++;//print();
			determine_level(target_index,i);
			return;
		}
	}
	
	//If code reaches this point, we need more inverters!
	//First let's look at the lower levels, to see if we can propagate some space
	//This is possible due to different target signals
	//Inverted signal
	
	if(signal)
		i=0; 
	else
		i=1;
	for(;(i+1)<height;i+=2)
	{
		if(levels[i].vacant>0)
		{
			//printf("Added Inverter to level %u\n",i);
			levels[i].vacant--;
			levels[i].inv_taken++;
			levels[i+1].vacant +=degree-1;
			levels[i+1].signal_taken++;//print();
			determine_level(target_index,i+1);
			//printf("2Added Target %u to level %u\n",target_index,i+1);
			return;
		}
	}
	//If code reaches this point, we need more levels!
	if(i==height)
		add_levels(2);
	else
		add_levels(1);//i == height -1
	i++;
	levels[i].vacant--;
	levels[i].signal_taken++;
	determine_level(target_index,i);
	//printf("3Added Target %u to level %u\n",target_index,i);
	return;
}
void InverterTree::add_negative_target(unsigned target,bool isVertex,float delay,Point position)
{
	//Allocate according to position
	//Make a list of all non critical targets that need to be reached and then determine which inverters feed which targets
    //Make one for positive and one for negative targets, send through parameters if the target is inverter or vertex
	//printf("Adding (-) target: %u, delay: %.4f\n",target,delay);
	negativeTargets[numNegativeTargets].target = target;
	negativeTargets[numNegativeTargets].isVertex = isVertex;
	negativeTargets[numNegativeTargets].post_delay = delay;
	negativeTargets[numNegativeTargets++].position = position;
}
void InverterTree::add_positive_target(unsigned target,bool isVertex,float delay,Point position)
{
	//Allocate according to position
	//Make a list of all non critical targets that need to be reached and then determine which inverters feed which targets
    //Make one for positive and one for negative targets, send through parameters if the target is inverter or vertex
	//printf("Adding (+) target: %u, delay: %.4f\n",target,delay);
	positiveTargets[numPositiveTargets].target = target;
	positiveTargets[numPositiveTargets].isVertex = isVertex;
	positiveTargets[numPositiveTargets].post_delay = delay;
	positiveTargets[numPositiveTargets++].position = position;
}
void InverterTree::add_current_negative_target(unsigned target,bool isVertex,float delay,Point position)
{
	//printf("Adding Current (+) target: %u, delay: %.4f\n",target,delay);
	currentNegativeTargets[numCurrentNegativeTargets].target = target;
	currentNegativeTargets[numCurrentNegativeTargets].isVertex = isVertex;
	currentNegativeTargets[numCurrentNegativeTargets].post_delay = delay;
	currentNegativeTargets[numCurrentNegativeTargets++].position = position;
}
void InverterTree::add_current_positive_target(unsigned target,bool isVertex,float delay,Point position)
{
	//printf("Adding Current (-) target: %u, delay: %.4f\n",target,delay);
	currentPositiveTargets[numCurrentPositiveTargets].target = target;
	currentPositiveTargets[numCurrentPositiveTargets].isVertex = isVertex;
	currentPositiveTargets[numCurrentPositiveTargets].post_delay = delay;
	currentPositiveTargets[numCurrentPositiveTargets++].position = position;
}
unsigned InverterTree::min_height(unsigned posConsumers,unsigned negConsumers)
{
    unsigned posAvailable = maximumCellFanout;
    unsigned negAvailable = 0;
    unsigned minHeight = 0;
    unsigned leavesAvailable, leaves;
    unsigned height1_branches;
    
    if((negConsumers == 0)&&(posConsumers<=maximumCellFanout)) 
    {
        return 0; 
    }
    do
    {
        minHeight++;
        if(minHeight%2)
        {
            //New layer is odd (negative)
            negAvailable=posAvailable*degree;
            leaves = negConsumers;
            height1_branches = posConsumers;
            leavesAvailable = negAvailable;
        }
        else
        {
            //New layer is even (positive)
            posAvailable=negAvailable*degree;
            leaves = posConsumers;
            height1_branches = negConsumers;
            leavesAvailable = posAvailable;
        }
        //printf("Consumers: %u %u\n",posConsumers,negConsumers);
        //printf("Height:%d Available P %d, N %d \n",minHeight,posAvailable,negAvailable);
    }
    while((leaves > leavesAvailable)||(height1_branches > ((leavesAvailable-leaves)/degree)));
	
    return minHeight;
}
void InverterTree::determine_level(unsigned tgt_index, unsigned level)
{
	//printf("Determining Target %u, level: %u\n",tgt_index,level);
	if(level %2)
    {
        if(tgt_index<numNegativeTargets)
            negativeLevels[tgt_index] = level;
        else
        {
            Debug = true;
            printf("Trying to assign tgt_index %u to level %u\n",tgt_index,level);
        }
    }
	else
    {
        if(tgt_index<numPositiveTargets)
            positiveLevels[tgt_index] = level;
        else
        {
            Debug = true;
            printf("Trying to assign tgt_index %u to level %u\n",tgt_index,level);
        }
    }
    //if(Debug)
        //print();
}
void InverterTree::collect_targets(unsigned level)
{
    verify();
	if(level % 2)
		for(unsigned i=0;i<numNegativeTargets;i++)
		{
			if(negativeLevels[i] == level)
			{
				//printf("Introducing target %u to level %u\n",critical_targets[i].target,level);
				currentNegativeTargets[numCurrentNegativeTargets++] = negativeTargets[i];
			}
		}
	else
		for(unsigned i=0;i<numPositiveTargets;i++)
		{
			if(positiveLevels[i] == level)
			{
				//printf("Introducing target %u to level %u\n",critical_targets[i].target,level);
				currentPositiveTargets[numCurrentPositiveTargets++] = positiveTargets[i];
			}
		}
}
unsigned InverterTree::consolidate_inverter(TARGET * target_list,TEMP_INVERTER temp_inv)
{
	//Depends on the temporary inverters and the TARGET array
	INVERTER inv;
	inv.targets = (unsigned*) malloc(degree*sizeof(unsigned));
	inv.num_inv_targets = 0;
	inv.num_vert_targets = 0;
	inv.position = temp_inv.position;
	inv.post_delay = 0;
	float post_delay_candidate =0;
	for(unsigned i=0;i<temp_inv.num_targets;i++)
	{//For better performance: Iterate over "temp_inv.targets_indexes[i]"
		
		if(target_list[temp_inv.targets_indexes[i]].isVertex)
		{
			inv.targets[inv.num_vert_targets++]=target_list[temp_inv.targets_indexes[i]].target;
			post_delay_candidate = target_list[temp_inv.targets_indexes[i]].post_delay;
		}
		else
		{
			inv.targets[degree-1-(inv.num_inv_targets++)]=target_list[temp_inv.targets_indexes[i]].target;
			post_delay_candidate = target_list[temp_inv.targets_indexes[i]].post_delay + inverterDelay;
		}
		//Add here path delay
		if(post_delay_candidate>inv.post_delay)
				inv.post_delay = post_delay_candidate;
	}
	//printf("Consolidating inverter: %u,%u targets \n",inv.num_inv_targets,inv.num_vert_targets);
	
	positionedInverters.push_back(inv);
	return positionedInverters.size()-1;
}
void InverterTree::print()
{
	printf("All Targets: +:[%u]/%u -:[%u]/%u criticals\n",numPositiveCriticals,numPositiveTargets,numNegativeCriticals,numNegativeTargets);
	unsigned i;
	for( i=0;i<numPositiveCriticals;i++)
		printf("(+) [%u]: %u %.4f\n",positiveTargets[i].target,positiveLevels[i],positiveTargets[i].post_delay);
	for(;i<numPositiveTargets;i++)
		printf("(+) %u: %u %.4f\n",positiveTargets[i].target,positiveLevels[i],positiveTargets[i].post_delay);
	for(i=0;i<numNegativeCriticals;i++)
		printf("(-) [%u]: %u %.4f\n",negativeTargets[i].target,negativeLevels[i],negativeTargets[i].post_delay);
	for(;i<numNegativeTargets;i++)
		printf("(-) %u: %u %.4f\n",negativeTargets[i].target,negativeLevels[i],negativeTargets[i].post_delay);

	printf("Inv Tree of Height %u, Degree %u\n",height,degree);
	for(unsigned i=0;i<height;i++)
		printf("Vacant: %u Inverters Fed: %u Signals Fed: %u\n",levels[i].vacant,levels[i].inv_taken,levels[i].signal_taken);
	
	printf("First Level:\n");
	for(unsigned i=0;i<numPositiveTargets;i++)
		if(positiveLevels[i] == 0)
			printf("Target: %u Post Delay: %.4f\n",positiveTargets[i].target,positiveTargets[i].post_delay);
	for(unsigned i=0;i<numCurrentPositiveTargets;i++)
		printf("Inverter %u Post Delay: %.4f\n",currentPositiveTargets[i].target,positionedInverters[currentPositiveTargets[i].target].post_delay);
    printf("Max Tree delay: %.2f\n",maxDelay);
}
void InverterTree::print_inverters()
{
	unsigned i=0,j;
	for (std::vector<INVERTER>::iterator it = positionedInverters.begin() ; it != positionedInverters.end(); ++it,i++)
	{
		printf("Inverter: %u (%.2f,%.2f) Post Delay: %.4f",i,it->position.x,it->position.y,it->post_delay);
		printf("\n\tSignals Fed(%u): ",it->num_vert_targets);
		for(j=0;j<it->num_vert_targets;j++)
			printf("%u ",it->targets[j]);
		printf("\n\tInverters Fed: (%u)",it->num_inv_targets);
		for(j=0;j<it->num_inv_targets;j++)
			printf("%u ",it->targets[degree-j-1]);
		printf("\n");
	}
}
void InverterTree::verify()
{
    for(unsigned i=0;i<numPositiveTargets;i++)
    {
        if(positiveLevels[i] > height)
        { 
            printf("Invalid Positive level\n");
            Debug = true;
        }
    }
    for(unsigned i=0;i<numNegativeTargets;i++)
    {
        if(negativeLevels[i] > height)
        {
            printf("Invalid Negative level\n");
            Debug = true;
        }
    }
    for(unsigned i=0;i<height-1;i++)
    {
        if((levels[i+1].vacant+levels[i+1].signal_taken+levels[i+1].inv_taken)!=(levels[i].inv_taken*degree))
        {
            printf("Incoherent Levels\n");
            Debug = true;
        }
    }
    if(Debug)
    {
        printf("Printing\n");getchar();
        //print();
    }
}
