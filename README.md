# *Custom Memory Allocator* (`my_malloc()`)

A custom implementation of the standard C library memory allocation functions (`malloc` and `free`). This project serves as an educational dive into low-level systems programming, demonstrating how dynamic memory management works under the hood using a singly linked list architecture.

## *Table of Contents*

* [Understanding Computer Memory](#Understanding-computer-memory)
  * [What are `malloc` and `free`?](#what-are-malloc-and-free)
* [Memory Implementation as a Singly Linked List](#memory-implementation-as-a-singly-linked-list)
  * [Allocation (`my_malloc`)](#allocation-my_malloc)
  * [Deallocation (`my_free`)](#deallocation-my_free)
* [Key Helper Functions](#Key-helper-functions)
  * [`AlignBytes` – Data Alignment](#1-alignbytessize_t-size--data-alignment)
  * [`Block_Splitting` – Preventing Internal Fragmentation](#2-block_splittingsize_t-size-mem_block-blockfound--preventing-internal-fragmentation)
* [Free Block Search Algorithms](#free-block-search-algorithms)

# *Understanding Computer Memory*
To understand how `malloc` works, it is essential to first understand how a computer manages memory. We can think of a computer's Random Access Memory (RAM) as a massive array of storage bytes, each with its own unique memory address. When a program is executed, the operating system isolates a specific layout of virtual memory for it to use.
Every program operates using virtual memory—an operating system feature that allows hard disk space to be used as RAM. Thanks to virtual memory, processes are under the illusion that the entire memory space is available to them; the hardware translates these virtual addresses into actual physical RAM addresses on the fly.

The structure of the address space can be visualized as a vertical block: low addresses—starting at 0x00000000—are at the very bottom, while high addresses—0xFFFFFFFF—are at the very top.
The RAM is divided into couple different segments :
  * **Text Segment** : This is the section containing machine instructions that the processor executes directly—such as program functions and instructions. This section is read-only.
  * **Initialized Data Segment** : Global and static variables that have already been assigned a value are stored here.
  * **Uninitialized Data Segment** : This is the place for uninitialized global and static variables.
  * **Heap** : Used for dynamic memory allocation. This is an unorganized pool of memory utilized when the exact memory requirement is unknown at compile time (e.g., handling dynamic arrays or user inputs). The programmer is fully responsible for manually requesting and releasing heap memory.
  * **Stack** : Used for static memory allocation. It automatically manages local variables, function parameters, and control flow. While operations on the stack are very fast, its size is strictly fixed and small.

If we would want to visualise this concept it would look something like this : 
<img width="1024" height="509" alt="image" src="https://github.com/user-attachments/assets/e046aae7-a62c-43fa-bb67-4d5aabd981ad" />

## *What is malloc() and free() ?* 
malloc() and free() are basic C functions used by a programmer to both manually allocate memory and then deallocate it. Both of these functions are included in `<stdlib.h>` which is a standard library in C programming language.

### *The structure of malloc() and free()`* : 
* `void* malloc( size_t size );`
  
-> `void*` means that `malloc()` returns pointer to void. That makes perfect sense because the only job of `malloc()` function is to retrieve a specified number of bytes from the operating system and reserve them in memory. This function does not know WHAT type of data is stored at that address. Let's assume that `malloc()` doesn't return void*. The people who created C would have to create special `malloc()` functions for every data type (e.g. `int* malloc_int(size_t size)`).
-> `size_t` is a special data type (size_t, "size" it's just a variable name) used to represent object size in bytes. A memory size can never be negative. For this reason, `size_t` is an unsigned integer type (it does not accept values ​​less than zero). This allows the available range of bits to be fully utilized for specifying memory size. The other very important thing is that size_t is platform dependent which means that on 32-bit systems it's size is 32 bits (4 bytes) and on 64-bit systems it's 64 bits (8 bytes).

* `void free( void* ptr );` This function is pretty simple, it doesn't return anything (void) and takes one argument - a pointer to a previously allocated memory. Notice how `ptr` only stores address of memory that we want to free, however it doesn't include information of HOW many bytes are allocated meaning that it "technically" shouldn't be able to free the right amount. We will cover this in section :[Memory Implementation as a Singly Linked List](#memory-implementation-as-a-singly-linked-list)

# *Memory Implementation as a Singly Linked List*
Now that we know what is a heap let's try to implem

