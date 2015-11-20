
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list>
#include<queue>
#include "Geometry.h"

#define MAX_LINE 255
#define MAX_NAME 255
#define MAX_SOURCES 2
#define MAX_LABEL 32

class InverterTree;
class Point;

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
    InverterTree * inverter_tree;
}VERT;

typedef struct 
{
    unsigned target;
    float path_delay;
    unsigned level;
} EDGE;

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


class Topology
{
	public:
		VERT * vertices;
		unsigned num_vertices;
		EDGE * edges;
		unsigned num_edges;
		
		INPUT * inputs;
		unsigned num_inputs;
        OUTPUT * outputs;
		unsigned num_outputs;
		
		Topology();
		int allocate_memory(unsigned v, unsigned e, unsigned I, unsigned O);
        void preallocate(unsigned src,unsigned tgt,bool signal);
        void indexify();
        void add_edge(unsigned src,unsigned tgt,bool signal);
        void set_position(unsigned vert,float x,float y);
		
};


//PAAG data

class Paag
{
    public:
        //Paag
		Topology * topology;
		unsigned M,I,L,O,A,X,Y;
		
		
        Paag(char * paagFileName);  
		~Paag();
		void print();
		
	private:
		
};
