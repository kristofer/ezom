// ============================================================================
// File: src/vm/memory.c
// Basic memory allocation implementation
// ============================================================================

#include "../include/ezom_memory.h"
#include "../include/ezom_object.h"
#include <stdio.h>
#include <string.h>

ezom_heap_t g_heap;

void ezom_init_memory(void) {
    g_heap.next_free = EZOM_HEAP_START;
    g_heap.heap_end = EZOM_HEAP_END;
    g_heap.objects_allocated = 0;
    g_heap.bytes_allocated = 0;
    
    printf("EZOM: Memory initialized, %dKB heap at 0x%06X\n", 
           EZOM_HEAP_SIZE / 1024, EZOM_HEAP_START);
}

uint24_t ezom_allocate(uint16_t size) {
    // Align to 2-byte boundary
    size = (size + 1) & ~1;
    
    // Check if we have space
    if (g_heap.next_free + size > g_heap.heap_end) {
        printf("EZOM: Out of memory! Requested %d bytes\n", size);
        return 0;
    }
    
    uint24_t ptr = g_heap.next_free;
    g_heap.next_free += size;
    g_heap.objects_allocated++;
    g_heap.bytes_allocated += size;
    
    // Clear allocated memory
    memset((void*)ptr, 0, size);
    
    return ptr;
}

void ezom_memory_stats(void) {
    uint16_t used = g_heap.bytes_allocated;
    uint16_t total = EZOM_HEAP_SIZE;
    
    printf("Memory: %d/%d bytes used (%d%%), %d objects\n",
           used, total, (used * 100) / total, g_heap.objects_allocated);
}