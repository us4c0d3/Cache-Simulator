#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>


int main(int argc, char* argv[]) {
    int i, j;
    int cache_size, set_size, block_size;
    char* temp;
    char* filename;
    for (i = 1; i < argc; ++i){
        printf("%s\n", argv[i]);
    }

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
                printf("block_size = %d\n", block_size);
                break;
            case 'f':   // file_name
                temp = strtok(argv[i], "=");
                filename = strtok(NULL, "\0");
                printf("filename = %s\n", filename);
                break;
        }
    }
}