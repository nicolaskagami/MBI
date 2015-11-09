
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list>
#include<queue>
#include "Liberty.h"

#define CRITICAL_THRESHOLD 0.5 //Up to 10% 

#define MAX_LINE 255
#define MAX_NAME 255
#define MAX_SOURCES 2 
#define MAX_LABEL 32

class Point
{
    public:
        unsigned x;
        unsigned y;

        bool operator== (Point b);
};
typedef struct
{
	//Vertex Position
    Point position;
    unsigned num_srcs;
    unsigned srcs[MAX_SOURCES];

	//Vertex Delay Data
    float pre_delay;
    float post_delay;

	//Vertex Edges
    unsigned pindex;
    unsigned positive_targets;
    unsigned nindex;
    unsigned negative_targets;
	unsigned num_positive_critical;
	unsigned num_negative_critical;
}VERT;

typedef struct 
{
    unsigned target;
    float path_delay;
    unsigned level;
} EDGE;

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

//PAAG data
typedef struct
{
    unsigned index;
    char name[MAX_LABEL];
    float delay;
} INPUT;

typedef struct
{
    unsigned index;
    char name[MAX_LABEL];
    float max_delay;
} OUTPUT;

typedef struct
{
    char name[MAX_LABEL];
    float period;
} CLOCK;


class MBI
{
    public:
		//Forward Star Graph
        VERT * vertices;
        unsigned num_vertices;
        EDGE * edges;
        unsigned num_edges;
		
		int allocate_memory(unsigned v, unsigned e);
        void preallocate(unsigned src,unsigned tgt,bool signal);
        void indexify();
        void add_edge(unsigned src,unsigned tgt,bool signal);
        void set_position(unsigned vert,unsigned x,unsigned y);

		//MBI data
        unsigned max_cell_fanout;
        unsigned max_inv_fanout;
        float nodal_delay;
		float inv_delay;

        MBI(char * paagFileName,char * sdcFileName,char * libFileName);
        ~MBI();
        void print();
        
        void estimate_delay();
        void insert_buffers();
		void select_critical(unsigned vert);
		
        //Sorting
        void sort_vert(VERT vert);
		void sort_vert(unsigned vert);
        void merge(EDGE * a,EDGE *aux,int left,int right,int rightEnd);
        void mSort(EDGE * a,EDGE *aux,int left,int right);

        //Parsers
        //PAAG
        unsigned M,I,L,O,A,X,Y;
        INPUT * paag_inputs;
        OUTPUT * paag_outputs;
        void parse_paag(char * paagFileName);
		void clean_paag();
		
        //SDC
        std::list<CLOCK> clocks;
		CLOCK current_clock;
        void parse_sdc(char * sdcFileName);
		void clean_sdc();
        void set_clock();
		
        //LIB
		Liberty * lib;
        void set_nodal_delay(char * cellName,char * invName);
		
		//
        void option1(unsigned vert);
		
    private:
        //Constructing auxiliary variables
        unsigned current_edge;
        unsigned current_vert;

         


};

