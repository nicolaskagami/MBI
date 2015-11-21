#include "InverterTree.h"

InverterTree::InverterTree(unsigned posTargets,unsigned negTargets,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition, unsigned num_critical)
{
	sourcePosition = srcPosition;
	num_critical_targets = num_critical;
	inverterDelay = invDelay;
	maxDelay = 0;
	
	degree = maxInvFanout;
	maximumCellFanout = maxCellFanout;
	
	positiveConsumersLeft = posTargets;
	negativeConsumersLeft = negTargets;
	
	positiveTargets = (TARGET*) malloc((posTargets+negTargets)*sizeof(TARGET));//Careful here, it may be bigger than positiveConsumers...
	negativeTargets = (TARGET*) malloc((posTargets+negTargets)*sizeof(TARGET));
	numPositiveTargets = 0;
	numNegativeTargets = 0;
	
	critical_targets = (TARGET*) malloc(num_critical_targets*sizeof(TARGET));
	critical_levels = (unsigned*) malloc(num_critical_targets*sizeof(unsigned));
	
	critical_targets_occupied = 0;
	
	height = min_height(posTargets,negTargets);
	if(height == 0)
		height = 1;
	
	levels=(LEVEL*) malloc(height*sizeof(LEVEL));
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
    free(positiveTargets);
    free(negativeTargets);
	free(critical_levels);
	free(critical_targets);
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
void InverterTree::add_critical_target(unsigned target,bool signal,float delay, Point position)
{
	//Allocate as close to the source as possible, always leaving a space to deliver the rest of the signals
	unsigned i;
	TARGET tgt;
	tgt.target = target;
	tgt.isVertex = true;
	tgt.position = position;
	tgt.post_delay = delay;
	
	if(signal)
	{
		i=1; //Negative levels
		negativeConsumersLeft--;
	}
	else
	{
		i=0;// Positive levels
		positiveConsumersLeft--;
	}
	//printf("Height: %u\n",height);
	for(;i<height;i+=2)
	{
		//printf("Trying Target: %u at %u, vacant: %u\n",target,i,levels[i].vacant);
		if(levels[i].vacant>0)
		{
			//printf("1Added Target %u to level %u\n",target,i);
			save_critical_target(tgt,i);
			levels[i].vacant--;
			levels[i].signal_taken++;
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
			levels[i+1].signal_taken++;
			save_critical_target(tgt,i+1);
			//printf("2Added Target %u to level %u\n",target,i+1);
			return;
		}
	}
	//If code reaches this point, we need more levels!
	add_levels(1);
	i++;
	levels[i].vacant--;
	levels[i].signal_taken++;
	save_critical_target(tgt,i);
	printf("3Added Target %u to level %u\n",target,i);
	return;
	
}
void InverterTree::save_critical_target(TARGET tgt, unsigned level)
{
	if(critical_targets_occupied<num_critical_targets)
	{
		critical_levels[critical_targets_occupied] = level;
		critical_targets[critical_targets_occupied++] = tgt;
	}
}
void InverterTree::introduce_critical_targets(unsigned level)
{
	
	for(unsigned i=0;i<critical_targets_occupied;i++)
	{
		if(critical_levels[i] == level)
		{
			//printf("Introducing target %u to level %u\n",critical_targets[i].target,level);
			if(level % 2)
				negativeTargets[numNegativeTargets++] = critical_targets[i];
			else
				positiveTargets[numPositiveTargets++] = critical_targets[i];
		}
	}
}
void InverterTree::expand()
{
	//Expand until there is space for the rest, as determined by the targetLeft variables
	unsigned positiveAvailable =0;
	unsigned negativeAvailable =0;
	unsigned expanded;
	unsigned i;
	printf("Starting height: %u\n",height);
	//Expand to provide for non criticals
	for(i=0;(positiveAvailable<positiveConsumersLeft)||(negativeAvailable<negativeConsumersLeft);i++)
	{
		if((i+1)>=height)
			add_levels(1);
		
		expanded = levels[i].vacant;
		if(expanded > 0)
		{
			if(i % 2)
			{
                //Current Level (i) is negative
				//Next level (i+1) is positive
                //Remember that we allocated an extra slot for an inverter at the last level
				positiveAvailable = levels[i+1].vacant + (expanded*degree) + 1; //+1 for the inverter that was alloted
				if(positiveAvailable >= positiveConsumersLeft)
				{
					negativeAvailable = (positiveAvailable-positiveConsumersLeft)/degree;
					positiveAvailable-=positiveConsumersLeft;
						printf("Height: %u/%u, Pos: %u/%u Neg %u/%u\n",i,height,positiveAvailable,positiveConsumersLeft,negativeAvailable,negativeConsumersLeft);

				    if(negativeAvailable >= negativeConsumersLeft) //If this is not enough, these won't be the final layers and we should expand all
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
				if(negativeAvailable >= negativeConsumersLeft)
				{
					positiveAvailable = (negativeAvailable-negativeConsumersLeft)/degree;
					negativeAvailable-=negativeConsumersLeft;
											printf("Height: %u/%u, Pos: %u/%u Neg %u/%u\n",i,height,positiveAvailable,positiveConsumersLeft,negativeAvailable,negativeConsumersLeft);

				    if(positiveAvailable >= positiveConsumersLeft) //If this is not enough, these won't be the final layers and we should expand all
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
}
void InverterTree::add_negative_target(unsigned target,bool isVertex,float delay,Point position)
{
	//Allocate according to position
	//Make a list of all non critical targets that need to be reached and then determine which inverters feed which targets
    //Make one for positive and one for negative targets, send through parameters if the target is inverter or vertex
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
	positiveTargets[numPositiveTargets].target = target;
	positiveTargets[numPositiveTargets].isVertex = isVertex;
	positiveTargets[numPositiveTargets].post_delay = delay;
	positiveTargets[numPositiveTargets++].position = position;
}
unsigned InverterTree::min_height(unsigned posConsumers,unsigned negConsumers)
{
    unsigned posAvailable = maximumCellFanout;
    unsigned negAvailable = 0;
    unsigned minHeight = 0;
    unsigned leavesAvailable, leaves;
    unsigned height1_branches;
    
    if((negConsumers == 0)&&(posConsumers<=degree)) 
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
        //printf("Height:%d Available P %d, N %d \n",minHeight,posAvailable,negAvailable);
    }
    while((leaves > leavesAvailable)||(height1_branches > ((leavesAvailable-leaves)/degree)));
	
    return minHeight;
}
void InverterTree::connect_positioned_targets()
{
	srand(time(NULL));
	
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
		introduce_critical_targets(currentLayer);
		if(currentLayer%2)
		{
			targets = negativeTargets;
			numTargets = numNegativeTargets;
		}
		else
		{
			targets = positiveTargets;
			numTargets = numPositiveTargets;
		}
		
		numInverters = ((numTargets-1) / degree)+1;
		//numInverters = levels[currentLayer-1].inv_taken;
		printf("Layer: %u, Num inv: %u/%u Num Targets: %u/%u/%u\n",currentLayer,numInverters,levels[currentLayer-1].inv_taken,numTargets,degree*numInverters,degree*levels[currentLayer-1].inv_taken);
		if(numInverters>sizeInvertersArray)
		{
			printf("More inverters than calculated %u > %u\n",numInverters,sizeInvertersArray);//This should never happen
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
				add_positive_target(inv_index,false,0,inverters[i].position);
			}
			else
			{
				//Current layer is positive, these inverters will be targets in the negative layer
				add_negative_target(inv_index,false,0,inverters[i].position);
			}
		}
		//Zero out current layer occupation
		if(currentLayer%2)
			numNegativeTargets = 0;
		else
			numPositiveTargets = 0;
	}
	
	//Deallocate reused space
	for(unsigned i=0;i<sizeInvertersArray;i++)
		free(inverters[i].targets_indexes);
	free(inverters);

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
			for(unsigned j=0;j<degree;j++)
				supplied_targets[inverters[inv].targets_indexes[j]] = true;
		}
	}
	
	free(supplied_targets);
	free(distances);
    return worstDelay;
}
float InverterTree::non_critical_allocation_kmeans() 
{ 
	//Randomly seed inverters
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
void InverterTree::print()
{
	printf("Inv Tree of Height %u, Degree %u\n",height,degree);
	
		
	for(unsigned i=0;i<height;i++)
	{
		printf("Vacant: %u Inverters Fed: %u Signals Fed: %u\n",levels[i].vacant,levels[i].inv_taken,levels[i].signal_taken);
	}
	printf("First Level:\n");
	for(unsigned i=0;i<critical_targets_occupied;i++)
		if(critical_levels[i] == 0)
			printf("Target: %u\n",i);
	for(unsigned i=0;i<numPositiveTargets;i++)
		printf("Inverter %u\n",positiveTargets[i].target);
}
void InverterTree::print_inverters()
{
	unsigned i=0,j;
	for (std::vector<INVERTER>::iterator it = positionedInverters.begin() ; it != positionedInverters.end(); ++it,i++)
	{
		printf("Inverter: %u (%.2f,%.2f): Signals Fed: (%u)",i,it->position.x,it->position.y,it->num_vert_targets);
		for(j=0;j<it->num_vert_targets;j++)
			printf("%u ",it->targets[j]);
		printf("Inverters Fed: (%u)",it->num_inv_targets);
		for(j=0;j<it->num_inv_targets;j++)
			printf("%u ",it->targets[degree-j-1]);
		printf("\n");
	}
}
void InverterTree::position_inverter(TEMP_INVERTER * inv)
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

unsigned InverterTree::consolidate_inverter(TARGET * target_list,TEMP_INVERTER temp_inv)
{
	//Depends on the temporary inverters and the TARGET array
	INVERTER inv;
	inv.targets = (unsigned*) malloc(degree*sizeof(unsigned));
	inv.num_inv_targets = 0;
	inv.num_vert_targets = 0;
	inv.position = temp_inv.position;
	for(unsigned i=0;i<temp_inv.num_targets;i++)
	{//For better performance: Iterate over "temp_inv.targets_indexes[i]"
		
		if(target_list[temp_inv.targets_indexes[i]].isVertex)
			inv.targets[inv.num_vert_targets++]=target_list[temp_inv.targets_indexes[i]].target;
		else
			inv.targets[degree-1-(inv.num_inv_targets++)]=target_list[temp_inv.targets_indexes[i]].target;
	}
	//printf("Consolidating inverter: %u,%u targets \n",inv.num_inv_targets,inv.num_vert_targets);
	
	positionedInverters.push_back(inv);
	return positionedInverters.size()-1;
}
