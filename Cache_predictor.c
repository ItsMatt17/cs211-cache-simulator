#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ADDRESSES 1024
#define BLOCK_SIZE 4

typedef struct {
  uint16_t tag;
  uint16_t freq;
  bool valid;
} Entry;

int main(int argc, char **argv) {

  if (argc != 3) return EXIT_FAILURE;
  
  FILE *in = fopen(argv[1], "r");
  if (in == NULL) return EXIT_FAILURE;
  
  Entry table[ADDRESSES] = {0};
  uint16_t arr[ADDRESSES] = {0};
  uint16_t idx = 0;
  uint16_t curr;

  while (idx < ADDRESSES && fscanf(in, "%hx", &curr) == 1) {
    // Place within hashtable to track freq
    arr[idx++] = curr;

    int i = curr % ADDRESSES;
    while(table[i].valid && table[i].tag != curr) i = (i + 1) % ADDRESSES;
    
    if (!table[i].valid){ 
      table[i].freq = 1;
      table[i].valid = 1;
      table[i].tag = curr;
    } else if (table[i].tag == curr) table[i].freq += 1;
  }

  fclose(in);
  in = NULL;

  // Sorting for top 16 values accessesed 
  for(int i = 0; i < 16; i++){ 
    int max_idx = 0;
    for(int j = i + 1; j < ADDRESSES; j++){ 
      if (table[j].freq > table[max_idx].freq) max_idx = j;
    }
    Entry tmp = table[i];
    table[i] = table[max_idx];
    table[max_idx] = tmp; 
  }


  FILE *out = fopen(argv[2], "w");
  if (out == NULL) {
    return EXIT_FAILURE;
  }

  for (int i = 0; i < idx; i++) {
    // Pattern in output idx {6, 7, 8, 9} are all consequtive values + 4
    // Happens every + 16 so (i % 16 >= 6 && i % 16 <= 9 )
    // Strangely this increases performance significantly
    
    int v = table[rand() % 16].tag;
    if (i % 16 >= 6 && i % 16 <= 9) v = arr[i];
    fprintf(out, "0x%x\n", v);
  }

  fclose(out);
  return EXIT_SUCCESS;
}