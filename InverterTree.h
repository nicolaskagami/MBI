
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
	bool signal;
    unsigned target;
} TARGET;

class InverterTree
{
	public:
		Point sourcePosition;
		float inverterDelay;
		float maxDelay;
		
		unsigned degree;
		unsigned maximumCellFanout;
		
		unsigned positiveTargetsLeft;
		unsigned negativeTargetsLeft;
		
		TARGET * positionedTargets;
		unsigned numPositionedTargets;
		unsigned numTargets;
		
		unsigned height;
		LEVEL * levels;
		
		std::vector<INVERTER> inverters;
		
		InverterTree(unsigned positiveTargets,unsigned negativeTargets,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition);
		~InverterTree();
		void add_levels(unsigned newLevels);
		void add_critical_target(unsigned target,bool signal,float delay);
		void expand();
		void add_non_critical_target(unsigned target,bool signal,float delay,Point position);
		unsigned min_height(unsigned posConsumers,unsigned negConsumers);
		void connect_positioned_targets();
		
		int add_inverter(unsigned level);
		int add_vert_target_to_inverter(unsigned target,unsigned inverter);
		int add_inv_target_to_inverter(unsigned target,unsigned inverter);
		void print_inverters();
		void print();

};
