/*ID: 516021910333 name: Huang Ge*/

#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h> //atoi() malloc()
#include <string.h>
#include <math.h> //pow()
#include <unistd.h> 
#include <getopt.h> //getopt()

#define MAX_LINE_SIZE 1000
typedef unsigned long long int addr_t; //address type

/*cache line type*/
typedef struct line{
	int valid;
	addr_t tag;
	int time;
}line_t;
/*cache set type*/
typedef line_t *set_t;
/*cache type*/
typedef set_t *cache_t;

/*initial global values*/
int verbosity = 0;
int miss = 0;
int hit = 0;
int evict = 0;
int s = 0; //s bits for set index;
int S = 0; //set index; S = 2^s;
int E = 0; //associativity: E lines per set;
int b = 0; //b bits for line offset;
int B = 0; //B bytes per line; B = 2^b;
int t = 0; //t bits for tag;
//observation: in test cases one operation only invlove one line, size is useless;
char *trace_file_path = NULL;
int now = 0; //current time;
cache_t cache;

/*-h flag: print usage info*/
void print_usage_info(char *program_path)
{
	printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", program_path);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", program_path);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", program_path);
    exit(0);
}

/*search cache for address, if not found, update a line according to LRU*/
int search_update(addr_t addr)
{
	addr_t set = (addr << t) >> (t + b);
	addr_t tag = addr >> (s + b);

	/*search cache*/
	int i;
	for(i = 0; i < E; i++)
	{
		if((cache[set][i].tag == tag) && (cache[set][i].valid))
		{
			cache[set][i].time = now++;
			hit++;
			return 1;
		}
	}
	/*not found, search invalid line to fill the data*/
	miss++;
	for(i = 0; i < E; i++)
	{
		if(!cache[set][i].valid)
		{
			cache[set][i].time = now++;
			cache[set][i].valid = 1;
			cache[set][i].tag = tag;
			return 2;
		}
	}
	/*no invalid line, update a line according to LRU*/
	evict++;
	int min_time = cache[set][0].time;
	int evict_line = 0;
	for(i = 1; i < E; i++)
	{
		if(cache[set][i].time < min_time)
		{
			min_time = cache[set][i].time;
			evict_line = i;
		}
	}
	cache[set][evict_line].time = now++;
	cache[set][evict_line].tag = tag;
	return 3;
}


int main(int argc, char *argv[])
{
    int i, j;
    /*getopt() : parse commamd line options*/
    int opt;
    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) //optstring:"opt:" indicates a following parament;
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg); //optarg:the global pointer to current parament set by getopt(); atoi(): string(content) to int;
            S = pow(2, s);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            B = pow(2, b);
            break;
        case 't':
            trace_file_path = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            print_usage_info(argv[0]);
            exit(0);
        default:
            print_usage_info(argv[0]);
            exit(1);
        }
    }
    t = 64 - s - b;

    /*in case of missing necessary option*/
    if(s == 0 || E == 0 || b == 0 || trace_file_path == NULL)
    {
    	printf("%s: Missing required command line argument\n", argv[0]);
        print_usage_info(argv[0]);
        exit(1);
    }

    /*in case of wrong option*/
    if(s < 0) {printf("%s: Illegal negative -s parament\n", argv[0]); exit(1);};
    if(E < 0) {printf("%s: Illegal negative -E parament\n", argv[0]); exit(1);};
    if(b < 0) {printf("%s: Illegal negative -b parament\n", argv[0]); exit(1);};

    /*initialize cache*/
    cache = malloc(S * sizeof(set_t)); //malloc(): distribute space for pointer, note that multilevel pointer should malloc() level by level
	for(i = 0; i < S; i++)
	{
		cache[i] = malloc(E * sizeof(line_t));
		for(j = 0; j < E; j++) cache[i][j].valid = 0;
	}

	/*open trace file*/
	FILE *fp = fopen(trace_file_path, "r");
	if(fp == NULL)
	{
		printf("Cannot open file\n");
		exit(1);
	}

	/*simulate cache behavior*/
	char tmp[MAX_LINE_SIZE];
	addr_t addr;
	int size;

	while(fgets(tmp, MAX_LINE_SIZE, fp) != NULL) //do not use feof XD
    {
        if(tmp[1] == 'S' || tmp[1] == 'L')
        {
        	sscanf(tmp + 3, "%llx,%u", &addr, &size);

        	int res = search_update(addr);
        	if(verbosity)
        	{
        		printf("%c %llx,%u ", tmp[1], addr, size);
        		switch(res)
        		{
        			case 1: {printf("hit\n"); break;}
        			case 2: {printf("miss\n"); break;}
        			case 3: {printf("miss eviction\n"); break;}
        		}

        	}
        }
        if(tmp[1] == 'M')
        {
        	sscanf(tmp + 3, "%llx,%u", &addr, &size);

        	int res = search_update(addr);
        	if(verbosity)
        	{
        		printf("%c %llx,%u ", tmp[1], addr, size);
        		switch(res)
        		{
        			case 1: {printf("hit "); break;}
        			case 2: {printf("miss "); break;}
        			case 3: {printf("miss eviction "); break;}
        		}

        	}

        	res = search_update(addr);
        	if(verbosity)
        	{
        		switch(res)
        		{
        			case 1: {printf("hit\n"); break;}
        			case 2: {printf("miss\n"); break;}
        			case 3: {printf("miss eviction\n"); break;}
        		}

        	}
        }
    }
    fclose(fp);

    /*free*/
    for (i = 0; i < S; i++)
    {
        free(cache[i]);
    }
    free(cache);

    printSummary(hit, miss, evict);
    return 0;
}

