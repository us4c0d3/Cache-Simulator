#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h> 
#include <math.h> // used to help calculate set and block 
#include "cachelab.h"

/*
 * Simulates a cache logic with a write-back and LRU (least recently used) policy.
 * Handles direct-mapped, set-associative and full-associative caches. 
 */

///////////////  structures ///////////////

struct cache_setting{
	int s; // set bits
	int S; // represents how many sets in a cache. S = 2**s
	int b; // data block bits
	int E; // represents how many lines in a set.
}cache;

struct Line {
	unsigned int valid;
	unsigned int tag;
	unsigned int lru;
};

typedef struct Line Line;
typedef long unsigned int mem_addr;

void print_Usage( char *argv[] ) {
   printf( "\nUsage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0] );
   printf( "Options:\n" );
   printf( "  -h         Print this help message.\n" );
   printf( "  -v         Optional verbose flag.\n" );
   printf( "  -s <num>   Number of set index bits.\n" );
   printf( "  -E <num>   Number of lines per set.\n" );
   printf( "  -b <num>   Number of block offset bits.\n" );
   printf( "  -t <file>  Trace file.\n\n" );
   printf( "Examples:\n" );
   printf( "  linux> %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0] );
   printf( "  linux> %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n\n", argv[0] );
   exit( 0 );
}

/////////////// Foward Declaration //////////////

Line **makeCache(int S, int E);
unsigned int getInfo(mem_addr a, unsigned int *set);
void checkCache(Line **my_cache, unsigned int set, unsigned int tag, unsigned int *hits, unsigned int *misses, unsigned int *evictions);
void update_lru(Line *set, int line);

int main(int argc, char** argv) {

	char *set_verify; 
	char *trace_file; // trace file that will be passed into our cache simulator
	char option; // variable used for our switch operation
        int set_check = 0; // checks to see if -s was passed
        int line_check = 0; // checks to see if -E was passed
        int block_check = 0; // checks to see if -b was passed
    	int trace_check = 0; // checks to see if -t was passed

	// getopt checks for which flags are being passed and gets the value.
	while ((option = getopt(argc, argv, "s:E:b:t:h::")) != -1){
		switch(option){
			case 's':
				set_verify = optarg; // initialize var if its NULL	 
				cache.s = atoi(optarg);
				cache.S = pow(2, cache.s); // calculates the # of sets
                         	set_check = 1;
				break;

			case 'E':
				cache.E = atoi(optarg);
				line_check = 1;
				break;

			case 'b':
				cache.b = atoi(optarg);
                                block_check = 1;
				break;
				
			case 't':
				trace_file = optarg; // string of the trace filename
				trace_check = 1;
				break;

			case 'h':
				print_Usage(argv);

			default:
				print_Usage(argv);
		}
	}
	// check to see if all required arguments were passed to command line
        if ( set_check == 0 || line_check == 0 || block_check == 0 || trace_check == 0) {
        	print_Usage(argv);
        }

	// check to see if values are not NULL or 0
	if (set_verify == NULL || cache.E == 0 || cache.b == 0 || trace_file == NULL) {
        	printf("%s: Missing required command line argument\n", argv[0]);
        	print_Usage(argv);
	}

	//if no values were inputted
	if(argv[1] == NULL){
		print_Usage(argv);
	}

	// local variables
	char c;
	int d;
	typedef long unsigned int mem_addr;
	mem_addr addr;
	unsigned int tag;
	unsigned int set;
	unsigned int hits = 0;
	unsigned int misses = 0;
	unsigned int evictions = 0;
	
	// initializes cache
	Line **my_cache = makeCache(cache.S, cache.E);
	FILE *trace = fopen(trace_file, "r"); // read in file
	int ret = fscanf(trace, " %c %lx,%d", &c, &addr, &d); 
	while (ret != -1) {
		tag = getInfo(addr, &set);
		if(c == 'L' || c == 'S') checkCache(my_cache, set, tag, &hits, &misses, &evictions);
		// modify is equivalent to load and store. accesses twice
		if(c == 'M') {
			checkCache(my_cache, set, tag, &hits, &misses, &evictions);
			checkCache(my_cache, set, tag, &hits, &misses, &evictions);
		}
		ret = fscanf(trace, " %c %lx,%d", &c, &addr, &d);
	}
	fclose(trace);
	printSummary(hits, misses, evictions);
	return 0;
}

/*
 * Function allocates memory for a cache with S sets and E lines.
 * @param S - Sets in cache
 * @param E - Lines per Set
 * @return - 2D array of lines
 */
Line **makeCache(int S, int E) {
	int i;
	int j;
	Line** cache1 = malloc(sizeof(Line*)*S);
	for(i = 0; i < S; i++) {
		cache1[i] = malloc(sizeof(Line)*E);
		for(j = 0; j < E; j++) {
			cache1[i][j].valid = 0;
			cache1[i][j].lru = j;
		}
	}
	return cache1;
}

/*
 * Function looks for tag in a line, updates valid and lru, counts hits/miss/eviction
 * @param my_cache - cache structure
 * @param set - set number
 * @param tag - tag we are looking for
 * @param hits - number of times we get a hit
 * @param misses - number of times we get a miss
 * @param evictions - number of times we replace a value
 */
void checkCache(Line **my_cache, unsigned int set, unsigned int tag, 
unsigned int *hits,  unsigned int *misses, unsigned int *evictions) {
	int i;
	for(i = 0; i < cache.E; i++) {
		if(my_cache[set][i].valid != 0 && tag == my_cache[set][i].tag) {
			*hits = *hits + 1;
			update_lru(my_cache[set], i);
			return;
		}
		
	}
	for(i = 0; i < cache.E; i++) {
		if(my_cache[set][i].lru == (cache.E - 1)) {
			if(my_cache[set][i].valid != 0) *evictions = *evictions + 1;
			my_cache[set][i].valid = 1;
			my_cache[set][i].tag = tag;
			update_lru(my_cache[set], i);
			*misses = *misses + 1;
			return;
		}
	}
}

/* Function gets set number and tag from address
 * @param a - memory address to get info from
 * @param set - variable to hold returned set number
 * @return - the tag
 */
unsigned int getInfo(mem_addr a, unsigned int *set) {
	*set = (a<<(32 - (cache.s + cache.b)))>>(32 - cache.s);
	return a>>(cache.s + cache.b);
}

/*
 * Function updates lru bit
 * @param set - set being updated
 * @param line - line being accessed
 */
void update_lru(Line *set, int line) {
	int i;
	for(i = 0; i < cache.E; i++){
		if(i != line){
			if(set[i].lru < set[line].lru){
				set[i].lru = set[i].lru++;
			}
		}
	}
	set[line].lru = 0;
}