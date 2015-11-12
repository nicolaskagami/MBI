#include "InverterTree.h"


bool Point::operator== (Point b)
{ 
    if((x == b.x)&&(y == b.y))
        return true; 
    else 
        return false;
}
float Point::manhattanDistance(Point b)
{
	return abs(x-b.x) + abs(y-b.y);
}
InverterTree::InverterTree(unsigned posTargets,unsigned negTargets,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition)
{
	sourcePosition = srcPosition;
	
	inverterDelay = invDelay;
	maxDelay = 0;
	
	degree = maxInvFanout;
	maximumCellFanout = maxCellFanout;
	
	positiveTargetsLeft = posTargets;
	negativeTargetsLeft = negTargets;
	
	numTargets = posTargets+negTargets;
	positionedTargets = (TARGET*) malloc(numTargets*sizeof(TARGET));
	numPositionedTargets = 0;
	
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
	for (std::vector<INVERTER>::iterator it = inverters.begin() ; it != inverters.end(); ++it)
	{
		if(it->targets)
			free(it->targets);
	}
	free(levels);
	free(positionedTargets);
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
//Maybe add targets equally for critical targets
void InverterTree::add_critical_target(unsigned target,bool signal,float delay)
{
	//Allocate as close to the source as possible, always leaving a space to deliver the rest of the signals
	unsigned i;
	
	
	if(signal)
	{
		i=1; //Negative levels
		negativeTargetsLeft--;
	}
	else
	{
		i=0;// Positive levels
		positiveTargetsLeft--;
	}
	//printf("Height: %u\n",height);
	for(;i<height;i+=2)
	{
		//printf("Trying Target: %u at %u, vacant: %u\n",target,i,levels[i].vacant);
		if(levels[i].vacant>0)
		{
			//printf("1Added Target %u to level %u\n",target,i);
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
			//printf("2Added Target %u to level %u\n",target,i+1);
			return;
		}
	}
	//If code reaches this point, we need more levels!
	add_levels(2);
	i++;
	levels[i].vacant--;
	levels[i].signal_taken++;
	//printf("3Added Target %u to level %u\n",target,i);
	return;
	
}
void InverterTree::expand()
{
	//Expand until there is space for the rest, as determined by the targetLeft variables
	unsigned positiveAvailable =0;
	unsigned negativeAvailable =0;
	unsigned expanded;
	unsigned i;
	//Expand to provide for non criticals
	for(i=0;(positiveAvailable<positiveTargetsLeft)||(negativeAvailable<negativeTargetsLeft);i++)
	{
		if((i+1)>=height)
			add_levels(1);
		
		expanded = levels[i].vacant;
		if(expanded > 0)
		{
			if(i % 2)
			{
				//Next level (i+1) is positive
                //Remember that we allocated an extra slot for an inverter at the last level
				positiveAvailable = levels[i+1].vacant + (expanded*degree);
				if(positiveAvailable >= positiveTargetsLeft)
				{
					negativeAvailable = (positiveAvailable-positiveTargetsLeft)/degree;
					
					if(negativeAvailable < expanded)
						expanded -= negativeAvailable;
					else
						expanded = 0;
				}
			}
			else
			{
				//Next level (i+1) is negative
                //Remember that we allocated an extra slot for an inverter at the last level
                //We also need to change it so that we expand as much as possible if the next layer is supplied, but not enough will remain for the current layer anyway
				negativeAvailable = levels[i+1].vacant+(expanded*degree);
				if(negativeAvailable >= negativeTargetsLeft)
				{
					positiveAvailable = (negativeAvailable-negativeTargetsLeft)/degree;
					
					if(positiveAvailable < expanded)//Just in case there are several unpropagated available at the next layer
						expanded -= positiveAvailable;
					else
						expanded = 0;
				}
			}
			//printf("Expanded: %u\n",expanded);
			levels[i].vacant -= expanded;
			levels[i].inv_taken += expanded;
			levels[i+1].vacant += degree*expanded;
		}
	}
	
	//printf("\tEnd of expansion\n");
	//printf("\tLeft/Available: (+): %u/%u (-) %u/%u\n",positiveTargetsLeft,positiveAvailable,negativeTargetsLeft,negativeAvailable);

}
void InverterTree::add_non_critical_target(unsigned target,bool signal,float delay,Point position)
{
	//Allocate according to position
	//Make a list of all non critical targets that need to be reached and then determine which inverters feed which targets
	positionedTargets[numPositionedTargets].target = target;
	positionedTargets[numPositionedTargets].signal = signal;
	positionedTargets[numPositionedTargets++].position = position;
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
	float closest=positionedTargets[0].position.manhattanDistance(positionedTargets[1].position)+1;
	unsigned closesta,closestb; // closest pair
	for(unsigned i=0;i<numPositionedTargets;i++)
	{
		printf("Target(%c): %u (%.2f,%.2f)\n",positionedTargets[i].signal ? '-' : '+',positionedTargets[i].target,positionedTargets[i].position.x,positionedTargets[i].position.y);
		for(unsigned j=0;j<numPositionedTargets;j++)
		{//Change to j=i+1 eventually
			if(i!=j)
			{
				if(positionedTargets[i].position.manhattanDistance(positionedTargets[j].position)<closest)
				{
					closest = positionedTargets[i].position.manhattanDistance(positionedTargets[j].position);
					closesta = positionedTargets[i].target;
					closestb = positionedTargets[j].target;
				}
				printf("Distance to %u: %f\n",positionedTargets[j].target,positionedTargets[i].position.manhattanDistance(positionedTargets[j].position));
			}
		}
	}
	printf("Closest: %u - %u, distance: %f\n",closesta,closestb,closest);
}
void InverterTree::print()
{
	printf("Inv Tree of Height %u, Degree %u\n",height,degree);
	
	for(unsigned i=0;i<height;i++)
	{
		printf("Vacant: %u Inverters Fed: %u Signals Fed: %u\n",levels[i].vacant,levels[i].inv_taken,levels[i].signal_taken);
	}
}
void InverterTree::print_inverters()
{
	unsigned i=0,j;
	for (std::vector<INVERTER>::iterator it = inverters.begin() ; it != inverters.end(); ++it,i++)
	{
		printf("Inverter: %u: Inverters Fed: ",i);
		for(j=0;j<it->num_inv_targets;j++)
			printf("%u ",it->targets[j]);
		printf("Signals Fed: ");
		for(j=0;j<it->num_inv_targets;j++)
			printf("%u ",it->targets[degree-j]);
	}
}
int InverterTree::add_inverter(unsigned level)
{
	INVERTER inv;
	inv.targets = (unsigned*) malloc(degree*sizeof(unsigned));
	inv.num_inv_targets = 0;
	inv.num_vert_targets = 0;
	inv.level = level;
	inverters.push_back(inv);
}
int InverterTree::add_vert_target_to_inverter(unsigned target,unsigned inverter)
{
	INVERTER * inv;
	inv = &(inverters[inverter]);
	if((inv->num_vert_targets+inv->num_inv_targets)<degree)
	{
		inv->targets[inv->num_vert_targets++] = target;
	}
}
int InverterTree::add_inv_target_to_inverter(unsigned target,unsigned inverter)
{
	INVERTER * inv;
	inv = &(inverters[inverter]);
	if((inv->num_vert_targets+inv->num_inv_targets)<degree)
	{
		inv->targets[degree-inv->num_vert_targets++] = target;
	}
}
