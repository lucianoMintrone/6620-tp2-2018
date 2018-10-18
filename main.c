#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

typedef struct Block {
	bool is_dirty;
	time_t last_used_at;
	int tag;
	char bytes[6];
} Block;

typedef struct Set {
	Block blocks[4];
} Set;

typedef struct Cache {
	int number_of_memory_accesses;
	int number_of_misses;
	Set sets[1024];
} Cache;

int get_offset(int address) {
	int bit_mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	return (byte)(address & bit_mask);
}

int get_index(int address) {
	int bit_mask = (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
	return (byte)(address & bit_mask);
}

int get_tag(int address) {
	int bit_mask = (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15);
	return (byte)(address & bit_mask);
}

int main (int argc, char *argv[]) {
	return EXIT_SUCCESS;
}