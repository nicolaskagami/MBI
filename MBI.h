
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list>
#include<queue>

#include "Liberty.h"
#include "InverterTree.h"
#include "Topology.h"
 //Up to 10% 

#ifndef NET_ORDER
#define NET_ORDER 2 
#endif

#define MAX_LINE 255
#define MAX_NAME 255
#define MAX_SOURCES 2 
#define MAX_LABEL 32


typedef struct
{
    char name[MAX_LABEL];
    float period;
} CLOCK;


class MBI
{
    public:
        //MBI data
        unsigned max_cell_fanout;
        unsigned max_inv_fanout;
        float nodal_delay;
        float inv_delay;

        //MBI(char * paagFileName,char * sdcFileName,char * libFileName);
        MBI(int argc,char ** argv);
		~MBI();
		
		void estimate_delay();
        void insert_buffers();
        void insert_buffer(unsigned vert);
		void add_targets(unsigned vert);
        void print();
        
        //Sorting
        void sort_vert(VERT vert);
        void sort_vert(unsigned vert);
        void merge(EDGE * a,EDGE *aux,int left,int right,int rightEnd);
        void mSort(EDGE * a,EDGE *aux,int left,int right);

        //Parsers
		
		//Position input
		//Forward Star Graph
        VERT * vertices;
        unsigned num_vertices;
        EDGE * edges;
        unsigned num_edges;
		
		INPUT * inputs;
		unsigned num_inputs;
        OUTPUT * outputs;
		unsigned num_outputs;
		
        //PAAG
		Paag * paag;
        void parse_paag(char * paagFileName);
		void clean_paag();
		
		//DEF
        Def * def;
		void parse_def(char * defFileName);
        void clean_def();
		
        //SDC
        std::list<CLOCK> clocks;
        CLOCK current_clock;
        void parse_sdc(char * sdcFileName);
        void clean_sdc();
        void set_clock();
        
        //LIB
        Liberty * lib;
        void set_nodal_delay(char * cellName,char * invName);
		
		//InverterTree
		InverterTree * inverter_trees;
        
        //
		unsigned min_height(unsigned posConsumers,unsigned negConsumers);
        //void option1(unsigned vert);
        
    private:
        //Constructing auxiliary variables
        unsigned current_edge;
        unsigned current_vert;

         


};

