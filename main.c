#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

typedef unsigned char byte;

typedef struct Block {
	bool is_dirty;
	bool is_valid:
	time_t last_used_at;
	int tag;
	byte bytes[64];
} Block;

typedef struct Set {
	Block blocks[4];
} Set;

typedef struct Cache {
	int number_of_memory_accesses;
	int number_of_misses;
	Set sets[16];
} Cache;

int get_offset(int address) {
	int bit_mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
	return (byte)(address & bit_mask);
}

int get_index(int address) {
	address = address >> 6;
	int bit_mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	return (byte)(address & bit_mask);
}

int get_tag(int address) {
	address = address >> 10;
	int bit_mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
	return (byte)(address & bit_mask);
}

Cache cache;

void init() {
	cache.number_of_misses = 0;
	cache.number_of_memory_accesses = 0;
	int i;
	for (i = 0; i < 16; i++) {
   		Set set;
   		int j;
   		for(j = 0; j < 4; j++) {
   			Block block;
   			block.is_dirty = false;
   			block.is_valid = false;
   			block.last_used_at = 0;
   			block.tag = 0;
   			int k;
   			for(k = 0; k < 64; k++) {
   				block.bytes[k] = 0;
   			}
   			set.blocks[j] = block;
   		}
   		cache.sets[i] = set;
	}
}

int main (int argc, char *argv[]) {
	init();
	return EXIT_SUCCESS;
}