#ifndef CACHE_H
#define CACHE_H

// debe inicializar los bloques de la cach ́e
// como inv ́ali- dos y la tasa de misses a 0.
void init();

// debe devolver el conjunto de cach ́e al que mapea la direcci ́on address
int find_set(int address);

// debe devolver el bloque menos re- cientemente usado dentro de un conjunto (o alguno de ellos si hay m ́as de uno), utilizando el campo correspondiente de los metadatos de los bloques del conjunto.
int find_lru(int setnum);

// debe devolver el es- tado del bit D del bloque correspondiente
int is_dirty(int way, int setnum);

// debe leer el bloque blocknum de memoria y guardarlo en el lugar que le corresponda en la memoria cach ́e.
void read_block(int blocknum);

// debe escribir los da- tos contenidos en el bloque setnum de la v ́ıa way
void write_block(int way, int setnum);

// debe retornar el valor correspondien- te a la posici ́on address
int read_byte(int address);

// debe escribir el valor value en la posici ́on correcta del bloque que corresponde a address.
int write_byte(int address, char value);

// debe devolver el porcentaje de misses desde que se inicializ ́o el cache
int get_miss_rate();

#endif
