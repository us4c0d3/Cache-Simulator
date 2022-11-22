#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef char BYTE;  // 8-bit byte
typedef int WORD;   // 32-bit word 

struct cache {
    int tag;
    int valid;
    int dirty = 0;
    int data;
};

int t_hit, t_miss, t_cycle;


int main(int argc, char* argv[]) {

}