#include "InverterTree.h"


bool Point::operator== (Point b)
{ 
    if((x == b.x)&&(y == b.y))
        return true; 
    else 
        return false;
}
InverterTree::InverterTree(unsigned posTargets,unsigned negTargets,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition)
{
	sourcePosition = srcPosition;
	
	inverterDelay = invDelay;
	maxDelay = 0;
	
	degree = maxInvFanout;
	maximumCellFanout = maxCellFanout;
	
	numTargets = posTargets+negTargets;
	positionedTargets = (TARGET*) malloc(numTargets*sizeof(TARGET));
	numPositionedTargets = 0;
	
	height = min_height(posTargets,negTargets);
	if(height == 0)
		height = 1;
	
	
	levels=(LEVEL*) malloc(height*sizeof(LEVEL));
	for(unsigned i=0;i<height;i++)
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
	}
	else
	{
		i=0;// Positive levels
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
	//First let's look at the lower levels, to see if we can propagate some space!
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
	unsigned expanded;
	for(unsigned i=0;(i+2)<height;i++)//Expand to next to last level, last level will add inverters
	{
		expanded = levels[i].vacant;
		if(expanded>0)
		{
			levels[i].vacant = 0;
			levels[i].inv_taken+=expanded;
			levels[i+1].vacant+=degree*expanded;
		}
	}
		
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
    while((leaves > leavesAvailable)||(height1_branches > ((leavesAvailable-leaves)/degree))) ;
    return minHeight;
}
void InverterTree::connect_positioned_targets()
{
	for(unsigned i=0;i<numPositionedTargets;i++)
	{
		printf("Target(%c): %u (%.2f,%.2f)\n",positionedTargets[i].signal ? '-' : '+',positionedTargets[i].target,positionedTargets[i].position.x,positionedTargets[i].position.y);
		
	}
}
void InverterTree::print()
{
	printf("Inv Tree of Height %u, Degree %u\n",height,degree);
	
	for(unsigned i=0;i<height;i++)
	{
		printf("Vacant: %u Inverters Fed: %u Signals Fed: %u\n",levels[i].vacant,levels[i].inv_taken,levels[i].signal_taken);
	}
}

int InverterTree::add_inverter()
{
	INVERTER inv;
	inv.targets = (unsigned*) malloc(degree*sizeof(unsigned));
	inv.num_inv_targets = 0;
	inv.num_vert_targets = 0;
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
