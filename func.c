#include "defs.h"




void* my_malloc(size_t size){


       if (heap_start == NULL) {
           // incrementing the heap :
           void* Prev_Program_Brake = sbrk((size + HEADER_SIZE)); // !sbrk returns OLD address where heap starts

           if(Prev_Program_Brake == (void*)-1)
           {
               printf("Error incrementing the HEAP.\n");
               return NULL;
           }

           // adding structure to heap space :
           Mem_Block* New_Chunk = (Mem_Block*)Prev_Program_Brake;
           New_Chunk->Size_Alloc = AlignBytes(size);
           New_Chunk->isFree = 0;
           New_Chunk->pNext = NULL;

           heap_start = New_Chunk;

           return heap_start + 1;
       }

    Mem_Block* current = heap_start;


    while( current != NULL) {

        if( current->isFree == 1 && current->Size_Alloc >= size ) {

            Block_Splitting(size, current);

            current->isFree = 0;

            return current + 1;
        }
        current = current->pNext;
    }

    void* AddToHeap = sbrk(size + HEADER_SIZE);
    current = (Mem_Block*)AddToHeap;
    current->Size_Alloc = AlignBytes(size);
    current->isFree = 0;
    current->pNext = NULL;

    return current + 1;
}


void my_free(void* ptr) {

    if( ptr == NULL ) {
        return;
    }

    Mem_Block* ChunkToFree = (Mem_Block*)ptr - 1;

    ChunkToFree->isFree = 1;


    Mem_Block* Right_Neighbour = ChunkToFree->pNext; // prawy sąsiad zwalnianego bloku

    // sprawdzenie prawego sąsiada :

    if( Right_Neighbour->isFree == 1) {

        ChunkToFree->Size_Alloc = ChunkToFree->Size_Alloc + Right_Neighbour->Size_Alloc + HEADER_SIZE;
        ChunkToFree->pNext = Right_Neighbour->pNext;
        return;
    }

    // sprawdzenie lewego sąsiada :

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

size_t AlignBytes( size_t size ) {

  /*  if ( size % MIN_MULTIPLE != 0 ) {

        size = size + MIN_MULTIPLE;

        size = size - (size % MIN_MULTIPLE);

        return size;

    }
    return size;
    */

    return ~(MIN_MULTIPLE-1) & size + MIN_MULTIPLE-1;
}


void Block_Splitting(size_t size, Mem_Block* BlockFound) {
    // check how many bytes of block is really usable and how much bytes my_malloc can give out

    size_t AvailableBytes = BlockFound->Size_Alloc - (size + HEADER_SIZE);

    // odlaczony blok zaczyna sie pod adresem : Adres bloku pamieci, ktory zostal poczatkowo znaleziony + naglowek + w bajtach ile uztkownik chcial pamieci
    // !! rzutowanie na typ jednobajtowy char* aby łatwo przesunac sie w pamieci o dowolna ilosc bajtow


    // Adres bloku ktory jest jeszcze do wykorzystania
    Mem_Block* SplitBlock = (Mem_Block*)((char*)BlockFound + HEADER_SIZE + size);

    SplitBlock->pNext = BlockFound->pNext;

    BlockFound->pNext = SplitBlock;

    BlockFound->Size_Alloc = size;

    SplitBlock->Size_Alloc = AvailableBytes;

    SplitBlock->isFree = 1;
}