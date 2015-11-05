
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

class Liberty
{
    public:
        //LIB
		std::list<TIMING_LUT> time_luts;
		std::list<POWER_LUT> power_luts;
		std::list<CELL> cells;
		std::list<VOLT_MAP> voltage_maps;
		std::list<WIRE_LOAD> wire_loads;
        WIRE_LOAD * default_wire_load;
		float nom_process,nom_temperature, nom_voltage;
		
        Liberty(char * libFileName);  
		void print();
		~Liberty();
};
