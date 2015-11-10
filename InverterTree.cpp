#include "InverterTree.h"


bool Point::operator== (Point b)
{ 
    if((x == b.x)&&(y == b.y))
        return true; 
    else 
        return false;
}
InverterTree::InverterTree(unsigned minHeight,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition)
{
	sourcePosition = srcPosition;
	inverterDelay = invDelay;
	degree = maxInvFanout;
	height = minHeight;
	if(height == 0)
		height = 1;
	maxDelay = 0;
	
	levels=(LEVEL*) malloc(minHeight*sizeof(LEVEL));
	printf("Height: %u,min: %u\n",height,minHeight);
	for(unsigned i=0;i<height;i++)
	{
		levels[i].vacant = degree - 1;
		levels[i].inv_taken = 1;
		levels[i].signal_taken = 0;
	}
}
InverterTree::~InverterTree()
{
	free(levels);
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
	i+=3;
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
	//if(signal)
		//negative
	//else
		//Positive
	
	//if(.
}
void InverterTree::print()
{
	printf("Inv Tree of Height %u, Degree %u\n",height,degree);
	
	for(unsigned i=0;i<height;i++)
	{
		printf("Vacant: %u Inverters Fed: %u Signals Fed: %u\n",levels[i].vacant,levels[i].inv_taken,levels[i].signal_taken);
	}
}