#ifndef MY_MALLOC_DEFS_H
#define MY_MALLOC_DEFS_H

#include <stdio.h>
#include <unistd.h>


#define INC_BYTE 1024 // one kilobyte
#define HEADER_SIZE sizeof(Mem_Block)
#define MIN_MULTIPLE 8
#define WORLD_LENGTH 2


typedef struct tagMem_Block{

    size_t Size_Alloc;
    int isFree;
    struct tagMem_Block* pNext;

}Mem_Block;

Mem_Block* heap_start = NULL;

void* my_malloc(size_t size);
void my_free(void* ptr);
size_t AlignBytes(size_t size ); // Robi tzw. Padding
void Block_Splitting(size_t, Mem_Block* BlockFound); // Funkcja używana po znalazieniu wolnego bloku. Dzieli blok na [size][rest_free_to_use]















#endif