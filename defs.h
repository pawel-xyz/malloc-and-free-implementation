#ifndef MY_MALLOC_DEFS_H
#define MY_MALLOC_DEFS_H

#include <stdio.h>
#include <unistd.h>

#define HEADER_SIZE sizeof(Mem_Block)
#define MIN_MULTIPLE 8

typedef struct tagMem_Block{

    size_t Size_Alloc;
    int isFree;
    struct tagMem_Block* pNext;

}Mem_Block;

extern Mem_Block* heap_start;
void* my_malloc(size_t size);
void my_free(void* ptr);
void CheckRightNeighbour(void* ptr);
void CheckLeftNeighbour(void* ptr);
size_t AlignBytes(size_t size );
void Block_Splitting(size_t, Mem_Block* BlockFound);


















#endif