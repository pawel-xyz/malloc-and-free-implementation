# *Custom Memory Allocator* (`my_malloc()`)

A custom implementation of the standard C library memory allocation functions (`malloc` and `free`). This project serves as an educational dive into low-level systems programming, demonstrating how dynamic memory management works under the hood using a singly linked list architecture.

## *Table of Contents*

* [Understanding Computer Memory](#Understanding-computer-memory)
  * [What are `malloc` and `free`?](#what-are-malloc-and-free)
* [How It Works](#how-it-works)
  * [The Singly Linked List Architecture](#the-singly-linked-list-architecture)
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
