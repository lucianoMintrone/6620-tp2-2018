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
	float number_of_misses;
	Set sets[NUMBER_OF_SETS_IN_CACHE];
} Cache;

typedef struct Memory {
	Block blocks[NUMBER_OF_BLOCKS_IN_MP];
} Memory;

Cache cache;
Memory memory;

long get_microtime() {
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

void init_principal_memory() {
	int i;
	for (i = 0; i < NUMBER_OF_BLOCKS_IN_MP; i++) {
		Block block;
		block.is_dirty = false;
		block.is_valid = true;
		block.last_used_at = 0;
		block.tag = -1;
		int k;
		for(k = 0; k < NUMBER_OF_BYTES_IN_BLOCK; k++) {
			block.bytes[k] = 0;
		}
		memory.blocks[i] = block;
	}
}

int get_way_of_block(Block block, int set_number) {
	int i;
	Set set = cache.sets[set_number];
	for(i = 0; i < NUMBER_OF_BLOCKS_IN_SET; i++) {
		if(set.blocks[i].tag == block.tag) {
			return i;
		}
	}
	return i;
}

int get_mp_address(int tag, int index) {
	int mp_address = tag;
	mp_address = mp_address << 4;
	return mp_address + index;
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
			block.tag = -1;
			int k;
			for(k = 0; k < NUMBER_OF_BYTES_IN_BLOCK; k++) {
				block.bytes[k] = 0;
			}
			set.blocks[j] = block;
		}
		cache.sets[i] = set;
	}
	init_principal_memory();
}

Set find_set(int address) {
	return cache.sets[get_index(address)];
}

Block find_lru_with_set(Set set) {
	Block lru_block = set.blocks[0];
	int i;
	for(i = 1; i < NUMBER_OF_BLOCKS_IN_SET; i++) {
		Block block = set.blocks[i];
		if (block.last_used_at < lru_block.last_used_at) {
			lru_block = block;
		}
	}
	return lru_block;

}

Block find_lru(int setnum) {
	Set set = cache.sets[setnum];
	return find_lru_with_set(set);
}

int is_dirty(int way, int blocknum) {
	return cache.sets[blocknum].blocks[way].is_dirty;
}

void write_block(int way, int set_number) {
	int mp_address = get_mp_address(cache.sets[set_number].blocks[way].tag, set_number);
	cache.sets[set_number].blocks[way].is_dirty = false;
	cache.sets[set_number].blocks[way].is_valid = true;
	cache.sets[set_number].blocks[way].last_used_at = 0;
	memory.blocks[mp_address].tag = cache.sets[set_number].blocks[way].tag;
	memory.blocks[mp_address].is_dirty = false;
	memory.blocks[mp_address].is_valid = true;
	memory.blocks[mp_address].last_used_at = 0;
	int byte;
	for(byte = 0; byte < NUMBER_OF_BYTES_IN_BLOCK; byte++) {
		memory.blocks[mp_address].bytes[byte] = cache.sets[set_number].blocks[way].bytes[byte];
	}
}

void read_block(int blocknum) {
	int set_number = blocknum % NUMBER_OF_SETS_IN_CACHE;
	Block cache_block = find_lru(set_number);
	if (cache_block.is_dirty) {
		write_block(get_way_of_block(cache_block, set_number), set_number);
	}
	int way;
	for(way = 0; way < NUMBER_OF_BLOCKS_IN_SET; way++) {
		if(cache.sets[set_number].blocks[way].tag == cache_block.tag) {
			cache.sets[set_number].blocks[way].last_used_at = get_microtime();
			cache.sets[set_number].blocks[way].is_valid = true;
			cache.sets[set_number].blocks[way].is_dirty = false;
			int tag = blocknum >> 4;
			cache.sets[set_number].blocks[way].tag = tag; 
			int byte;
			for(byte = 0; byte < NUMBER_OF_BYTES_IN_BLOCK; byte++) {
				cache.sets[set_number].blocks[way].bytes[byte] = memory.blocks[blocknum].bytes[byte];
			}
			return;
		}
	}
}

int read_byte(int address) {
	cache.number_of_memory_accesses ++;

	// Set set = find_set(address);
	int addres_index = get_index(address);
	int address_tag = get_tag(address);
	int address_offset = get_offset(address);

	// Iterate in all the ways of the set
	size_t way;
	for (way = 0; way < NUMBER_OF_BLOCKS_IN_SET; way++) {
		// Block block = find_set(address).blocks[way];

		// If the block was found and is valid
		if (address_tag == find_set(address).blocks[way].tag && find_set(address).blocks[way].is_valid) {
			return find_set(address).blocks[way].bytes[address_offset];
		}
	}

	// Miss ++
	cache.number_of_misses ++;
	// fetch block
	read_block(get_mp_address(address_tag, addres_index));

	// Iterate in all the ways of the set
	for (way = 0; way < NUMBER_OF_BLOCKS_IN_SET; way++) {
		// Block block = set.blocks[way];

		// If the block was found and is valid
		if (address_tag == find_set(address).blocks[way].tag) {
			return find_set(address).blocks[way].bytes[address_offset];
		}
	}
	// cannot return 300 is out of bounds of a char ...
	return 300;
}

int read_from_cache(char* address, char* value) {
	return read_byte(atoi(address));
}


void write_byte(int address, int value) {
	cache.number_of_memory_accesses ++;

	// Set set = find_set(address);
	int addres_index = get_index(address);
	int address_tag = get_tag(address);
	int address_offset = get_offset(address);

	// Iterate in all the ways of the set
	int way;
	for (way = 0; way < NUMBER_OF_BLOCKS_IN_SET; way++) {
		// Block block = find_set(address).blocks[way];

		// If the block was found and is valid
		if (address_tag == find_set(address).blocks[way].tag && find_set(address).blocks[way].is_valid) {
			find_set(address).blocks[way].bytes[address_offset] = value;
			find_set(address).blocks[way].is_dirty = true;
			find_set(address).blocks[way].is_valid = true;
			find_set(address).blocks[way].last_used_at = get_microtime();
			return;
		}
	}

	// Miss ++
	cache.number_of_misses ++;

	// fetch block
	read_block(get_mp_address(address_tag, addres_index));
	// override block
	for (way = 0; way < NUMBER_OF_BLOCKS_IN_SET; way++) {
		// Block block = set.blocks[way];

		if (address_tag == find_set(address).blocks[way].tag) {
			cache.sets[addres_index].blocks[way].bytes[address_offset] = value;
			cache.sets[addres_index].blocks[way].is_dirty = true;
			cache.sets[addres_index].blocks[way].last_used_at = get_microtime();
			return;
		}
	}
}

int write_in_cache(char* address, char* value) {
	write_byte(atoi(address), atoi(value));
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
			i ++;
			ptr = strtok(NULL, delim);
		}
		cache_data(operation, address, value);
	}
	fclose(input_file);
}

int main(int argc, char *argv[]) {
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
