#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// --- Config ---
const int MEMORY_SIZE = 4096;            // Number of unique byte addresses log_2(4096) = 12;
const int CACHE_SIZE = 1024;             // Size Cache given in Bytes
const int CACHE_BLOCK_SIZE = 4;          // Bytes each block can store
const int CACHE_WAYS = 16;               // Lines per set
const int CACHE_SETS = 16;               // Number of sets

int CACHE_LINE_OFFSET_SIZE;              // Bits for offset
int CACHE_LINE_INDEX_SIZE;               // Bits for set index  
int CACHE_TAG_SIZE;                      // Bits for tag 


// This is associated with the memory address block for a requested block of 4.
// It'll be place in 4 block alignments (i.e -> Read 0x125 -> Line = [0x124, 0x125, 0x126, 0x127] ) 
typedef struct { 
    unsigned int last_tick;
    unsigned int freq;
    int tag; // This refers to entire cache line indentifier not just "tag"
    char valid;
}Line;

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

    CACHE_LINE_INDEX_SIZE = (int) log2(CACHE_SETS);
    CACHE_LINE_OFFSET_SIZE = (int) log2(CACHE_BLOCK_SIZE); 
    CACHE_TAG_SIZE = (int) log2(MEMORY_SIZE) - CACHE_LINE_INDEX_SIZE - CACHE_LINE_OFFSET_SIZE;

    CacheSet *cache = cache_init();
    
    int curr;
    unsigned int tick = 0;
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
        Line least_freq = set.lines[0];
        int least_freq_idx = 0;
        for(int i = 0; i < CACHE_WAYS; i++){ 
            Line line = set.lines[i];

            // Track frequency and break ties based on LRU
            if (least_freq.freq == line.freq) { 
                least_freq_idx = least_freq.last_tick >= line.last_tick ? least_freq_idx : i;
                least_freq = set.lines[least_freq_idx];
            } 

            if (least_freq.freq < line.freq){ 
                least_freq = line;
                least_freq_idx = i;
            }


            if (line.tag == curr){ 
                set.lines[i].last_tick = tick;
                set.lines[i].freq += 1;
                break;
            }

            if (!(line.valid)){ 
                set.lines[i].tag = curr;
                set.lines[i].last_tick = tick;
                set.lines[i].valid = 1;  
                set.lines[i].freq = 0;
                break;
            }
    
            if (i == (CACHE_WAYS - 1)){ 
                set.lines[least_freq_idx].tag = curr; 
                set.lines[least_freq_idx].last_tick = tick;
                line.valid = 1;   
            }
        
        }
        tick++;
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
            Line line = set.lines[j];
            char valid = line.valid;
            if (!valid) continue;
            

            for (int k = 0; k < CACHE_BLOCK_SIZE; k++){ 
                fprintf(fp, "0x%x\n", line.tag + k);
            }
        }

    }

    cache_free(cache);
    fclose(fp);
    return EXIT_SUCCESS;
}

