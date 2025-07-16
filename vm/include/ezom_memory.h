// ============================================================================
// File: vm/include/ezom_memory.h
// Basic memory management
// ============================================================================

#pragma once
#include <stdint.h>
#include "ezom_platform.h"

// Memory layout - platform specific
#ifdef EZOM_PLATFORM_EZ80
#define EZOM_HEAP_START     0x042000    // Start of object heap on ez80
#define EZOM_HEAP_SIZE      0x0E000     // 56KB heap space
#else
// For native development - use allocated heap with base address
#define EZOM_HEAP_START     0x042000    // Virtual base address for compatibility
#define EZOM_HEAP_SIZE      0x40000     // 256KB heap for native (more generous)
#endif
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
void ezom_cleanup_memory(void);