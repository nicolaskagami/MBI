
#include<stdlib.h>
#include<stdio.h>
class Point
{
    public:
        unsigned x;
        unsigned y;

        bool operator== (Point b);
};

typedef struct
{
    Point position;
    unsigned * targets; //Both invs and verts
    unsigned num_inv_targets;
    unsigned num_vert_targets;
    
    //Needs edge super malloc
    //Consumers as indices? how about other inverters?
    //More espace - Less time:
    //  Inverter consumers separate from vertices
        
} INVERTER;

typedef struct
{
    unsigned vacant;
    unsigned inv_taken;
    unsigned signal_taken;
} LEVEL;

class InverterTree
{
	public:
		Point sourcePosition;
		float inverterDelay;
		float maxDelay;
		unsigned degree;
		unsigned height;
		LEVEL * levels;
		
		InverterTree(unsigned minHeight,unsigned maxCellFanout,unsigned maxInvFanout,float invDelay,Point srcPosition);
		~InverterTree();
		void add_levels(unsigned newLevels);
		void add_critical_target(unsigned target,bool signal,float delay);
		void expand();
		void add_non_critical_target(unsigned target,bool signal,float delay,Point position);
		
		void print();

};
