#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define NUMBER_OF_BLOCKS_IN_MP 65536
#define NUMBER_OF_BLOCKS_IN_SET 4
#define NUMBER_OF_SETS_IN_CACHE 16
#define NUMBER_OF_BYTES_IN_BLOCK 64

typedef unsigned char byte;

typedef struct Block {
	bool is_dirty;
	bool is_valid;
	long last_used_at; //The current time in microseconds.
	int tag;
	byte bytes[NUMBER_OF_BYTES_IN_BLOCK];
} Block;

typedef struct Set {
	Block blocks[NUMBER_OF_BLOCKS_IN_SET];
} Set;

typedef struct Cache {
	int number_of_memory_accesses;
	int number_of_misses;
	Set sets[NUMBER_OF_SETS_IN_CACHE];
} Cache;

typedef struct Memory {
	Block blocks[NUMBER_OF_BLOCKS_IN_MP];
} Memory;

Cache cache;
Memory memory;

long getMicrotime() {
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

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

void initPrincipalMemory() {
	int i;
	for (i = 0; i < NUMBER_OF_BLOCKS_IN_MP; i++) {
		Block block;
		block.is_dirty = false;
		block.is_valid = false;
		block.last_used_at = 0;
		block.tag = 0;
		int k;
		for(k = 0; k < NUMBER_OF_BYTES_IN_BLOCK; k++) {
			block.bytes[k] = 0;
		}
		memory.blocks[i] = block;
	}
}

void init() {
	cache.number_of_misses = 0;
	cache.number_of_memory_accesses = 0;
	int i;
	for (i = 0; i < NUMBER_OF_SETS_IN_CACHE; i++) {
		Set set;
		int j;
		for(j = 0; j < NUMBER_OF_BLOCKS_IN_SET; j++) {
			Block block;
			block.is_dirty = false;
			block.is_valid = false;
			block.last_used_at = 0;
			block.tag = 0;
			int k;
			for(k = 0; k < NUMBER_OF_BYTES_IN_BLOCK; k++) {
				block.bytes[k] = 0;
			}
			set.blocks[j] = block;
		}
		cache.sets[i] = set;
	}
	initPrincipalMemory();
}

Set find_set(int address) {
	return cache.sets[get_index(address) - 1];
}

Block find_lru(int setnum) {
	int i;
	Set set = cache.sets[setnum];
	Block lru_block = set.blocks[0];
	for(i = 1; i < NUMBER_OF_BLOCKS_IN_SET; i++) {
		Block block = set.blocks[i];
		if (block.last_used_at < lru_block.last_used_at) {
			lru_block = block;
		}
	}
	return lru_block;
}

int is_dirty(int way, int blocknum) {
	return cache.sets[blocknum].blocks[way].is_dirty;
}

void print_result(char **result, int len, FILE *output_file) {
	size_t i;
	for (i = 0; i < len; i++) {
		fprintf(output_file, "%s\n", result[i]);
	}
}

int read_from_cache(char* address, char* value) {
	cache.number_of_memory_accesses += 1;
	return 123;
}

int write_in_cache(char* address, char* value) {
	cache.number_of_memory_accesses += 1;
	return atoi(value);
}

double calculate_miss_rate() {
	if (cache.number_of_memory_accesses != 0) {
		return cache.number_of_misses / cache.number_of_memory_accesses;	
	} else {
		return 0;
	}
}

void cache_data(char* operation, char* address, char* value) {
	if (strcmp(operation, "R") == 0) {
		printf("Value read: %d\n", read_from_cache(address, value));
	}
	if (strcmp(operation, "W") == 0) {
		address[strlen(address)-1] = 0;
		printf("Value writen: %d\n", write_in_cache(address, value));
	}
	if (strcmp(operation, "MR") == 0) {
		printf("Miss Rate: %f\n", calculate_miss_rate());
	}
}

void read_file_and_cache_data(FILE *input_file, FILE *output_file) {
	char line[256];
	while (fgets(line, sizeof(line), input_file)) {
		line[strlen(line)-1] = 0; //Remove \n
		char delim[] = " ";

		char *ptr = strtok(line, delim);
		char *operation = 0;
		char *address = 0;
		char *value = 0;
		int i = 0;

		while(ptr != NULL) {
			switch (i) {
				case 0:
					operation = ptr;
					break;
				case 1:
					address = ptr;
					break;
				case 2:
					value = ptr;
					break;
			}
			i += 1;
			ptr = strtok(NULL, delim);
		}
		cache_data(operation, address, value);
	}
	fclose(input_file);
}

int main (int argc, char *argv[]) {
	FILE *output_file, *input_file;
	output_file = input_file = NULL;

	int flag = 0;
	struct option opts[] = {
		{"output", required_argument, 0, 'o'},
		{"input", required_argument, 0, 'i'}
	};

	while ((flag = getopt_long(argc, argv, "Vhno:i:d", opts, NULL)) != -1) {
		switch (flag) {
			case 'o' :
				if (!strcmp(optarg, "-")) {
					output_file = stdout;
				} else {
					output_file = fopen(optarg, "w");
				}
				break;
			case 'i' :
				input_file = fopen(optarg, "r");
				break;
		}
	}

	init();
	read_file_and_cache_data(input_file, output_file);

	return EXIT_SUCCESS;
}