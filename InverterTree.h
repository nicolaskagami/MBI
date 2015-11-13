
#include<stdlib.h>
#include<stdio.h>
#include<vector>

class Point
{
    public:
        float x;
        float y;

        bool operator== (Point b);
		float manhattanDistance(Point b);
};

typedef struct
{
	float minx;
	float miny;
	float maxx;
	float maxy;
	
} BOUNDING_BOX;

typedef struct
{
    Point position;
    unsigned * targets; //Both invs and verts
    unsigned num_inv_targets;
    unsigned num_vert_targets;
	unsigned level;
    
    //Needs edge super malloc
    //Consumers as indices? how about other inverters?
    //More espace - Less time:
    //  Inverter consumers separate from vertices
    //  unsigned * vert_targets = &targets[num_inv_targets];
	//One contiguous block
	//	Targets ->
	//  <- Inverters
        
} INVERTER;

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
		unsigned numTargets;
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
		std::vector<INVERTER> inverters;
		
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
		
		int add_inverter(unsigned level);
		int add_vert_target_to_inverter(unsigned target,unsigned inverter);
		int add_inv_target_to_inverter(unsigned target,unsigned inverter);
		void print_inverters();
		void print();

};
