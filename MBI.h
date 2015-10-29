
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_LINE 255

class Point
{
    public:
        int x;
        int y;

        bool operator== (Point b);
};
typedef struct vertex
{
    Point position;
    unsigned delay;//can be placed at the target vertex

    unsigned pindex;
    unsigned positive_targets;
    unsigned nindex;
    unsigned negative_targets;

}VERT;
typedef struct edge 
{
    unsigned target;

    unsigned path_delay;
    unsigned level;
}EDGE;

typedef struct level
{
    unsigned vacant;
    unsigned inv_taken;
    unsigned signal_taken;
} LEVEL;

class MBI
{
    public:
        VERT * vertices;
        unsigned num_vertices;
        EDGE * edges;
        unsigned num_edges;

        unsigned max_cell_fanout;
        unsigned max_inv_fanout;

        MBI(unsigned v,unsigned e);
        MBI(char *paagFileName);
        ~MBI();
        int allocate_memory(unsigned v, unsigned e);
        void preallocate(unsigned src,unsigned tgt,bool signal);
        void indexify();
        void add_edge(unsigned src,unsigned tgt,bool signal);
        void set_delay(unsigned vert,unsigned delay);
        void print();
        //Parse Files
        void parse_paag(char * paagFileName);
        //
        void option1(unsigned vert);
    private:
        //Constructing auxiliary variables
        unsigned current_edge;
        unsigned current_vert;

         


};
