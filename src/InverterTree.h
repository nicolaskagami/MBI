
#include<stdlib.h>
#include<stdio.h>
#include<time.h>//For rand seed
#include<vector>
#include "Geometry.h"

//For CRIT_ALG 0 and 1
#ifndef CRITICAL_THRESHOLD
#define CRITICAL_THRESHOLD 0.95
#endif

//Critical Algorithms:
//0: Flat percentage
//1: Relative percentage
//2: Inverter Delay Relative Groups
#ifndef CRIT_ALG
#define CRIT_ALG 2 
#endif

//Non-Critical Algorithms:
//0: K means
//1: Worst First
#ifndef NON_CRIT_ALG
#define NON_CRIT_ALG 1 
#endif

//Non-Critical Placing
#ifndef NON_CRIT_PLACE
#define NON_CRIT_PLACE 0 
#endif

//Inverter Positioning
//0: Centroid
//1: Weighted Centroid
#ifndef INV_POS
#define INV_POS 0
#endif

typedef struct
{
    Point position;
	float post_delay;
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
    Point position;
    bool isVertex;//If true, this target is a vertex, otherwise it is an inv
    unsigned target;
	float post_delay;
} TARGET;

typedef struct
{
    unsigned vacant;
    unsigned inv_taken;
    unsigned signal_taken;
} LEVEL;

class InverterTree
{
	public:
        //InverterTree Data
		Point sourcePosition;
		float inverterDelay;
		float maxDelay;
		unsigned degree;
		unsigned maximumCellFanout;
	    
		//All targets and their associated levels
		TARGET * positiveTargets;
		unsigned * positiveLevels;
		unsigned numPositiveTargets;
		unsigned numPositiveCriticals;
		
		TARGET * negativeTargets;
		unsigned * negativeLevels;
		unsigned numNegativeTargets;
		unsigned numNegativeCriticals;

		//We keep the number of consumers left so as to allocate memory precisely	
		unsigned positiveConsumersLeft;
		unsigned negativeConsumersLeft;
		
        //First Inverter Tree Abstraction:
        //An array of levels, modelling the availability and occupation of signals
		LEVEL * levels;
		unsigned height;
		
		//Second Inverter Tree Abstraction:
        //A vector of inverters
		std::vector<INVERTER> positionedInverters;
		
        //Third Inverter Tree Abstraction:
        //Arrays of targets to attend according to location
		TARGET * currentPositiveTargets;
		unsigned numCurrentPositiveTargets;
		TARGET * currentNegativeTargets;
		unsigned numCurrentNegativeTargets;
		
		//Temporary Variables
        unsigned currentLayer;
        TARGET * targets; //Switches between currentPositiveTargets and currentNegativeTargets
        unsigned numTargets; //Switches between currentPositiveTargets and currentNegativeTargets
        TEMP_INVERTER * inverters; //Temporary inverters
        unsigned numInverters; //Temporary inverters
        unsigned sizeInvertersArray; //Size of temporary array
		
        //Functions
		InverterTree(unsigned positiveConsumers,unsigned negativeConsumers,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition);
		~InverterTree();
		
		//Main functions
		void connect();
		void expand();
		void prune();
		void connect_targets();
		void determine_max_delay();
		
		//Critical targets algorithms
		void place_criticals();
		void place_criticals_FlatPercent();
		void place_criticals_RelativePercent();
		void place_criticals_InvDifference();
		
		//Non critical targets algorithms
		void place_non_criticals();
		void place_non_criticals_Ordered();
		void place_non_criticals_Random();
		float non_critical_allocation();
        float non_critical_allocation_kmeans();
        float non_critical_allocation_worstFirst();
		
		//Inverter Positioning
        void position_inverter(TEMP_INVERTER * inv);
		void position_inverter_centroid(TEMP_INVERTER * inv);
		void position_inverter_ponderateCentroid(TEMP_INVERTER * inv);
		
		void collect_targets(unsigned level);
		void add_levels(unsigned newLevels);
		void determine_level(unsigned tgt_index, unsigned level);
		
		void add_critical_target(unsigned target_index,bool signal);
        void add_negative_target(unsigned target,bool isVertex,float delay,Point position);
        void add_positive_target(unsigned target,bool isVertex,float delay,Point position);
		void add_current_negative_target(unsigned target,bool isVertex,float delay,Point position);
        void add_current_positive_target(unsigned target,bool isVertex,float delay,Point position);
		unsigned min_height(unsigned posConsumers,unsigned negConsumers);
		unsigned consolidate_inverter(TARGET * target_list,TEMP_INVERTER temp_inv);
		void print_inverters();
		void print();
        //Debug
        bool Debug;
        void verify();
        
        
};
