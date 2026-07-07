#include <stdlib.h>
#include <stdio.h>

// --- Config ---

const int LINES = 1;
const int BLOCK_SIZE = 4;
const int SETS = 8;

const int ADDRESS_BITS = 12;
const int INDEX_BITS = 3;
const int OFFSET_BITS = 1;
const int TAG = ADDRESS_BITS - OFFSET_BITS - SETS;

// This is associated with the memory address block for a requested block of 4.
// It'll be place in 4 block alignments (i.e -> Read 0x125 -> Line = [0x124, 0x125, 0x126, 0x127] ) 
typedef int Line;

// This is the abstraction of Cache Sets that allows us to implement different way set associations 
typedef struct {
    Line line;
} CacheSet;


CacheSet* cache_init(int sets){ 
    CacheSet *cache = (CacheSet*) malloc(sets * sizeof(CacheSet));
    return cache;
}

int get_index(int addr){ 
    return (addr >> 2) & (SETS - 1);
}

int get_offset(int addr){ 
    return addr & 1;
}

int get_tag(int addr){ 
    return addr >> (INDEX_BITS + OFFSET_BITS);
}

void binprintf(int v)
{
    unsigned int mask=1<<((sizeof(int)<<3)-1);
    while(mask) {
        printf("%d", (v&mask ? 1 : 0));
        mask >>= 1;
    }
}

int main(int argc, char** argv){
    
    if(argc != 3){ 
        printf("Invalid number of arguments!\n");
        return EXIT_FAILURE;

    }
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL){
        printf("Error opening the file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    CacheSet *cache = cache_init(SETS);
    
    int curr;
    while (fscanf(fp, "%x", &curr) != EOF){ 
        // Gets the addr aligned on 4, so READ 0x125 -> Line = [0x124, 0x125, 0x126, 0x127] 
        // This may be more optimal given a real cache, will test. 
        curr = curr - (curr % 4); 
        int idx = get_index(curr);
        int tag = get_tag(curr);
        int ofs = get_offset(curr);
        
        printf("{0x%x, 0b", curr);
        binprintf(curr);
        printf(" idx=%d, tag=%d, ofs=%d}\n", idx, tag, ofs);

        cache[idx].line = curr;
    }   
    fclose(fp);
    fp = NULL;

    fp = fopen(argv[2], "w");
    if (fp == NULL){
        printf("Error opening the file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    for(int i = 0; i < SETS; i++){ 
        int l = cache[i].line;
        for(int j = 0; j < BLOCK_SIZE; j++){ 
            fprintf(fp, "0x%x\n", l + j );
        }
    }



    fclose(fp);
    free(cache);
    return EXIT_SUCCESS;
}

