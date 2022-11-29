#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#define MISS_PENALTY 200
#define MAX_LINE 250000         // 1MB / 4B
#define MAX_BLOCK 256
#define MAX_WORD_IN_ONE_LINE 64 // 256 / 4

typedef unsigned char BYTE;     // 8-bit byte
typedef unsigned int WORD;      // 32-bit word == 4 byte

#define MALLOC_NODE(p, s) if(!((p) = (nodePointer)malloc(s))) { \
	fprintf(stderr, "Insufficient Memory"); \
	exit(EXIT_FAILURE); \
	}
#define MALLOC_CACHE(p, s) if(!((p) = (cachePointer)malloc(s))) { \
	fprintf(stderr, "Insufficient Memory"); \
	exit(EXIT_FAILURE); \
	}

typedef struct _node* nodePointer;
typedef struct cache_line* cachePointer;

/* cache structure */
typedef struct cache_line {
    WORD tag;
    bool valid;
    bool dirty;
    BYTE offset;
    BYTE data[MAX_WORD_IN_ONE_LINE];
    int time;
} Cache_line;

/* linked list node for memory */
typedef struct _node {
    Cache_line cache;
    struct _node *next;
} node;


/* global variables */
int t_hit, t_miss, t_cycle, t_dirty, t_memory_access;
Cache_line* cache_lines[MAX_LINE];


/* functions */
void write(WORD mem_addr, int data, BYTE offset_size);
void read(WORD mem_addr);
int getBitSize(BYTE num);
int power(int num, int n);
int getBits(WORD mem_addr, BYTE size);



int main(int argc, char* argv[]) {
    int i;
    double miss_rate, average_memory_access_cycle;
    char* temp;

    int lines, index_size;
    BYTE cache_size, set_size, block_size, offset_size, index_bits;
    char* filename;
    FILE* fp = NULL;

    WORD mem_addr;
    char inst;
    int data;

    /* get parameter */
    for(i = 1; i < argc; ++i) {
        switch (argv[i][1]) {
            case 's':   // cache_size
                temp = strtok(argv[i], "=");
                temp = strtok(NULL, "\0");
                cache_size = atoi(temp);
                printf("cache_size = %d\n", cache_size);
                break;
            case 'a':   // set_size
                temp = strtok(argv[i], "=");
                temp = strtok(NULL, "\0");
                set_size = atoi(temp);
                printf("set_size = %d\n", set_size);
                break;
            case 'b':   // block_size
                temp = strtok(argv[i], "=");
                temp = strtok(NULL, "\0");
                block_size = atoi(temp);
                offset_size = getBitSize(block_size);
                printf("block_size = %d\n", block_size);
                printf("offset_size = %d\n", offset_size);
                break;
            case 'f':   // filename
                temp = strtok(argv[i], "=");
                filename = strtok(NULL, "\0");
                printf("filename = %s\n", filename);
                break;
        }
    }

    lines = cache_size / block_size; /* 1 line == 1 block */
    index_size = lines / set_size;
    index_bits = getBitSize(index_size);
    printf("index_bits = %d\n", index_bits);

    fp = fopen(filename, "r");
    if(fp == NULL) {
        fprintf(stderr, "file open error");
        exit(EXIT_FAILURE);
    }

    while(fscanf(fp, "%x %c", &mem_addr, &inst) != EOF) {
        //printf("\n%08X %c", mem_addr, inst);
        
        if(inst == 'W') {
            fscanf(fp, " %d", &data);
            //printf(" %d", data);
            printf("mem_addr = %08X, ", mem_addr);
            BYTE offset = getBits(mem_addr, offset_size);
            mem_addr = mem_addr >> offset_size;
            BYTE index = getBits(mem_addr, index_bits);
            printf("offset = %d, index = %d\n", offset, index);

            write(mem_addr, data, offset_size);

        } else if(inst == 'R') {

            read(mem_addr);
        }
    }


    /* print total */
    miss_rate = (double)((double)t_miss / (t_hit + t_miss)) * 100;
    //average_memory_access_cycle = (double)(memory_access)

    printf("\ntotal number of hits: %d\n", t_hit);
    printf("total number of misses: %d\n", t_miss);
    printf("miss rate: %.1f%%\n", miss_rate);
    printf("total number of dirty blocks: %d\n", t_dirty);
    //printf("average memory access cycle: %.1f\n", )
}

void write(WORD mem_addr, int data, BYTE offset_size) {
    

    return;
}

void read(WORD mem_addr) {
    return;
}

int getBitSize(BYTE num) {
    int count = 0;
    while(num != 1) {
        count += 1;
        num = num >> 1;
    }
    return count;
}

int power(int num, int n) {
    if(n <= 0) return 1;
    else return(num * power(num, n - 1));
}

int getBits(WORD mem_addr, BYTE size) {
    int i;
    int res = 0;
    for(i = 0; i < size; i++) {
        res += (mem_addr % 2) * power(2, i);
        mem_addr /= 2;
    }
    return res;
}