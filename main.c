#include "defs.h"
#include <stdio.h>
#include <unistd.h>


int main() {

    printf("Address where the heap start: %p\n", &heap_start);

    // Making sure that Block_Splitting() works correctly

    // First we need to allocate some memory :
    int* var1 = my_malloc(10 * sizeof(int));
    printf("var1 Address: %p\n", var1);

    int* var2 = my_malloc(sizeof(int));
    printf("var2 Address: %p\n", var2);

    // Now we can use my_free() to free big chunk allocated in var1 :
    my_free(var1);

    // If Block_Splitting() works fine if we want to allocate small ammount of bytes my_malloc() will allocate it at the start of the old var1 address
    char* var3 = my_malloc(sizeof(char));
    printf("var3 Address: %p\n", var3);


    // Now the var4 should be pointing in memory space located between var1 and var2
    int* var4 = my_malloc(sizeof(int));
    printf("var4 Address: %p\n", var4);

    return 0;
}
