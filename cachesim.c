#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef unsigned char BYTE;  // 8-bit byte
typedef unsigned int WORD;   // 32-bit word 

#define MALLOC(p, s) if(!((p) = (nodePointer)malloc(s))) { \
	fprintf(stderr, "Insufficient Memory"); \
	exit(EXIT_FAILURE); \
	}

typedef struct _node* nodePointer;


/* cache structure */
typedef struct cache {
    WORD tag;
    BYTE valid;
    BYTE dirty;
    BYTE offset;
    int data;
    int time;
} Cache;

/* linked list node for memory */
typedef struct _node {
    Cache cache;
    struct _node *next;
} node;

int t_hit, t_miss, t_cycle;

void write(WORD mem_addr, int data, BYTE offset_size);
void read(WORD mem_addr);
int getOffsetBits(BYTE num);
int power(int num, int n);

int main(int argc, char* argv[]) {
    int i;
    char* temp;

    int lines, index;
    BYTE cache_size, set_size, block_size, offset_size;
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
                offset_size = getOffsetBits(block_size);
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
    index = lines / set_size;

    fp = fopen(filename, "r");
    if(fp == NULL) {
        fprintf(stderr, "file open error");
        exit(EXIT_FAILURE);
    }

    while(fscanf(fp, "%x %c", &mem_addr, &inst) != EOF) {
        printf("\n%08X %c", mem_addr, inst);
        if(inst == 'W') {
            fscanf(fp, " %d", &data);
            printf(" %d", data);
            write(mem_addr, data);
        } else if(inst == 'R') {
            read(mem_addr);
        }
    }

}

void write(WORD mem_addr, int data, BYTE offset_size) {
    

    return;
}

void read(WORD mem_addr) {
    return;
}

int getOffsetBits(BYTE num) {
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