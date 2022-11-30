// 2019114545 장우석

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

#define MAX(a, b) (((a)>(b))?(a):(b))
#define MIN(a, b) (((a)<(b))?(a):(b))


///////////////////////////////////////////////////////////////////////////

/* cache structure */
typedef struct cache_line {
    WORD tag;
    bool valid;
    bool dirty;
    BYTE offset;
    BYTE data[MAX_WORD_IN_ONE_LINE];
    int time;
} Cache_line;

/////////////////////////////////////////////////////////////////////////////////

/* linked list struct */
typedef struct _Memory {
    WORD addr;
    BYTE data[MAX_WORD_IN_ONE_LINE];
    struct _Memory* llink;
    struct _Memory* rlink;
} MEMORY;
MEMORY* memory_list;

void insert_node(MEMORY* destnode, MEMORY* newnode) {
    newnode->llink = destnode;
    newnode->rlink = destnode->rlink;
    destnode->rlink->llink = newnode;
    destnode->rlink = newnode;
}

MEMORY* find_node(MEMORY* head, WORD addr) {
    MEMORY* cur;
    for(cur = head->rlink; cur != head; cur = cur->rlink) {
        if(cur->addr == addr) {
            return cur;
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////

/* global variables */
int t_hit, t_miss, t_cycle;
Cache_line* cache_lines[MAX_LINE];


/* functions */
void write(WORD mem_addr, int data, BYTE set_size, BYTE offset_size, BYTE index_bits);
void read(WORD mem_addr, BYTE set_size, BYTE offset_size, BYTE index_bits);
int getBitSize(BYTE num);
int power(int num, int n);
int getBits(WORD mem_addr, BYTE size);
void printCache(int lines, int index_size, BYTE block_size, BYTE set_size);
WORD getAddressFromCache(Cache_line** cache, BYTE index, BYTE index_bits, BYTE offset_size);

/////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
    int i, j;
    double miss_rate, average_memory_access_cycle;
    int dirty_blocks = 0;
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
                // printf("cache_size = %d\n", cache_size);
                break;
            case 'a':   // set_size
                temp = strtok(argv[i], "=");
                temp = strtok(NULL, "\0");
                set_size = atoi(temp);
                // printf("set_size = %d\n", set_size);
                break;
            case 'b':   // block_size
                temp = strtok(argv[i], "=");
                temp = strtok(NULL, "\0");
                block_size = atoi(temp);
                offset_size = getBitSize(block_size);
                // printf("block_size = %d\n", block_size);
                // printf("offset_size = %d\n", offset_size);
                break;
            case 'f':   // filename
                temp = strtok(argv[i], "=");
                filename = strtok(NULL, "\0");
                // printf("filename = %s\n", filename);
                break;
        }
    }

    lines = cache_size / block_size; /* 1 line == 1 block */
    index_size = lines / set_size;
    index_bits = getBitSize(index_size);
    // printf("index_bits = %d\n", index_bits);

    /* init cache */
    for(i = 0; i < lines; i++) {
        Cache_line* temp;
        temp = (Cache_line*)malloc(sizeof(*temp));
        temp->valid = 0;
        temp->dirty = 0;
        temp->time = 0;
        for(j = 0; j < block_size; j++) {
            temp->data[j] = 0;
        }
        cache_lines[i] = temp;
    }

    /* init memory */
    memory_list = (MEMORY*)malloc(sizeof(*memory_list));
    memory_list->llink = memory_list;
    memory_list->rlink = memory_list;


    /* file open */
    fp = fopen(filename, "r");
    if(fp == NULL) {
        fprintf(stderr, "file open error");
        exit(EXIT_FAILURE);
    }

    while(fscanf(fp, "%x %c", &mem_addr, &inst) != EOF) {
        // printf("\n%08X %c", mem_addr, inst);
        
        if(inst == 'W') {
            fscanf(fp, " %d", &data);
            // printf(" %d", data);
            // printf("mem_addr = %08X, ", mem_addr);
            write(mem_addr, data, set_size, offset_size, index_bits);

        } else if(inst == 'R') {
            // printf("\n\ndebug read: %08X\n", mem_addr);
            read(mem_addr, set_size, offset_size, index_bits);
        }
    }

    /* print cache blocks */
    printCache(lines, index_size, block_size, set_size);

    for(i = 0; i < lines; i++) {
        if(cache_lines[i]->dirty) dirty_blocks++;
    }

    /* print total */
    miss_rate = (double)((double)t_miss / (t_hit + t_miss));
    average_memory_access_cycle = (double)(t_hit + miss_rate * MISS_PENALTY);

    printf("\ntotal number of hits: %d\n", t_hit);
    printf("total number of misses: %d\n", t_miss);
    printf("miss rate: %.1f%%\n", miss_rate * 100);
    printf("total number of dirty blocks: %d\n", dirty_blocks);
    printf("average memory access cycle: %.1f\n", average_memory_access_cycle);
}




void write(WORD mem_addr, int data, BYTE set_size, BYTE offset_size, BYTE index_bits) {
    int i, j;
    int max_time = -1, idx;
    MEMORY *cache_to_mem;
    WORD addr = mem_addr;
    WORD temp_addr, temp_data;

    // get cache line from address //
    BYTE offset = getBits(addr, offset_size);
    addr = addr >> offset_size;
    BYTE index = getBits(addr, index_bits);
    addr = addr >> index_bits;
    WORD tag = getBits(addr, 32 - offset_size - index_bits);
    // printf("debug in write tag = %d\n", tag);

    for(i = 0; i < set_size; i++) { // hit
        /* cache_lines[index * set_size + i] */ // index's i-way
        if(cache_lines[index * set_size + i]->tag == tag && cache_lines[index * set_size + i]->valid == 1) {
            // printf("debug hit\n");
            cache_lines[index * set_size + i]->dirty = 1;
            temp_data = data;
            for(j = offset; j < offset + 4; j++) {
                cache_lines[index * set_size + i]->data[j] = temp_data & 255;
                temp_data = temp_data >> 8;
            }
            t_hit += 1;
            t_cycle += 1;
            return;
        }
    }


    // tag 일치 cache 없음(miss)
    // printf("debug miss\n");
    idx = index * set_size;
    for(i = 0; i < set_size; i++) {
        // 각 way의 cache에 저장된 시간을 비교 후 더 오래된 캐시 교체(메모리 접근 -> miss)
        // cache에 저장된 시간이 0일 경우 캐시가 빈 상태 -> 메모리 접근 필요
        if(cache_lines[index * set_size + i]->time == 0) {
            idx = index * set_size + i;
            break;
        } else if(max_time < cache_lines[index * set_size + i]->time) {
            max_time = cache_lines[index * set_size + i]->time;
            idx = index * set_size + i;
        }
    }
    
    t_miss += 1;
    t_cycle += MISS_PENALTY;
    if(cache_lines[idx]->dirty == 1) {  // main memory에 evict 필요(dirty bit == 1)
        /* 교체될 cache에서 메모리 주소 가져오기 */
        temp_addr = getAddressFromCache(&cache_lines[idx], index, index_bits, offset_size);

        if((cache_to_mem = find_node(memory_list, temp_addr)) == NULL) {    // main memory에 해당 주소값 없음 -> 추가
            MEMORY* newnode;
            newnode = (MEMORY*)malloc(sizeof(*newnode));
            newnode->addr = temp_addr;
            memcpy(newnode->data, cache_lines[idx]->data, sizeof(newnode->data));
            insert_node(memory_list, newnode);
        } else {    // main memory에 해당 주소 있음 -> 해당 memory의 data 교체
            memcpy(cache_to_mem->data, cache_lines[idx]->data, sizeof(cache_to_mem->data));
            insert_node(memory_list, cache_to_mem);
        }
    }
    

    cache_lines[idx]->valid = 1;
    cache_lines[idx]->dirty = 0;
    cache_lines[idx]->tag = tag;
    cache_lines[idx]->time = t_cycle;
    cache_lines[idx]->offset = offset;
    // read와 다른점: memory에서 data를 가져올 필요 없이 cache에만 올리고 바로 write
    // 이후 evict과정에서 main memory에 data write
    temp_data = data;
    for(j = offset; j < offset + 4; j++) {
        cache_lines[idx]->data[j] = temp_data & 255;
        temp_data = temp_data >> 8;
    }

    t_cycle += 1;

    return;
}

void read(WORD mem_addr, BYTE set_size, BYTE offset_size, BYTE index_bits) {
    int i;
    int max_time = -1, idx;
    MEMORY *mem_to_cache, *cache_to_mem;
    WORD addr = mem_addr;
    WORD temp_addr;

    // get cache line from address //
    BYTE offset = getBits(addr, offset_size);
    addr = addr >> offset_size;
    BYTE index = getBits(addr, index_bits);
    addr = addr >> index_bits;
    WORD tag = getBits(addr, 32 - offset_size - index_bits);
    // printf("debug in read tag = %d\n", tag);

    for(i = 0; i < set_size; i++) { // hit
        /* cache_lines[index * set_size + i] */ // index's i-way
        if(cache_lines[index * set_size + i]->tag == tag && cache_lines[index * set_size + i]->valid == 1) {
            // printf("debug hit\n");
            t_hit += 1;
            t_cycle += 1;
            return;
        }
    }


    // tag 일치 cache 없음(miss)
    // printf("debug miss\n");
    idx = index * set_size;
    for(i = 0; i < set_size; i++) {
        // 각 way의 cache에 저장된 시간을 비교 후 더 오래된 캐시 교체(메모리 접근 -> miss)
        // cache에 저장된 시간이 0일 경우 캐시가 빈 상태 -> 메모리 접근 필요
        if(cache_lines[index * set_size + i]->time == 0) {
            idx = index * set_size + i;
            break;
        } else if(max_time < cache_lines[index * set_size + i]->time) {
            max_time = cache_lines[index * set_size + i]->time;
            idx = index * set_size + i;
        }
    }
    
    t_miss += 1;
    t_cycle += MISS_PENALTY;
    if(cache_lines[idx]->dirty == 1) {  // main memory에 evict 필요(dirty bit == 1)
        /* 교체될 cache에서 메모리 주소 가져오기 */
        temp_addr = getAddressFromCache(&cache_lines[idx], index, index_bits, offset_size);

        if((cache_to_mem = find_node(memory_list, temp_addr)) == NULL) {    // main memory에 해당 주소값 없음 -> 추가
            MEMORY* newnode;
            newnode = (MEMORY*)malloc(sizeof(*newnode));
            newnode->addr = temp_addr;
            memcpy(newnode->data, cache_lines[idx]->data, sizeof(newnode->data));
            insert_node(memory_list, newnode);
        } else {    // main memory에 해당 주소 있음 -> 해당 memory의 data 교체
            memcpy(cache_to_mem->data, cache_lines[idx]->data, sizeof(cache_to_mem->data));
            insert_node(memory_list, cache_to_mem);
        }
    }
    

    cache_lines[idx]->valid = 1;
    cache_lines[idx]->dirty = 0;
    cache_lines[idx]->tag = tag;
    cache_lines[idx]->time = t_cycle;
    cache_lines[idx]->offset = offset;
    if((mem_to_cache = find_node(memory_list, addr)) != NULL) { // main memory에 해당 주소값 있음 -> cache에 해당 memory의 data 가져오기
        memcpy(cache_lines[idx]->data, mem_to_cache->data, sizeof(cache_lines[idx]->data));
    }

    t_cycle += 1;

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

void printCache(int lines, int index_size, BYTE block_size, BYTE set_size) {
    int i, j;
    int k = 0;
    for(i = 0; i < lines; i++) {
        if(i % set_size == 0) {
            printf("%d: ", i / set_size);
        } else {
            printf("   ");
        }
        for(j = block_size - 1; j >= 0; j--) { // print little endian
            printf("%02X", cache_lines[i]->data[j % 4 + (4 * k)]);
            if(j % 4 == 0) {
                printf(" ");
                k += 1;
            }
        }
        printf("v:%d d:%d\n", cache_lines[i]->valid, cache_lines[i]->dirty);
        k = 0;
    }
}

WORD getAddressFromCache(Cache_line** cache, BYTE index, BYTE index_bits, BYTE offset_size) {
    WORD addr;
    WORD temp_tag, temp_index, temp_offset;
    temp_tag = (*cache)->tag;
    temp_tag = temp_tag << (index_bits + offset_size);
    temp_index = (WORD)index << index_bits;
    temp_offset = (WORD)((*cache)->offset);
    addr = temp_tag + temp_index + temp_offset;
    return addr;
}