#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ADDRESS 4096
#define ADDRESSES 1024
#define BLOCK_SIZE 4

// Hash Table to track access frequency
typedef struct {
  int tag;
  int freq;
} Entry;

int main(int argc, char **argv) {
  if (argc != 3) {
    return EXIT_FAILURE;
  }

  FILE *in = fopen(argv[1], "r");
  if (in == NULL) {
    return EXIT_FAILURE;
  }

  Entry table[MAX_ADDRESS] = {0};
  int idx = 0;
  int curr;
  while (fscanf(in, "%x", &curr) == 1) {
    // Place within hashtable to track freq
      table[curr].freq += 1;
      table[curr].tag = curr;
      idx++;
  }

  fclose(in);
  in = NULL;

  for(int i = 0; i < idx; i++){ 
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

  // Randomly print out the most frequent values. 
  for (int i = 0; i < idx; i++) {
    int v = table[rand() % 16].tag;
    fprintf(out, "0x%x\n",v);
  }

  fclose(out);
  return EXIT_SUCCESS;
}