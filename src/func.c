#include "defs.h"
Mem_Block* heap_start = NULL;


void* my_malloc(size_t size){

       if (heap_start == NULL) {
           // Incrementing the heap :
           void* Prev_Program_Brake = sbrk((size + HEADER_SIZE));

           if(Prev_Program_Brake == (void*)-1)
           {
               printf("Error incrementing the HEAP.\n");
               return NULL;
           }

           // Adding structure to the heap space :
           Mem_Block* New_Chunk = (Mem_Block*)Prev_Program_Brake;
           New_Chunk->Size_Alloc = AlignBytes(size);
           New_Chunk->isFree = 0;
           New_Chunk->pNext = NULL;

           heap_start = New_Chunk;

           return heap_start + 1;
       }

    Mem_Block* current = heap_start;
    Mem_Block* findlast = NULL;

    while( current != NULL) {

        if( current->isFree == 1 && current->Size_Alloc >= size ) {

            Block_Splitting(size, current);

            current->isFree = 0;

            return current + 1;
        }
        findlast = current;
        current = current->pNext;
    }

    void* AddToHeap = sbrk(size + HEADER_SIZE);
    current = (Mem_Block*)AddToHeap;
    current->Size_Alloc = AlignBytes(size);
    current->isFree = 0;
    current->pNext = NULL;
    findlast->pNext = current;

    return current + 1;
}


void my_free(void* ptr) {

    if( ptr == NULL ) {
        return;
    }

   CheckRightNeighbour(ptr);
   CheckLeftNeighbour(ptr);

    Mem_Block* ChunkToFree = (Mem_Block*)ptr - 1;
    ChunkToFree->isFree = 1;
}

size_t AlignBytes( size_t size ) {

    // The `modulo` way :

  /*  if ( size % MIN_MULTIPLE != 0 ) {

        size = size + MIN_MULTIPLE;

        size = size - (size % MIN_MULTIPLE);

        return size;

    }
    return size;
    */

    // The 'bit-masking' way :
    return ~(MIN_MULTIPLE-1) & size + MIN_MULTIPLE-1;
}


void Block_Splitting(size_t size, Mem_Block* BlockFound) {

    // We need to check how many bytes of that block is really usable and how much bytes my_malloc() can give out to the users.
    size_t AvailableBytes = BlockFound->Size_Alloc - (size + HEADER_SIZE);

    // The 'Available for user' block is stored in memory at address :
    Mem_Block* SplitBlock = (Mem_Block*)((char*)BlockFound + HEADER_SIZE + size);
    SplitBlock->pNext = BlockFound->pNext;
    BlockFound->pNext = SplitBlock;
    BlockFound->Size_Alloc = size;
    SplitBlock->Size_Alloc = AvailableBytes;
    SplitBlock->isFree = 1;
}



void CheckRightNeighbour(void* ptr) {

    Mem_Block* ChunkToFree = (Mem_Block*)ptr - 1;
    ChunkToFree->isFree = 1;

    // Checking if the right neighbour is available for use
    Mem_Block* Right_Neighbour = ChunkToFree->pNext;
    if( Right_Neighbour->isFree == 1) {

        ChunkToFree->Size_Alloc = ChunkToFree->Size_Alloc + Right_Neighbour->Size_Alloc + HEADER_SIZE;
        ChunkToFree->pNext = Right_Neighbour->pNext;
    }
}

void CheckLeftNeighbour(void* ptr) {

    Mem_Block* ChunkToFree = (Mem_Block*)ptr - 1;
    ChunkToFree->isFree = 1;

    // Checking if the left neighbour is available for use
    Mem_Block* Left_Neighbour = heap_start;

    while (Left_Neighbour != NULL) {
        if (Left_Neighbour->pNext == ChunkToFree && Left_Neighbour->isFree == 1 && ChunkToFree->isFree == 1) {
            Left_Neighbour->Size_Alloc = Left_Neighbour->Size_Alloc + ChunkToFree->Size_Alloc + HEADER_SIZE;
            Left_Neighbour->pNext = ChunkToFree->pNext;
            return;
        }
        Left_Neighbour = Left_Neighbour->pNext;
    }
}