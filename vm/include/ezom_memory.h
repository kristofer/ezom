// ============================================================================
// File: vm/include/ezom_memory.h
// Basic memory management
// ============================================================================

#pragma once
#include <stdint.h>
#include <stdbool.h>
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

// Phase 3: Free List Allocator Constants
#define EZOM_SIZE_CLASSES   16      // Number of size classes for free lists
#define EZOM_LARGE_OBJECT_THRESHOLD 512  // Objects larger than this use large object heap

// Free block structure for linked lists
typedef struct ezom_free_block {
    struct ezom_free_block* next;   // Next free block in list
    uint16_t size;                  // Size of this block
    uint16_t padding;               // Alignment padding
} ezom_free_block_t;

// Memory allocator state
typedef struct ezom_heap {
    uint24_t next_free;         // Next free address
    uint24_t heap_end;          // End of allocated space
    uint16_t objects_allocated; // Statistics
    uint16_t bytes_allocated;
    
    // Phase 3: Enhanced statistics
    uint16_t total_allocations;     // Total allocation requests
    uint16_t allocation_failures;   // Failed allocations
    uint16_t peak_bytes_used;       // Peak memory usage
    uint16_t bytes_since_last_gc;   // Bytes allocated since last GC
    
    // Object type statistics
    uint16_t integer_objects;
    uint16_t string_objects;
    uint16_t array_objects;
    uint16_t block_objects;
    uint16_t other_objects;
    
    // Memory fragmentation tracking
    uint16_t largest_free_block;
    uint16_t free_block_count;
    
    // Phase 3: Free List Allocator
    uint24_t free_lists[EZOM_SIZE_CLASSES];     // Free list heads
    uint16_t free_counts[EZOM_SIZE_CLASSES];    // Count per size class
    uint24_t large_object_list;                 // Large objects linked list
    uint16_t large_object_count;                // Number of large objects
    bool use_free_lists;                        // Enable free list allocator
    
    // GC preparation
    uint16_t gc_threshold;          // Trigger GC after this many bytes
    bool gc_enabled;                // Whether GC is enabled
} ezom_heap_t;

extern ezom_heap_t g_heap;

// Memory management functions
void ezom_init_memory(void);
uint24_t ezom_allocate(uint16_t size);
uint24_t ezom_allocate_typed(uint16_t size, uint8_t object_type);  // Phase 3: With type tracking

// Phase 3: Free List Allocator Functions
uint8_t ezom_size_to_class(uint16_t size);
uint16_t ezom_class_to_size(uint8_t class_index);
uint24_t ezom_freelist_allocate(uint16_t size);
void ezom_freelist_deallocate(uint24_t ptr, uint16_t size);
void ezom_init_free_lists(void);
void ezom_enable_free_lists(bool enable);
void ezom_free_list_stats(void);

void ezom_memory_stats(void);
void ezom_cleanup_memory(void);

// Phase 3: Enhanced memory management
void ezom_track_allocation(uint8_t object_type, uint16_t size);
void ezom_detailed_memory_stats(void);
void ezom_memory_fragmentation_report(void);
uint16_t ezom_get_memory_pressure(void);
void ezom_set_gc_threshold(uint16_t threshold);
bool ezom_should_trigger_gc(void);

// Phase 3 Step 3: Object Marking System
#define EZOM_MAX_GC_ROOTS 64   // Maximum number of GC roots

// GC root set management
typedef struct ezom_gc_roots {
    uint24_t roots[EZOM_MAX_GC_ROOTS];   // Array of root object pointers
    uint8_t count;                       // Number of active roots
    bool gc_in_progress;                 // Flag to prevent recursive GC
} ezom_gc_roots_t;

extern ezom_gc_roots_t g_gc_roots;

// Object marking functions
void ezom_init_marking_system(void);
bool ezom_is_marked(uint24_t obj);
void ezom_mark_object(uint24_t obj);
void ezom_unmark_object(uint24_t obj);
void ezom_clear_all_marks(void);

// Reference traversal functions
void ezom_mark_object_references(uint24_t obj);
void ezom_mark_from_roots(void);
uint16_t ezom_count_marked_objects(void);
uint16_t ezom_count_unmarked_objects(void);

// GC root management
void ezom_add_gc_root(uint24_t obj);
void ezom_remove_gc_root(uint24_t obj);
void ezom_clear_gc_roots(void);
void ezom_list_gc_roots(void);

// Mark phase execution
void ezom_mark_phase(void);
void ezom_marking_stats(void);

// Sweep detection (identifies objects for collection)
uint16_t ezom_identify_garbage(uint24_t* garbage_list, uint16_t max_objects);
void ezom_sweep_detection_stats(void);