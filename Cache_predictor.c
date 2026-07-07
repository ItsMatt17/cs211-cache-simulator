#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Config ---

const int MEMORY_SIZE = 4096;            // Number of unique byte addresses log_2(4096) = 12;
const int CACHE_SIZE = 1024;             // Size Cache given in Bytes
const int CACHE_BLOCK_SIZE = 4;          // Bytes each block can store
const int CACHE_WAYS = 16;               // Lines per set
const int CACHE_SETS = 16;               // Number of sets

const int CACHE_LINE_OFFSET_SIZE = 2;    // Bits for offset
const int CACHE_LINE_INDEX_SIZE = 4;     // Bits for set index  
const int CACHE_TAG_SIZE = 6;            // Bits for tag 




// This is associated with the memory address block for a requested block of 4.
// It'll be place in 4 block alignments (i.e -> Read 0x125 -> Line = [0x124, 0x125, 0x126, 0x127] ) 
typedef int Line;

// This is the abstraction of Cache Sets that allows us to implement different way set associations 
typedef struct {
    Line *lines;
} CacheSet;


void cache_free(CacheSet *cache){ 
    for (int i = 0; i < CACHE_SETS; i++){ 
        free(cache[i].lines);
    }
    free(cache);
}

CacheSet* cache_init(){ 
    CacheSet *cache = (CacheSet*) malloc(CACHE_SETS * sizeof(CacheSet));

    for (int i = 0; i < CACHE_SETS; i++){ 
        cache[i].lines = (Line *) malloc(CACHE_WAYS * sizeof(Line));

        // Have all lines by default start at zero, so a validity bit can be included
        memset(cache->lines, 0, CACHE_WAYS * sizeof(Line));
    }
    return cache;
}

int get_index(int addr){    
    return (addr >> CACHE_LINE_OFFSET_SIZE) & (CACHE_SETS - 1);
}

int get_offset(int addr){ 
    return addr & (CACHE_LINE_OFFSET_SIZE - 1);
}

int get_tag(int addr){ 
    return addr >> (CACHE_LINE_OFFSET_SIZE + CACHE_LINE_INDEX_SIZE);
}

int valid_bit(int addr){ 
    return addr & (1 << 31);
}

int set_valid_bit (int addr) {
    return addr | (1 << 31);
}

// This gets the address w/o any valid bits set;
int get_addr(int addr){ 
    return addr & (MEMORY_SIZE - 1);
}

void binprintf(int v){
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

    CacheSet *cache = cache_init();
    
    int curr;
    while (fscanf(fp, "%x", &curr) != EOF){ 
        // Gets the addr aligned on 4, so READ 0x125 -> Line = [0x124, 0x125, 0x126, 0x127] 
        // This may be more optimal given a real cache, will test. 
        curr = curr - curr % CACHE_BLOCK_SIZE;
        int idx = get_index(curr);
        int tag = get_tag(curr);
        int ofs = get_offset(curr);
        
        printf("{0x%x, 0b", curr);
        binprintf(curr);
        printf(" idx=%d, tag=%d, ofs=%d}\n", idx, tag, ofs);
        
        CacheSet set = cache[idx];
        for(int i = 0; i < CACHE_WAYS; i++){ 
            int line = set.lines[i];
            if (get_addr(line) == curr) break;
            if (valid_bit(line)) continue; 
            
            set.lines[i] = set_valid_bit(curr);
            break;
        }

    }   
    fclose(fp);
    fp = NULL;

    fp = fopen(argv[2], "w");
    if (fp == NULL){
        printf("Error opening the file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }


    for(int i = 0; i < CACHE_SETS; i++){ 
        CacheSet set = cache[i];
        for(int j = 0; j < CACHE_WAYS; j++){ 
            int line = get_addr(set.lines[j]);
            int valid = valid_bit(set.lines[j]);
            if (!valid) continue;
            

            for (int k = 0; k < CACHE_BLOCK_SIZE; k++){ 
                fprintf(fp, "0x%x\n", line + k);
            }
        }

    }

    cache_free(cache);
    fclose(fp);
    return EXIT_SUCCESS;
}

