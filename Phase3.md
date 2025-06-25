# EZOM Phase 3 Implementation Specification
## Memory Management and Garbage Collection

**Target Platform:** ez80 Agon Light 2  
**Implementation Language:** C with AgDev toolchain  
**Timeline:** Weeks 5-6  
**Dependencies:** Phases 1-2 completed

---

## Phase 3: Memory Management (Weeks 5-6)

### 3.1 Overview

Phase 3 implements a complete memory management system with mark-and-sweep garbage collection, optimized for the ez80's 24-bit address space and limited RAM. The design prioritizes low pause times, efficient memory utilization, and integration with the existing object system.

### 3.2 Memory Architecture

#### 3.2.1 Enhanced Memory Layout

```c
// ezom_memory.h - Enhanced for Phase 3
#pragma once
#include <stdint.h>
#include "ezom_object.h"

// Memory regions (512KB total)
#define EZOM_HEAP_START         0x042000    // Object heap start
#define EZOM_HEAP_SIZE          0x00E000    // 56KB main heap
#define EZOM_LARGE_HEAP_START   0x050000    // Large object heap
#define EZOM_LARGE_HEAP_SIZE    0x008000    // 32KB large objects
#define EZOM_STACK_START        0x058000    // GC mark stack
#define EZOM_STACK_SIZE         0x002000    // 8KB mark stack
#define EZOM_FREE_LIST_START    0x05A000    // Free list metadata
#define EZOM_FREE_LIST_SIZE     0x001000    // 4KB free lists
#define EZOM_STRING_POOL_START  0x05B000    // String interning pool
#define EZOM_STRING_POOL_SIZE   0x005000    // 20KB string pool

// Memory block header for free list management
typedef struct ezom_free_block {
    uint16_t size;              // Size of this block
    uint24_t next;              // Next free block of same size class
    uint8_t  flags;             // Block flags
} ezom_free_block_t;

// Free list size classes (powers of 2 + common sizes)
#define EZOM_SIZE_CLASSES       16
#define EZOM_MIN_BLOCK_SIZE     8
#define EZOM_MAX_SMALL_BLOCK    256

extern uint24_t g_free_lists[EZOM_SIZE_CLASSES];
```

#### 3.2.2 Heap Manager Structure

```c
// Enhanced heap manager
typedef struct ezom_heap_manager {
    // Allocation tracking
    uint24_t heap_start;
    uint24_t heap_end;
    uint24_t next_free;
    uint24_t large_heap_start;
    uint24_t large_heap_end;
    
    // Statistics
    uint32_t total_allocated;
    uint32_t total_freed;
    uint16_t objects_alive;
    uint16_t gc_cycles;
    uint16_t bytes_since_gc;
    
    // GC thresholds
    uint16_t gc_threshold;          // Trigger GC after this many bytes
    uint16_t emergency_threshold;   // Emergency GC threshold
    
    // Free list management
    uint24_t free_lists[EZOM_SIZE_CLASSES];
    uint16_t free_counts[EZOM_SIZE_CLASSES];
    
    // Large object tracking
    uint24_t large_objects;         // Linked list of large objects
    uint16_t large_object_count;
    
    // GC state
    bool gc_in_progress;
    uint16_t gc_mark_stack_top;
    uint24_t gc_mark_stack[256];    // Mark stack for non-recursive marking
} ezom_heap_manager_t;

extern ezom_heap_manager_t g_heap_manager;
```

### 3.3 Free List Allocator

#### 3.3.1 Size Class Management

```c
// ezom_freelist.c

// Map size to size class index
static uint8_t ezom_size_to_class(uint16_t size) {
    if (size <= 8) return 0;       // 8 bytes
    if (size <= 16) return 1;      // 16 bytes
    if (size <= 24) return 2;      // 24 bytes (common for small objects)
    if (size <= 32) return 3;      // 32 bytes
    if (size <= 48) return 4;      // 48 bytes
    if (size <= 64) return 5;      // 64 bytes
    if (size <= 96) return 6;      // 96 bytes
    if (size <= 128) return 7;     // 128 bytes
    if (size <= 192) return 8;     // 192 bytes
    if (size <= 256) return 9;     // 256 bytes
    
    // Larger size classes for bigger objects
    for (uint8_t i = 10; i < EZOM_SIZE_CLASSES; i++) {
        if (size <= (32 << (i - 10))) {
            return i;
        }
    }
    
    return EZOM_SIZE_CLASSES - 1;  // Largest class
}

// Get actual size for size class
static uint16_t ezom_class_to_size(uint8_t class_index) {
    static const uint16_t sizes[] = {
        8, 16, 24, 32, 48, 64, 96, 128, 192, 256,
        512, 1024, 2048, 4096, 8192, 16384
    };
    
    return sizes[class_index];
}

// Allocate from free list
uint24_t ezom_freelist_allocate(uint16_t requested_size) {
    // Align size to 2-byte boundary
    uint16_t size = (requested_size + 1) & ~1;
    uint8_t class_index = ezom_size_to_class(size);
    
    // Check if we have a free block of this size
    uint24_t block_ptr = g_heap_manager.free_lists[class_index];
    
    if (block_ptr) {
        // Remove from free list
        ezom_free_block_t* block = (ezom_free_block_t*)block_ptr;
        g_heap_manager.free_lists[class_index] = block->next;
        g_heap_manager.free_counts[class_index]--;
        
        // Clear the block
        memset((void*)block_ptr, 0, block->size);
        
        return block_ptr;
    }
    
    // Try larger size classes
    for (uint8_t i = class_index + 1; i < EZOM_SIZE_CLASSES; i++) {
        block_ptr = g_heap_manager.free_lists[i];
        if (block_ptr) {
            ezom_free_block_t* block = (ezom_free_block_t*)block_ptr;
            g_heap_manager.free_lists[i] = block->next;
            g_heap_manager.free_counts[i]--;
            
            // Split block if significantly larger
            uint16_t block_size = block->size;
            uint16_t needed_size = ezom_class_to_size(class_index);
            
            if (block_size >= needed_size + sizeof(ezom_free_block_t) + 8) {
                // Split the block
                uint24_t remainder_ptr = block_ptr + needed_size;
                ezom_free_block_t* remainder = (ezom_free_block_t*)remainder_ptr;
                remainder->size = block_size - needed_size;
                remainder->flags = 0;
                
                // Add remainder back to appropriate free list
                ezom_freelist_add_block(remainder_ptr, remainder->size);
            }
            
            // Clear the allocated portion
            memset((void*)block_ptr, 0, needed_size);
            
            return block_ptr;
        }
    }
    
    // No suitable free block found - allocate new
    return ezom_allocate_new_block(size);
}

// Add block to free list
void ezom_freelist_add_block(uint24_t block_ptr, uint16_t size) {
    uint8_t class_index = ezom_size_to_class(size);
    
    ezom_free_block_t* block = (ezom_free_block_t*)block_ptr;
    block->size = size;
    block->next = g_heap_manager.free_lists[class_index];
    block->flags = 0;
    
    g_heap_manager.free_lists[class_index] = block_ptr;
    g_heap_manager.free_counts[class_index]++;
}
```

#### 3.3.2 Large Object Allocation

```c
// Handle objects larger than EZOM_MAX_SMALL_BLOCK
uint24_t ezom_allocate_large_object(uint16_t size) {
    // Align to 4-byte boundary for large objects
    size = (size + 3) & ~3;
    
    // Check if we have space in large object heap
    if (g_heap_manager.large_heap_start + size > g_heap_manager.large_heap_end) {
        // Try garbage collection first
        ezom_collect_garbage();
        
        if (g_heap_manager.large_heap_start + size > g_heap_manager.large_heap_end) {
            return 0; // Out of memory
        }
    }
    
    uint24_t ptr = g_heap_manager.large_heap_start;
    g_heap_manager.large_heap_start += size;
    
    // Add to large object tracking list
    ezom_large_object_header_t* header = (ezom_large_object_header_t*)ptr;
    header->size = size;
    header->next = g_heap_manager.large_objects;
    header->flags = EZOM_FLAG_LARGE_OBJECT;
    
    g_heap_manager.large_objects = ptr;
    g_heap_manager.large_object_count++;
    
    return ptr + sizeof(ezom_large_object_header_t);
}

// Large object header
typedef struct ezom_large_object_header {
    uint16_t size;
    uint24_t next;      // Next large object
    uint8_t  flags;
} ezom_large_object_header_t;
```

### 3.4 Mark-and-Sweep Garbage Collector

#### 3.4.1 GC Root Management

```c
// ezom_gc.h
#define EZOM_MAX_ROOTS      64
#define EZOM_MAX_MARK_STACK 256

typedef struct ezom_gc_roots {
    uint24_t roots[EZOM_MAX_ROOTS];
    uint8_t  count;
    
    // Special roots
    uint24_t global_objects;    // Global object table
    uint24_t symbol_table;      // Interned symbols
    uint24_t class_table;       // All classes
    uint24_t current_context;   // Active execution context
} ezom_gc_roots_t;

extern ezom_gc_roots_t g_gc_roots;

// Root management functions
void ezom_gc_add_root(uint24_t object);
void ezom_gc_remove_root(uint24_t object);
void ezom_gc_push_root(uint24_t object);    // Temporary root
uint24_t ezom_gc_pop_root(void);
```

#### 3.4.2 Mark Phase Implementation

```c
// ezom_gc.c

// Non-recursive marking using explicit stack
void ezom_gc_mark_phase(void) {
    // Clear all mark bits
    ezom_gc_clear_marks();
    
    // Initialize mark stack
    g_heap_manager.gc_mark_stack_top = 0;
    g_heap_manager.gc_in_progress = true;
    
    // Add all roots to mark stack
    for (uint8_t i = 0; i < g_gc_roots.count; i++) {
        if (g_gc_roots.roots[i]) {
            ezom_gc_push_mark_stack(g_gc_roots.roots[i]);
        }
    }
    
    // Mark special roots
    if (g_gc_roots.global_objects) {
        ezom_gc_push_mark_stack(g_gc_roots.global_objects);
    }
    if (g_gc_roots.symbol_table) {
        ezom_gc_push_mark_stack(g_gc_roots.symbol_table);
    }
    if (g_gc_roots.class_table) {
        ezom_gc_push_mark_stack(g_gc_roots.class_table);
    }
    if (g_gc_roots.current_context) {
        ezom_gc_push_mark_stack(g_gc_roots.current_context);
    }
    
    // Process mark stack
    while (g_heap_manager.gc_mark_stack_top > 0) {
        uint24_t object = ezom_gc_pop_mark_stack();
        ezom_gc_mark_object(object);
    }
}

void ezom_gc_mark_object(uint24_t object_ptr) {
    if (!object_ptr || !ezom_is_valid_object(object_ptr)) {
        return;
    }
    
    ezom_object_t* object = (ezom_object_t*)object_ptr;
    
    // Skip if already marked
    if (object->flags & EZOM_FLAG_MARKED) {
        return;
    }
    
    // Mark this object
    object->flags |= EZOM_FLAG_MARKED;
    
    // Mark object-specific references
    uint8_t object_type = object->flags & 0xF0;
    
    switch (object_type) {
        case EZOM_TYPE_OBJECT:
            ezom_gc_mark_generic_object(object_ptr);
            break;
            
        case EZOM_TYPE_CLASS:
            ezom_gc_mark_class_object(object_ptr);
            break;
            
        case EZOM_TYPE_ARRAY:
            ezom_gc_mark_array_object(object_ptr);
            break;
            
        case EZOM_TYPE_STRING:
        case EZOM_TYPE_INTEGER:
            // No references to mark
            break;
            
        default:
            // Unknown type - treat as generic object
            ezom_gc_mark_generic_object(object_ptr);
            break;
    }
    
    // Always mark the class
    if (object->class_ptr) {
        ezom_gc_push_mark_stack(object->class_ptr);
    }
}

void ezom_gc_mark_class_object(uint24_t class_ptr) {
    ezom_class_t* class_obj = (ezom_class_t*)class_ptr;
    
    // Mark superclass
    if (class_obj->superclass) {
        ezom_gc_push_mark_stack(class_obj->superclass);
    }
    
    // Mark method dictionary
    if (class_obj->method_dict) {
        ezom_gc_push_mark_stack(class_obj->method_dict);
    }
    
    // Mark instance variable names
    if (class_obj->instance_vars) {
        ezom_gc_push_mark_stack(class_obj->instance_vars);
    }
}

void ezom_gc_mark_array_object(uint24_t array_ptr) {
    ezom_array_t* array = (ezom_array_t*)array_ptr;
    
    // Mark all array elements
    for (uint16_t i = 0; i < array->size; i++) {
        if (array->elements[i]) {
            ezom_gc_push_mark_stack(array->elements[i]);
        }
    }
}

void ezom_gc_mark_generic_object(uint24_t object_ptr) {
    ezom_object_t* object = (ezom_object_t*)object_ptr;
    ezom_class_t* class_obj = (ezom_class_t*)object->class_ptr;
    
    if (!class_obj) return;
    
    // Mark instance variables
    uint24_t* instance_vars = (uint24_t*)(object_ptr + sizeof(ezom_object_t));
    
    for (uint16_t i = 0; i < class_obj->instance_var_count; i++) {
        if (instance_vars[i]) {
            ezom_gc_push_mark_stack(instance_vars[i]);
        }
    }
}
```

#### 3.4.3 Sweep Phase Implementation

```c
// Sweep phase - reclaim unmarked objects
void ezom_gc_sweep_phase(void) {
    uint16_t objects_freed = 0;
    uint32_t bytes_freed = 0;
    
    // Sweep small objects
    ezom_gc_sweep_small_objects(&objects_freed, &bytes_freed);
    
    // Sweep large objects
    ezom_gc_sweep_large_objects(&objects_freed, &bytes_freed);
    
    // Update statistics
    g_heap_manager.total_freed += bytes_freed;
    g_heap_manager.objects_alive -= objects_freed;
    g_heap_manager.gc_cycles++;
    g_heap_manager.bytes_since_gc = 0;
    
    // Compact free lists
    ezom_gc_compact_free_lists();
    
    printf("GC: Freed %d objects, %ld bytes\n", objects_freed, bytes_freed);
}

void ezom_gc_sweep_small_objects(uint16_t* objects_freed, uint32_t* bytes_freed) {
    uint24_t current = EZOM_HEAP_START;
    uint24_t heap_end = g_heap_manager.next_free;
    
    while (current < heap_end) {
        ezom_object_t* object = (ezom_object_t*)current;
        
        if (!ezom_is_valid_object_header(object)) {
            // Skip invalid or free blocks
            current += sizeof(ezom_object_t);
            continue;
        }
        
        uint16_t object_size = ezom_get_object_size(current);
        
        if (object->flags & EZOM_FLAG_MARKED) {
            // Clear mark bit for next GC cycle
            object->flags &= ~EZOM_FLAG_MARKED;
        } else {
            // Object is garbage - add to free list
            ezom_finalize_object(current);
            ezom_freelist_add_block(current, object_size);
            
            (*objects_freed)++;
            (*bytes_freed) += object_size;
        }
        
        current += object_size;
    }
}

void ezom_gc_sweep_large_objects(uint16_t* objects_freed, uint32_t* bytes_freed) {
    uint24_t* current_ptr = &g_heap_manager.large_objects;
    
    while (*current_ptr) {
        uint24_t object_ptr = *current_ptr + sizeof(ezom_large_object_header_t);
        ezom_object_t* object = (ezom_object_t*)object_ptr;
        ezom_large_object_header_t* header = (ezom_large_object_header_t*)*current_ptr;
        
        if (object->flags & EZOM_FLAG_MARKED) {
            // Keep object, clear mark bit
            object->flags &= ~EZOM_FLAG_MARKED;
            current_ptr = &header->next;
        } else {
            // Remove from list and free
            *current_ptr = header->next;
            
            ezom_finalize_object(object_ptr);
            
            (*objects_freed)++;
            (*bytes_freed) += header->size;
            g_heap_manager.large_object_count--;
            
            // Large objects are not added to free lists
            // They will be allocated fresh next time
        }
    }
}
```

### 3.5 Memory Compaction

#### 3.5.1 Free List Compaction

```c
// Merge adjacent free blocks to reduce fragmentation
void ezom_gc_compact_free_lists(void) {
    for (uint8_t class_index = 0; class_index < EZOM_SIZE_CLASSES; class_index++) {
        ezom_gc_compact_size_class(class_index);
    }
}

void ezom_gc_compact_size_class(uint8_t class_index) {
    uint24_t current = g_heap_manager.free_lists[class_index];
    uint24_t prev = 0;
    
    while (current) {
        ezom_free_block_t* block = (ezom_free_block_t*)current;
        uint24_t next = block->next;
        
        // Try to merge with adjacent blocks
        uint24_t merged_size = ezom_try_merge_adjacent(current, block->size);
        
        if (merged_size > block->size) {
            // Block was merged - update size and possibly move to different class
            uint8_t new_class = ezom_size_to_class(merged_size);
            
            if (new_class != class_index) {
                // Remove from current list
                if (prev) {
                    ((ezom_free_block_t*)prev)->next = next;
                } else {
                    g_heap_manager.free_lists[class_index] = next;
                }
                g_heap_manager.free_counts[class_index]--;
                
                // Add to new list
                block->size = merged_size;
                ezom_freelist_add_block(current, merged_size);
                
                current = next;
                continue;
            } else {
                block->size = merged_size;
            }
        }
        
        prev = current;
        current = next;
    }
}

uint16_t ezom_try_merge_adjacent(uint24_t block_ptr, uint16_t block_size) {
    uint24_t block_end = block_ptr + block_size;
    uint16_t total_size = block_size;
    
    // Check if the next memory location is also a free block
    if (block_end < g_heap_manager.next_free) {
        ezom_free_block_t* next_block = (ezom_free_block_t*)block_end;
        
        if (ezom_is_free_block(next_block)) {
            // Remove next block from its free list
            ezom_freelist_remove_block(block_end);
            total_size += next_block->size;
            
            // Recursively try to merge more blocks
            return ezom_try_merge_adjacent(block_ptr, total_size);
        }
    }
    
    return total_size;
}
```

### 3.6 Finalization Support

#### 3.6.1 Finalizer Registry

```c
// ezom_finalize.h
typedef void (*ezom_finalizer_fn)(uint24_t object);

typedef struct ezom_finalizer_entry {
    uint24_t object;
    ezom_finalizer_fn finalizer;
    struct ezom_finalizer_entry* next;
} ezom_finalizer_entry_t;

typedef struct ezom_finalizer_registry {
    ezom_finalizer_entry_t* entries;
    uint16_t count;
} ezom_finalizer_registry_t;

extern ezom_finalizer_registry_t g_finalizer_registry;

// Register finalizer for object
void ezom_register_finalizer(uint24_t object, ezom_finalizer_fn finalizer);

// Run finalizers for garbage objects
void ezom_run_finalizers(void);
```

```c
// ezom_finalize.c

void ezom_finalize_object(uint24_t object_ptr) {
    ezom_object_t* object = (ezom_object_t*)object_ptr;
    
    // Check if object has a finalizer
    if (object->flags & EZOM_FLAG_FINALIZE) {
        ezom_finalizer_entry_t* entry = ezom_find_finalizer_entry(object_ptr);
        if (entry && entry->finalizer) {
            entry->finalizer(object_ptr);
        }
        
        // Remove from finalizer registry
        ezom_remove_finalizer_entry(object_ptr);
    }
    
    // Object-specific cleanup
    uint8_t object_type = object->flags & 0xF0;
    
    switch (object_type) {
        case EZOM_TYPE_STRING:
            // Strings might be in intern table
            ezom_remove_from_intern_table(object_ptr);
            break;
            
        case EZOM_TYPE_ARRAY:
            // Arrays don't need special cleanup
            break;
            
        case EZOM_TYPE_CLASS:
            // Classes might be in global class table
            ezom_remove_from_class_table(object_ptr);
            break;
    }
}

// Example finalizer for file handles
void ezom_file_finalizer(uint24_t object_ptr) {
    ezom_file_object_t* file_obj = (ezom_file_object_t*)object_ptr;
    if (file_obj->file_handle) {
        fclose(file_obj->file_handle);
        file_obj->file_handle = NULL;
    }
}
```

### 3.7 GC Triggering and Tuning

#### 3.7.1 Automatic GC Triggering

```c
// Enhanced allocation with GC triggering
uint24_t ezom_allocate_with_gc(uint16_t size) {
    // Check if we need garbage collection
    if (ezom_should_trigger_gc(size)) {
        ezom_collect_garbage();
    }
    
    // Try allocation
    uint24_t ptr = ezom_freelist_allocate(size);
    
    if (!ptr) {
        // Emergency garbage collection
        printf("Emergency GC triggered\n");
        ezom_collect_garbage();
        
        ptr = ezom_freelist_allocate(size);
        
        if (!ptr) {
            // Still out of memory
            ezom_out_of_memory_error(size);
            return 0;
        }
    }
    
    // Update allocation tracking
    g_heap_manager.bytes_since_gc += size;
    g_heap_manager.objects_alive++;
    
    return ptr;
}

bool ezom_should_trigger_gc(uint16_t requested_size) {
    // Trigger GC if:
    // 1. We've allocated more than threshold since last GC
    if (g_heap_manager.bytes_since_gc > g_heap_manager.gc_threshold) {
        return true;
    }
    
    // 2. Available free memory is getting low
    uint32_t free_memory = ezom_calculate_free_memory();
    if (free_memory < (requested_size * 4)) {
        return true;
    }
    
    // 3. Free lists are getting fragmented
    if (ezom_is_heavily_fragmented()) {
        return true;
    }
    
    return false;
}

// Adaptive GC threshold tuning
void ezom_tune_gc_threshold(void) {
    static uint16_t last_gc_freed = 0;
    static uint16_t gc_frequency = 0;
    
    gc_frequency++;
    
    // If last GC freed very little, increase threshold
    if (last_gc_freed < (g_heap_manager.gc_threshold / 8)) {
        g_heap_manager.gc_threshold = min(g_heap_manager.gc_threshold * 2, 16384);
    }
    // If GC is very frequent, decrease threshold
    else if (gc_frequency > 10) {
        g_heap_manager.gc_threshold = max(g_heap_manager.gc_threshold / 2, 1024);
        gc_frequency = 0;
    }
}
```

### 3.8 Weak References

#### 3.8.1 Weak Reference Implementation

```c
// ezom_weak.h
typedef struct ezom_weak_ref {
    ezom_object_t header;
    uint24_t target;        // Object being weakly referenced
    uint24_t callback;      // Optional callback when target is collected
} ezom_weak_ref_t;

// Weak reference table for efficient cleanup
typedef struct ezom_weak_table {
    uint24_t* refs;         // Array of weak reference objects
    uint16_t capacity;
    uint16_t count;
} ezom_weak_table_t;

extern ezom_weak_table_t g_weak_table;

// Create weak reference
uint24_t ezom_create_weak_ref(uint24_t target, uint24_t callback);

// Clear weak references to collected objects
void ezom_clear_weak_references(void);
```

```c
// ezom_weak.c

uint24_t ezom_create_weak_ref(uint24_t target, uint24_t callback) {
    uint24_t ptr = ezom_allocate_with_gc(sizeof(ezom_weak_ref_t));
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_weak_ref_class, EZOM_TYPE_OBJECT);
    
    ezom_weak_ref_t* weak_ref = (ezom_weak_ref_t*)ptr;
    weak_ref->target = target;
    weak_ref->callback = callback;
    
    // Add to weak reference table
    ezom_weak_table_add(ptr);
    
    return ptr;
}

// Called during sweep phase to clear weak references
void ezom_clear_weak_references(void) {
    for (uint16_t i = 0; i < g_weak_table.count; i++) {
        uint24_t weak_ref_ptr = g_weak_table.refs[i];
        ezom_weak_ref_t* weak_ref = (ezom_weak_ref_t*)weak_ref_ptr;
        
        if (weak_ref->target) {
            ezom_object_t* target = (ezom_object_t*)weak_ref->target;
            
            // Check if target was collected (not marked)
            if (!(target->flags & EZOM_FLAG_MARKED)) {
                // Call callback if present
                if (weak_ref->callback) {
                    ezom_call_weak_callback(weak_ref->callback, weak_ref->target);
                }
                
                // Clear the weak reference
                weak_ref->target = 0;
            }
        }
    }
}
```

### 3.9 Memory Debugging and Profiling

#### 3.9.1 Memory Debugging Support

```c
// ezom_debug.h
#ifdef EZOM_DEBUG_MEMORY

typedef struct ezom_alloc_info {
    uint24_t address;
    uint16_t size;
    uint16_t line;
    char*    file;
    uint32_t timestamp;
} ezom_alloc_info_t;

typedef struct ezom_debug_heap {
    ezom_alloc_info_t allocations[512];
    uint16_t count;
    uint32_t total_allocated;
    uint32_t peak_usage;
} ezom_debug_heap_t;

extern ezom_debug_heap_t g_debug_heap;

#define ezom_debug_allocate(size) \
    ezom_debug_allocate_impl(size, __FILE__, __LINE__)

uint24_t ezom_debug_allocate_impl(uint16_t size, const char* file, uint16_t line);
void ezom_debug_free_impl(uint24_t ptr, const char* file, uint16_t line);
void ezom_debug_print_leaks(void);
void ezom_debug_print_heap_stats(void);

#else
#define ezom_debug_allocate(size) ezom_allocate_with_gc(size)
#endif
```

```c
// ezom_debug.c
#ifdef EZOM_DEBUG_MEMORY

uint24_t ezom_debug_allocate_impl(uint16_t size, const char* file, uint16_t line) {
    uint24_t ptr = ezom_allocate_with_gc(size);
    
    if (ptr && g_debug_heap.count < 512) {
        ezom_alloc_info_t* info = &g_debug_heap.allocations[g_debug_heap.count];
        info->address = ptr;
        info->size = size;
        info->file = file;
        info->line = line;
        info->timestamp = ezom_get_timestamp();
        
        g_debug_heap.count++;
        g_debug_heap.total_allocated += size;
        
        if (g_debug_heap.total_allocated > g_debug_heap.peak_usage) {
            g_debug_heap.peak_usage = g_debug_heap.total_allocated;
        }
    }
    
    return ptr;
}

void ezom_debug_print_heap_stats(void) {
    printf("Memory Debug Statistics:\n");
    printf("  Active allocations: %d\n", g_debug_heap.count);
    printf("  Total allocated: %ld bytes\n", g_debug_heap.total_allocated);
    printf("  Peak usage: %ld bytes\n", g_debug_heap.peak_usage);
    printf("  Objects alive: %d\n", g_heap_manager.objects_alive);
    printf("  GC cycles: %d\n", g_heap_manager.gc_cycles);
    
    // Free list statistics
    uint16_t total_free = 0;
    for (uint8_t i = 0; i < EZOM_SIZE_CLASSES; i++) {
        total_free += g_heap_manager.free_counts[i] * ezom_class_to_size(i);
    }
    printf("  Free memory: %d bytes\n", total_free);
}

void ezom_debug_print_leaks(void) {
    printf("Memory Leaks Detected:\n");
    
    uint16_t leak_count = 0;
    uint32_t leaked_bytes = 0;
    
    for (uint16_t i = 0; i < g_debug_heap.count; i++) {
        ezom_alloc_info_t* info = &g_debug_heap.allocations[i];
        
        if (info->address && ezom_is_valid_object(info->address)) {
            printf("  Leak: %d bytes at 0x%06X (%s:%d)\n", 
                   info->size, info->address, info->file, info->line);
            leak_count++;
            leaked_bytes += info->size;
        }
    }
    
    if (leak_count == 0) {
        printf("  No leaks detected!\n");
    } else {
        printf("  Total: %d leaks, %ld bytes\n", leak_count, leaked_bytes);
    }
}

#endif
```

### 3.10 Phase 3 Testing

#### 3.10.1 GC Stress Tests

```c
// test_phase3.c

bool test_basic_gc_cycle() {
    uint16_t initial_objects = g_heap_manager.objects_alive;
    
    // Create many temporary objects
    for (uint16_t i = 0; i < 100; i++) {
        uint24_t temp = ezom_create_integer(i);
        // Don't keep references - these should be collected
    }
    
    // Force garbage collection
    ezom_collect_garbage();
    
    // Should have same number of live objects as before
    return g_heap_manager.objects_alive == initial_objects;
}

bool test_gc_preserves_roots() {
    // Create objects and add to roots
    uint24_t obj1 = ezom_create_string("test1", 5);
    uint24_t obj2 = ezom_create_string("test2", 5);
    
    ezom_gc_add_root(obj1);
    ezom_gc_add_root(obj2);
    
    // Create garbage objects
    for (uint16_t i = 0; i < 50; i++) {
        ezom_create_integer(i);
    }
    
    // Force GC
    ezom_collect_garbage();
    
    // Root objects should still be valid
    bool result = ezom_is_valid_object(obj1) && ezom_is_valid_object(obj2);
    
    ezom_gc_remove_root(obj1);
    ezom_gc_remove_root(obj2);
    
    return result;
}

bool test_gc_handles_cycles() {
    // Create circular references
    uint24_t array1 = ezom_create_array(1);
    uint24_t array2 = ezom_create_array(1);
    
    // Make them reference each other
    ezom_array_set_element(array1, 0, array2);
    ezom_array_set_element(array2, 0, array1);
    
    // Remove our references
    array1 = 0;
    array2 = 0;
    
    uint16_t objects_before = g_heap_manager.objects_alive;
    
    // Force GC - should collect both arrays
    ezom_collect_garbage();
    
    uint16_t objects_after = g_heap_manager.objects_alive;
    
    // Should have collected the circular structures
    return objects_after < objects_before;
}

bool test_large_object_gc() {
    uint16_t initial_large_objects = g_heap_manager.large_object_count;
    
    // Create large objects
    for (uint8_t i = 0; i < 10; i++) {
        uint24_t large_str = ezom_create_string_of_size(1000);
        // Don't keep references
    }
    
    // Force GC
    ezom_collect_garbage();
    
    // Should be back to initial count
    return g_heap_manager.large_object_count == initial_large_objects;
}

bool test_weak_references() {
    uint24_t target = ezom_create_string("target", 6);
    uint24_t weak_ref = ezom_create_weak_ref(target, 0);
    
    // Clear strong reference
    target = 0;
    
    // Force GC
    ezom_collect_garbage();
    
    // Weak reference should be cleared
    ezom_weak_ref_t* weak_obj = (ezom_weak_ref_t*)weak_ref;
    return weak_obj->target == 0;
}

bool test_finalizers() {
    static bool finalizer_called = false;
    
    // Custom finalizer
    void test_finalizer(uint24_t object) {
        finalizer_called = true;
    }
    
    // Create object with finalizer
    uint24_t obj = ezom_create_string("test", 4);
    ezom_register_finalizer(obj, test_finalizer);
    
    // Clear reference
    obj = 0;
    
    // Force GC
    ezom_collect_garbage();
    
    return finalizer_called;
}

bool test_memory_pressure() {
    uint16_t successful_allocations = 0;
    
    // Allocate until we get close to memory limit
    for (uint16_t i = 0; i < 1000; i++) {
        uint24_t obj = ezom_create_array(100);
        if (obj) {
            successful_allocations++;
            ezom_gc_add_root(obj); // Keep it alive
        } else {
            break;
        }
    }
    
    // Should have triggered GC multiple times
    bool gc_triggered = g_heap_manager.gc_cycles > 0;
    
    // Clean up roots
    ezom_gc_clear_roots();
    ezom_collect_garbage();
    
    return gc_triggered && successful_allocations > 10;
}

bool test_fragmentation_handling() {
    // Create pattern that causes fragmentation
    uint24_t small_objects[50];
    uint24_t large_objects[10];
    
    // Allocate alternating small and large objects
    for (uint8_t i = 0; i < 50; i++) {
        small_objects[i] = ezom_create_integer(i);
        if (i < 10) {
            large_objects[i] = ezom_create_string_of_size(200);
        }
    }
    
    // Free every other small object
    for (uint8_t i = 0; i < 50; i += 2) {
        ezom_freelist_add_block(small_objects[i], sizeof(ezom_integer_t));
    }
    
    // Force compaction
    ezom_gc_compact_free_lists();
    
    // Should be able to allocate efficiently now
    uint24_t new_obj = ezom_create_integer(999);
    
    return new_obj != 0;
}

int main() {
    printf("EZOM Phase 3 Tests (Memory Management)\n");
    printf("=====================================\n");
    
    // Initialize VM
    ezom_init_vm();
    
    // Run GC tests
    TEST(basic_gc_cycle);
    TEST(gc_preserves_roots);
    TEST(gc_handles_cycles);
    TEST(large_object_gc);
    TEST(weak_references);
    TEST(finalizers);
    TEST(memory_pressure);
    TEST(fragmentation_handling);
    
    // Memory debugging tests
    #ifdef EZOM_DEBUG_MEMORY
    TEST(memory_leak_detection);
    TEST(allocation_tracking);
    #endif
    
    printf("\nFinal Memory Statistics:\n");
    ezom_debug_print_heap_stats();
    
    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_total);
    return tests_passed == tests_total ? 0 : 1;
}
```

### 3.11 Performance Benchmarks

#### 3.11.1 GC Performance Tests

```c
// benchmark_gc.c

void benchmark_allocation_speed() {
    uint32_t start_time = ezom_get_timestamp();
    
    // Allocate 1000 objects
    for (uint16_t i = 0; i < 1000; i++) {
        uint24_t obj = ezom_create_integer(i);
        ezom_gc_add_root(obj); // Keep alive to avoid GC during test
    }
    
    uint32_t end_time = ezom_get_timestamp();
    
    printf("Allocation speed: %ld objects/second\n", 
           1000000UL / (end_time - start_time));
    
    ezom_gc_clear_roots();
}

void benchmark_gc_pause_time() {
    // Fill heap to ~80% capacity
    uint16_t objects_to_create = (EZOM_HEAP_SIZE * 8) / (10 * sizeof(ezom_integer_t));
    
    for (uint16_t i = 0; i < objects_to_create; i++) {
        ezom_create_integer(i);
    }
    
    // Measure GC time
    uint32_t start_time = ezom_get_timestamp();
    ezom_collect_garbage();
    uint32_t end_time = ezom_get_timestamp();
    
    printf("GC pause time: %ld microseconds\n", end_time - start_time);
    printf("GC throughput: %ld objects/second\n", 
           (objects_to_create * 1000000UL) / (end_time - start_time));
}

void benchmark_fragmentation_overhead() {
    // Measure allocation time with high fragmentation
    ezom_create_fragmentation_pattern();
    
    uint32_t start_time = ezom_get_timestamp();
    
    for (uint16_t i = 0; i < 100; i++) {
        uint24_t obj = ezom_create_integer(i);
        if (!obj) break;
    }
    
    uint32_t fragmented_time = ezom_get_timestamp() - start_time;
    
    // Clean up and measure with low fragmentation
    ezom_collect_garbage();
    ezom_gc_compact_free_lists();
    
    start_time = ezom_get_timestamp();
    
    for (uint16_t i = 0; i < 100; i++) {
        uint24_t obj = ezom_create_integer(i);
        if (!obj) break;
    }
    
    uint32_t clean_time = ezom_get_timestamp() - start_time;
    
    printf("Fragmentation overhead: %ld%% slower\n", 
           ((fragmented_time - clean_time) * 100) / clean_time);
}
```

### 3.12 Integration with Previous Phases

#### 3.12.1 Updated Object Creation

```c
// Updated object creation functions to use GC allocation

uint24_t ezom_create_integer(int16_t value) {
    uint24_t ptr = ezom_allocate_with_gc(sizeof(ezom_integer_t));
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_integer_class, EZOM_TYPE_INTEGER);
    
    ezom_integer_t* obj = (ezom_integer_t*)ptr;
    obj->value = value;
    
    return ptr;
}

uint24_t ezom_create_string(const char* data, uint16_t length) {
    uint16_t total_size = sizeof(ezom_string_t) + length + 1;
    uint24_t ptr = ezom_allocate_with_gc(total_size);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_string_class, EZOM_TYPE_STRING);
    
    ezom_string_t* obj = (ezom_string_t*)ptr;
    obj->length = length;
    memcpy(obj->data, data, length);
    obj->data[length] = '\0';
    
    return ptr;
}

uint24_t ezom_create_array(uint16_t size) {
    uint16_t total_size = sizeof(ezom_array_t) + (size * sizeof(uint24_t));
    uint24_t ptr = ezom_allocate_with_gc(total_size);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_array_class, EZOM_TYPE_ARRAY);
    
    ezom_array_t* obj = (ezom_array_t*)ptr;
    obj->size = size;
    
    // Initialize elements to nil
    for (uint16_t i = 0; i < size; i++) {
        obj->elements[i] = g_nil;
    }
    
    return ptr;
}
```

#### 3.12.2 Context Management Integration

```c
// Integrate GC with execution contexts from Phase 2

uint24_t ezom_create_context(uint24_t outer_context, uint8_t local_count) {
    uint16_t total_size = sizeof(ezom_context_t) + (local_count * sizeof(uint24_t));
    uint24_t ptr = ezom_allocate_with_gc(total_size);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_context_class, EZOM_TYPE_OBJECT);
    
    ezom_context_t* context = (ezom_context_t*)ptr;
    context->outer_context = outer_context;
    context->method = 0;
    context->receiver = 0;
    
    // Initialize locals to nil
    for (uint8_t i = 0; i < local_count; i++) {
        context->locals[i] = g_nil;
    }
    
    // Add to GC roots while active
    ezom_gc_add_root(ptr);
    
    return ptr;
}

void ezom_destroy_context(uint24_t context_ptr) {
    // Remove from GC roots
    ezom_gc_remove_root(context_ptr);
    
    // Object will be collected in next GC cycle
}
```

### 3.13 Phase 3 Deliverables

**Core Files:**
- `ezom_gc.c/h` - Mark-and-sweep garbage collector
- `ezom_freelist.c/h` - Free list allocator with size classes
- `ezom_finalize.c/h` - Object finalization support
- `ezom_weak.c/h` - Weak reference implementation
- `ezom_debug.c/h` - Memory debugging and profiling
- `test_phase3.c` - Comprehensive GC test suite
- `benchmark_gc.c` - Performance benchmarking

**Enhanced Files:**
- `ezom_memory.c/h` - Updated with GC integration
- `ezom_object.c/h` - Updated object creation functions
- `ezom_primitives.c/h` - GC-aware primitive operations

**Test Criteria:**
- [ ] Automatic garbage collection with configurable thresholds
- [ ] Proper handling of object references and cycles
- [ ] Root set management and weak references
- [ ] Object finalization and cleanup
- [ ] Memory compaction and fragmentation reduction
- [ ] GC pause times under 10ms for typical workloads
- [ ] Memory debugging and leak detection
- [ ] Performance benchmarks within target specifications

### 3.14 Success Metrics

**Memory Management:**
- Automatic GC with < 10ms pause times
- Memory utilization > 80% before collection
- Fragmentation < 20% after compaction
- Zero memory leaks in test suite

**Performance Targets:**
- Object allocation: < 50 cycles per object
- GC throughput: > 1000 objects/second
- Memory overhead: < 10% for GC metadata
- Startup time: < 3 seconds including GC initialization

**Integration:**
- All Phase 1-2 tests continue to pass
- Long-running programs execute without memory issues
- Complex object hierarchies handled correctly
- Finalizers work reliably for resource cleanup

This completes the Phase 3 specification, providing a production-quality memory management system that will support the full EZOM language implementation while maintaining the performance characteristics needed for the ez80 Agon Light 2 platform.
