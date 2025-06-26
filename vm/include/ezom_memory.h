// ============================================================================
// File: src/include/ezom_memory.h
// Basic memory management
// ============================================================================

#pragma once
#include <stdint.h>

// ez80 24-bit address type support for AgonDev toolchain
#ifndef uint24_t
#ifdef __ez80__
// For AgonDev ez80 toolchain - use built-in 24-bit type
#define uint24_t unsigned long
#else
// For other platforms (testing)
typedef uint32_t uint24_t;
#endif
#endif

#define EZOM_HEAP_START     0x042000    // Start of object heap
#define EZOM_HEAP_SIZE      0x0E000     // 56KB heap space
#define EZOM_HEAP_END       (EZOM_HEAP_START + EZOM_HEAP_SIZE)

// Memory allocator state
typedef struct ezom_heap {
    uint24_t next_free;         // Next free address
    uint24_t heap_end;          // End of allocated space
    uint16_t objects_allocated; // Statistics
    uint16_t bytes_allocated;
} ezom_heap_t;

extern ezom_heap_t g_heap;

// Memory management functions
void ezom_init_memory(void);
uint24_t ezom_allocate(uint16_t size);
void ezom_memory_stats(void);