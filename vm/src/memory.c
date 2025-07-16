// ============================================================================
// File: src/vm/memory.c
// Basic memory allocation implementation
// ============================================================================

#include "../include/ezom_memory.h"
#include "../include/ezom_object.h"
#include "../include/ezom_platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ezom_heap_t g_heap;

void ezom_init_memory(void) {
#ifdef EZOM_PLATFORM_NATIVE
    // For native development - allocate heap using malloc
    g_heap_base = malloc(EZOM_HEAP_SIZE);
    if (!g_heap_base) {
        printf("EZOM: Failed to allocate heap memory!\n");
        exit(1);
    }
    printf("EZOM: Native heap allocated, %dKB heap at %p\n", 
           EZOM_HEAP_SIZE / 1024, g_heap_base);
#else
    printf("EZOM: Memory initialized, %dKB heap at 0x%06X\n", 
           EZOM_HEAP_SIZE / 1024, EZOM_HEAP_START);
#endif
    
    g_heap.next_free = EZOM_HEAP_START;
    g_heap.heap_end = EZOM_HEAP_END;
    g_heap.objects_allocated = 0;
    g_heap.bytes_allocated = 0;
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
#ifdef EZOM_PLATFORM_NATIVE
    memset(ezom_ptr_to_native(ptr), 0, size);
#else
    memset((void*)ptr, 0, size);
#endif
    
    return ptr;
}

void ezom_memory_stats(void) {
    uint16_t used = g_heap.bytes_allocated;
    uint32_t total = EZOM_HEAP_SIZE;
    
    printf("Memory: %d/%d bytes used (%d%%), %d objects\n",
           used, (int)total, (used * 100) / (int)total, g_heap.objects_allocated);
}

void ezom_cleanup_memory(void) {
#ifdef EZOM_PLATFORM_NATIVE
    // Free the heap memory on native platforms
    if (g_heap_base) {
        free(g_heap_base);
        g_heap_base = NULL;
    }
#endif
}