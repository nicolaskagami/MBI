
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list>
#include<queue>

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
    Point position;
    unsigned num_srcs;
    unsigned srcs[MAX_SOURCES];

    float pre_delay;//can be placed at the target vertex
    float post_delay;

    unsigned pindex;
    unsigned positive_targets;
    unsigned nindex;
    unsigned negative_targets;

}VERT;
typedef struct 
{
    unsigned target;

    float path_delay;
    unsigned level;
}EDGE;

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

//LIB data
typedef struct
{
    char name[MAX_NAME];
	char var1[MAX_NAME];
	char var2[MAX_NAME];
	unsigned num_indices;
	float * ind1;
	float * ind2;
} TIMING_LUT;

typedef struct
{
    char name[MAX_NAME];
	char var1[MAX_NAME];
	char var2[MAX_NAME];
	unsigned num_indices;
	float * ind1;
	float * ind2;
} POWER_LUT;

typedef struct
{
    char when[MAX_NAME];
    float value;
} LEAKAGE_POWER;

typedef struct
{
	char name[MAX_LABEL];
	float voltage;
} VOLT_MAP;

typedef struct 
{
    char name[MAX_LABEL];
    VOLT_MAP * volt_map;
    char type[MAX_NAME];
} PG_PIN;

typedef struct
{
	int related_power_pin;//index of cell's pg_pins
	int related_ground_pin;//index of cell's pg_pins
	float capacitance;
	float fall_capacitance;
	float rise_capacitance;
	char name[MAX_NAME];
} INPUT_PIN;

typedef struct
{
	char timing_lut[MAX_NAME];
	float index1;
	float index2;
	float values;
} TIMING_CHARACTER;

typedef struct
{
	char timing_sense[MAX_NAME];
	char related_pin[MAX_LABEL];
	TIMING_CHARACTER cell_fall;
	TIMING_CHARACTER cell_rise;
	TIMING_CHARACTER fall_transition;
	TIMING_CHARACTER rise_transition;
} PIN_TIMING_PROFILE;

typedef struct
{
	char power_lut[MAX_NAME];
	float index1;
	float index2;
	float values;
} POWER_CHARACTER;

typedef struct
{
	char related_pin[MAX_LABEL];
	POWER_CHARACTER fall_power;
	POWER_CHARACTER rise_power;
} PIN_POWER_PROFILE;

typedef struct
{
	int related_power_pin;//index of cell's pg_pins
	int related_ground_pin;//index of cell's pg_pins
	float max_capacitance;
	char function[MAX_NAME];
	char name[MAX_NAME];
	std::list<PIN_POWER_PROFILE> power_profiles;
	std::list<PIN_TIMING_PROFILE> timing_profiles;
} OUTPUT_PIN;

typedef struct
{
	char name[MAX_LABEL];
	float capacitance;
	float resistance;
	float slope;
	unsigned num_indices;
	unsigned * fanout;
	float * length;
} WIRE_LOAD;

typedef struct
{
    char name[MAX_NAME];
	unsigned drive_strength;
	float area;
	float cell_leakage_power;
    std::list<PG_PIN> pg_pins;
    std::list<LEAKAGE_POWER> leakage_powers;
	std::list<INPUT_PIN> input_pins;
	std::list<OUTPUT_PIN> output_pins;
} CELL;

class MBI
{
    public:
        VERT * vertices;
        unsigned num_vertices;
        EDGE * edges;
        unsigned num_edges;

        unsigned max_cell_fanout;
        unsigned max_inv_fanout;
        float unit_delay;

        MBI(unsigned v,unsigned e);
        MBI(char * paagFileName,char * sdcFileName,char * libFileName);
        ~MBI();
        int allocate_memory(unsigned v, unsigned e);
        void preallocate(unsigned src,unsigned tgt,bool signal);
        void indexify();
        void add_edge(unsigned src,unsigned tgt,bool signal);
        void set_position(unsigned vert,unsigned x,unsigned y);
        //void set_delay(unsigned vert,float delay);
      
        void estimate_delay();
        void insert_buffers();
        void print();

        //Parser
        //PAAG
        unsigned M,I,L,O,A,X,Y;
        INPUT * paag_inputs;
        OUTPUT * paag_outputs;
        void parse_paag(char * paagFileName);
		void clean_paag();
        //SDC
        std::list<CLOCK> clocks;
        void parse_sdc(char * sdcFileName);
		void clean_sdc();
        //LIB
		std::list<TIMING_LUT> time_luts;
		std::list<POWER_LUT> power_luts;
		std::list<CELL> cells;
		std::list<VOLT_MAP> voltage_maps;
		std::list<WIRE_LOAD> wire_loads;
        WIRE_LOAD * default_wire_load;
		float nom_process,nom_temperature, nom_voltage;
        void parse_lib(char * libFileName);  
		void print_lib();
		void clean_lib();
		//
        void option1(unsigned vert);
    private:
        //Constructing auxiliary variables
        unsigned current_edge;
        unsigned current_vert;

         


};
