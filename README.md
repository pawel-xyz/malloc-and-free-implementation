# *Custom Memory Allocator* (`my_malloc()`)

A custom implementation of the standard C library memory allocation functions (`malloc` and `free`). This project serves as an educational dive into low-level systems programming, demonstrating how dynamic memory management works under the hood using a singly linked list architecture.

## *Table of Contents*

* [Understanding Computer Memory](#Understanding-computer-memory)
  * [What is `malloc` and `free`?](#what-is-malloc-and-free)
* [Memory Implementation as a Singly Linked List](#-memory-implementation-as-a-singly-linked-list)
   * [The Initial Heap, Program Break, and sbrk()](#the-initial-heap-program-break-and-sbrk)
   * [Metadata and why it is important (Header + Payload)](#metadata-and-why-it-is-important-header--payload)
   * [Returning the address - how malloc handles it](#returning-the-address---how-malloc-handles-it)
* [Key Helper Functions](#Key-helper-functions)
  * [`AlignBytes` – Data Alignment](#alignbytessize_t-size--data-alignment)
  * [Internal and External Fragmentation](#internal-and-external-fragmentation)
  * [`Block_Splitting` – Preventing Internal Fragmentation](#block_splittingsize_t-size-mem_block-blockfound--preventing-internal-fragmentation)
* [Free Block Search Algorithms](#free-block-search-algorithms)

# *Understanding Computer Memory*
To understand how `malloc` works, it is essential to first understand how a computer manages memory. We can think of a computer's Random Access Memory (RAM) as a massive array of storage bytes, each with its own unique memory address. When a program is executed, the operating system isolates a specific layout of virtual memory for it to use.
Every program operates using virtual memory an operating system feature that allows hard disk space to be used as RAM. Thanks to virtual memory, processes are under the illusion that the entire memory space is available to them; the hardware translates these virtual addresses into actual physical RAM addresses on the fly.

The structure of the address space can be visualized as a vertical block: low addresses—starting at 0x00000000 are at the very bottom, while high addresses—0xFFFFFFFF are at the very top.
The RAM is divided into couple of different segments :
  * **Text Segment** : This is the section containing machine instructions that the processor executes directly such as program functions and instructions. This section is read-only.
  * **Initialized Data Segment** : Global and static variables that have already been assigned a value are stored here.
  * **Uninitialized Data Segment** : This is the place for uninitialized global and static variables.
  * **Heap** : Used for dynamic memory allocation. This is an unorganized pool of memory utilized when the exact memory requirement is unknown at compile time (e.g., handling dynamic arrays or user inputs). The programmer is fully responsible for manually requesting and releasing heap memory.
  * **Stack** : Used for static memory allocation. It automatically manages local variables, function parameters, and control flow. While operations on the stack are very fast, its size is strictly fixed and small.

If we would want to visualise this concept it would look something like this : 

<img width="1024" height="509" alt="image" src="https://github.com/user-attachments/assets/e046aae7-a62c-43fa-bb67-4d5aabd981ad" />

## *What is malloc() and free() ?* 
malloc() and free() are basic C functions used by a programmer to manually allocate memory and then deallocate it. Both of these functions are included in `<stdlib.h>` which is a standard library in C programming language.

### *The structure of malloc() and free()* : 
`void* malloc( size_t size );`
  
`void*` means that `malloc()` returns pointer to void. That makes perfect sense because the only job of `malloc()` function is to retrieve a specified number of bytes from the operating system and reserve them in memory. This function does not know WHAT type of data is stored at that address. Let's assume that `malloc()` doesn't return void*. The people who created C would have to create special `malloc()` functions for every data type (e.g. `int* malloc_int(size_t size)`).

`size_t` is a special data type (size_t, "size" it's just a variable name) used to represent object's size in bytes. A memory size can never be negative. For this reason, `size_t` is an unsigned integer type (it does not accept values ​​less than zero). This allows the available range of bits to be fully utilized for specifying memory size. The other very important thing is that `size_t` is platform dependent, which means that on 32-bit systems it's size is 32 bits (4 bytes) and on 64-bit systems it's 64 bits (8 bytes).

`void free( void* ptr );` 
This function is pretty simple, it doesn't return anything (void) and takes one argument - a pointer to a previously allocated memory. Notice how `ptr` only stores address of memory that we want to free, however it doesn't include information of HOW many bytes are allocated, meaning that it "technically" shouldn't be able to free the right amount of bytes. We will cover this "problem of unknown size" in section [Memory Implementation as a Singly Linked List](#memory-implementation-as-a-singly-linked-list)

# *Memory Implementation as a Singly Linked List*
The operating system sees the heap as a huge "box" of bytes. To know which parts of the "box" are occupied and which are free, we use a **singly linked list** structure. But before we can link anything, we need to understand how we actually get memory from the OS.

### *The Initial Heap, Program Break, and sbrk()*

When a C program starts, the initial size of the heap is exactly **0 bytes**. It doesn't exist yet. 

The operating system keeps track of the top boundary of your program's data segment using a special pointer called the **program break** (or simply *brk*).Everything **below** the program break is memory that your program has been granted and can safely use. Everything **above** the program break belongs to the OS. If our program tries to touch it, the Operating System will immediately terminate it with a *Segmentation Fault*. Let's go back to the previous image covered in [Understanding Computer Memory](#Understanding-computer-memory) but now lets add **program break** to it : 

<img width="1024" height="509" alt="image" src="https://github.com/user-attachments/assets/223dc7c5-955a-4600-a8e0-8eaa79ec38da" />


To ask the OS for memory (which means pushing program brake "up") our allocator uses a system call wrapper named `sbrk()` (set break):
* `sbrk(0)`: Returns the current address of the program break without changing it.
* `sbrk(size)`: Moves the program break up by `size` bytes, effectively growing the heap. 

**!!!** When you call `sbrk(size)`, it returns the **OLD** program break address. This is incredibly convenient because this old address is exactly where our newly allocated chunk of memory begins! We can immediately cast this raw memory address into our Metadata structure (Header) and append it to our linked list.

### *Metadata and why it is important (Header + Payload)*
Let's say that we want to allocate 24 bytes of memory, naturally we type `malloc(24)` which means that the system allocated exactly 24 bytes right? Not quite, to understand why we need to go back to section [The structure of malloc() and free()](#the-structure-of-malloc()-and-free()). There we talked about how `free()` function shouldn't be able to know how many bytes of memory is there to free. When we call `malloc(24)` the OS reservers a bit more than 24 bytes, because right before our data (Payload), it has to hide a secret structure with information, the so-called **Metadata (Header)**. The Header stores critical information: how big the block is, whether it is free, and a pointer to the next memory block on the heap.
In this project, the Header looks like this:

```c
typedef struct Mem_Block {
    size_t Size_Alloc;       // Size of the block
    int isFree;              // 1 = free, 0 = allocated
    struct Mem_Block* pNext; // Address of the next block
} Mem_Block;
```

So the total amount of bytes reserved can be calculated by simple equation: 
                                 *TotalSize = Amount of bytes called by user + sizeof(Mem_Block)*                           

<img width="1024" height="572" alt="image" src="https://github.com/user-attachments/assets/6c842adc-44c0-4af3-9364-5a2e2881d136" />


### *Returning the address - how malloc handles it*
If `malloc` returned the address of the very beginning of the block (where the Header is), the user would immediately overwrite the metadata with their own data! To prevent this, a specific pointer arithmetic operation is used. Internally, `malloc` uses two levels of management:

1. It operates on `Mem_Block*` pointers to modify list parameters and ask the system for memory (using the `sbrk` function).
2. When returning the result to the user, the pointer is shifted **past the metadata**.

In the code, this is implemented by a simple instruction: `return current + 1;`
Adding "1" to a pointer of type `Mem_Block*` shifts the address to the right by exactly the size of the Header structure, landing perfectly at the beginning of the Payload section.

# *Key Helper Functions*
### *AlignBytes – Data Alignment*
When building a custom memory allocator, simply allocating the exact byte size requested by the user is not enough. The `AlignBytes` function is responsible for rounding up the requested allocation size to a specific memory boundary (typically a multiple of 8 bytes on a 64-bit architecture).

**Why is alignment important?**

A CPU (Central Processing Unit) does not read memory byte-by-byte. It fetches memory in fixed-size chunks called **words** (e.g., 8 bytes at a time for a 64-bit processor). If a data structure is unaligned, meaning it's data crosses the boundary between two adjacent words the CPU is forced to perform two separate memory read cycles to fetch a single value, stitch them together, and then process them. This heavily degrades program performance. On some strict architectures (like ARM - Advanced RISC Machine), unaligned memory access is not permitted at all and will immediately crash the program with a hardware exception (bus error). `AlignBytes` guarantees that the payload always starts at a perfectly aligned memory address.

**Implementation Approaches**

Our repository will cover two different ways to calculate memory alignment to demonstrate the difference in their efficiency:

1. *The "Modulo" Way*

   This method relies on standard arithmetic operations (addition, division, and the modulo `%` operator). It mathematically checks if the requested size is a multiple of the alignment requirement, and if not, calculates the exact remainder to add the necessary padding bytes.
   * *Efficiency:* Low. Division and modulo are mathematically intuitive but computationally expensive. They are among the slowest instructions for a CPU's ALU (Arithmetic Logic Unit) to execute, often taking couple of clock cycles.

2. *The "Bit-Masking" Way*

   This method leverages bitwise operators (`&` - AND, `~` - NOT). Because memory alignment requirements are always powers of 2 (4, 8, 16, etc.), their binary representations are highly predictable. The number 8 in binary is 1000, number 16 in binary is 10000 etc. The algorithm adds `(ALIGNMENT - 1)` to the requested size and then uses a bitwise AND `&` with the inverted mask `~(ALIGNMENT - 1)` to instantly clear the lower bits, snapping the value to the nearest aligned boundary.
   * *Efficiency:* High. Bitwise operations are processed directly at the logic-gate level inside the CPU, typically executing in just a single clock cycle. This is the standard for writing high-performance low-level system code.
  
### Internal and External Fragmentation
* **Internal Fragmentation** occurs when the system allocates slightly more memory than requested (e.g., due to rounding up/alignment for CPU optimization). The remaining bytes inside the allocated block are wasted.
* **External Fragmentation** arises when we have many small, free blocks scattered throughout memory. In total, we might have 1024 bytes of free space, but no single contiguous block is large enough to handle a `malloc(512)` request.

It is very important that our function for allocating memory is prepared to minimize the chance of Fragmentation.

### *Block Splitting and Coalescing*

**Block Splitting (Preventing Internal Fragmentation)**

When `my_malloc()` iterates through the linked list and finds a free block that is significantly larger than the requested size, it performs a "split". Giving the entire large block to a small allocation request would waste the remaining space inside that block a -critical issue known as **Internal Fragmentation**. In our implementation, the `Block_Splitting()` function prevents this. It calculates the memory offset where the requested payload ends, creates a brand new `Header` (metadata) at that exact location for the leftover space, marks this new smaller block as `isFree = 1`, and correctly wires it into the existing linked list by updating the `pNext` pointers. In that case our function doesn't waste any bytes !

**Coalescing (Preventing External Fragmentation)**

While block splitting optimizes space, it has a side effect: over time, the heap becomes littered with tiny, separated free blocks. If a user subsequently requests a massive memory chunk, `my_malloc()` might fail to find a single block large enough, even if the *total* free memory across all tiny blocks is sufficient. The exact problem covered in [Internal and External Fragmentation](#internal-and-external-fragmentation). To fix this, `my_free()` implements **Coalescing**. Whenever memory is freed, the allocator looks at the neighboring blocks. If it detects that an adjacent block is also free, it merges them by adding their sizes together and bypassing the intermediate headers. This consolidates scattered memory back into large, contiguous free blocks ready for "big" (big in size) allocations.


# Free Block Search Algorithms
When `my_malloc` traverses the linked list to satisfy a memory request, different search strategies can be employed:

| Algorithm | How It Works | Pros | Cons |
| :--- | :--- | :--- | :--- |
| **First Fit** | Scans the linked list from the beginning and picks the *first* free block large enough to fit the request. | Fast allocation; simple implementation. | Tends to accumulate small, unusable free blocks at the beginning of the list over time. |
| **Next Fit** | Similar to First Fit, but starts searching from the location of the *last* allocated block instead of the head. | Faster searches when the list is long; spreads allocations throughout memory. | Poor memory locality; prone to higher overall external fragmentation. |
| **Best Fit** | Traverses the *entire* list to find the free block that is closest in size to the requested size. | Leaves larger contiguous blocks intact for future large requests; minimizes wasted space per block. | Slow allocation speed ($O(N)$ traversal time); produces tiny "leftover" fragments that are too small to use. |
| **Worst Fit** | Traverses the *entire* list and picks the *largest* available free block, then splits it. | Reduces the generation of unusable micro-fragments by leaving split remainders as large as possible. | Slow traversal ($O(N)$ time); quickly destroys large contiguous blocks needed for big allocations. |
| **Quick Fit / Segregated Fit** | Maintains distinct linked lists for common allocation sizes (e.g., 8B, 16B, 32B, 64B). | Extremely fast $O(1)$ allocation time for standard sizes. | More complex metadata structure; higher overhead memory footprint. |
| **Buddy System (Buddy Search)** | Divides memory into power-of-two blocks ($2^k$). Recursively splits blocks in halves ("buddies") until a small enough fit is found, and coalesces buddies back upon `free`. | Lightning-fast allocation and coalescing ($O(\log N)$ or $O(1)$ via bitwise operations); minimal external fragmentation. | High internal fragmentation because allocation sizes are forced to round up to the nearest power of 2 (e.g., 65 bytes requires 128 bytes). |
