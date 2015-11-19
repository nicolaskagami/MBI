
#include<stdlib.h>
#include<stdio.h>
#include<time.h>//For rand seed
#include<cmath>//For abs
#include<vector>

#ifndef NON_CRIT_ALG
#define NON_CRIT_ALG 0 
#endif


class Point
{
    public:
        float x;
        float y;

        bool operator== (Point b);
		float distance(Point b);
};

typedef struct
{
    Point position;
    unsigned * targets; //Both invs and verts
    unsigned num_inv_targets;
    unsigned num_vert_targets;
    
    //More espace - Less time:
    //  Inverter consumers separate from vertices, could be two dynamically alloc'd arrays, (double the memory)
    //  unsigned * vert_targets = &targets[num_inv_targets];
	//One contiguous block (better spatial locality
	//	Targets ->
	//  <- Inverters
        
} INVERTER;

typedef struct
{
	Point position;
	unsigned * targets_indexes;
	unsigned num_targets;
} TEMP_INVERTER; //Structure to increase performance when building inverters

typedef struct
{
    unsigned vacant;
    unsigned inv_taken;
    unsigned signal_taken;
} LEVEL;

typedef struct
{
    Point position;
    bool isVertex;//If true, this target is a vertex, otherwise it is an inv
    unsigned target;
} TARGET;

class InverterTree
{
	public:
        //InverterTree Data
		Point sourcePosition;
		float inverterDelay;
		float maxDelay;
		unsigned degree;
		unsigned maximumCellFanout;
	    //We keep the number of consumers left so as to allocate memory precisely	
		unsigned positiveConsumersLeft;
		unsigned negativeConsumersLeft;

        //First Inverter Tree Abstraction:
        //An array of levels, modelling the availability and occupation of signals
		LEVEL * levels;
		unsigned height;
		
        //Second Inverter Tree Abstraction:
        //Arrays of targets to attend according to location
		TARGET * positiveTargets;
		unsigned numPositiveTargets;
		TARGET * negativeTargets;
		unsigned numNegativeTargets;
		
	    //Third Inverter Tree Abstraction:
        //A vector of inverters
		std::vector<INVERTER> positionedInverters;

        //Temporary Variables
        unsigned currentLayer;
        TARGET * targets;
        unsigned numTargets;
        TEMP_INVERTER * inverters;
        unsigned numInverters;
        unsigned sizeInvertersArray;
		
        //Functions
		InverterTree(unsigned positiveConsumers,unsigned negativeConsumers,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition);
		~InverterTree();
		void add_levels(unsigned newLevels);
		void add_critical_target(unsigned target,bool signal,float delay);
		void expand();
        void add_negative_target(unsigned target,bool isVertex,float delay,Point position);
        void add_positive_target(unsigned target,bool isVertex,float delay,Point position);
		unsigned min_height(unsigned posConsumers,unsigned negConsumers);
		void connect_positioned_targets();
		
		unsigned consolidate_inverter(TARGET * target_list,TEMP_INVERTER temp_inv);
		void print_inverters();
		void print();
        //Non Critical Allocation
        void non_critical_allocation();
        void non_critical_allocation_kmeans();
        void non_critical_allocation_worstFirst();
        //Inverter Positioning
        void position_inverter(TEMP_INVERTER * inv);
};
