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
#include <stdbool.h>

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
    
    // Phase 3: Initialize enhanced statistics
    g_heap.total_allocations = 0;
    g_heap.allocation_failures = 0;
    g_heap.peak_bytes_used = 0;
    g_heap.bytes_since_last_gc = 0;
    
    g_heap.integer_objects = 0;
    g_heap.string_objects = 0;
    g_heap.array_objects = 0;
    g_heap.block_objects = 0;
    g_heap.other_objects = 0;
    
    g_heap.largest_free_block = EZOM_HEAP_SIZE;
    g_heap.free_block_count = 1;
    
    g_heap.gc_threshold = EZOM_HEAP_SIZE / 4; // Trigger GC at 25% capacity
    g_heap.gc_enabled = false; // Disabled until GC is implemented
    
    // Phase 3: Initialize free list allocator
    ezom_init_free_lists();
    
    // Phase 3 Step 3: Initialize marking system
    ezom_init_marking_system();
    
    // Phase 3 Step 4: Initialize garbage collector
    ezom_init_garbage_collector();
    
    printf("EZOM: Enhanced memory tracking initialized\n");
}

uint24_t ezom_allocate(uint16_t size) {
    // Align to 2-byte boundary
    size = (size + 1) & ~1;
    
    // Phase 3: Enhanced allocation tracking
    g_heap.total_allocations++;
    
    // Check if we have space
    if (g_heap.next_free + size > g_heap.heap_end) {
        printf("EZOM: Out of memory! Requested %d bytes\n", size);
        g_heap.allocation_failures++;
        return 0;
    }
    
    uint24_t ptr = g_heap.next_free;
    g_heap.next_free += size;
    g_heap.objects_allocated++;
    g_heap.bytes_allocated += size;
    g_heap.bytes_since_last_gc += size;
    
    // Update peak usage
    if (g_heap.bytes_allocated > g_heap.peak_bytes_used) {
        g_heap.peak_bytes_used = g_heap.bytes_allocated;
    }
    
    // Update fragmentation tracking (simplified for bump allocator)
    g_heap.largest_free_block = g_heap.heap_end - g_heap.next_free;
    
    // Clear allocated memory
#ifdef EZOM_PLATFORM_NATIVE
    memset(ezom_ptr_to_native(ptr), 0, size);
#else
    memset((void*)ptr, 0, size);
#endif
    
    return ptr;
}

// Phase 3: Enhanced allocate with object type tracking
uint24_t ezom_allocate_typed(uint16_t size, uint8_t object_type) {
    // Check if GC should be triggered before allocation
    if (ezom_should_gc_now()) {
        printf("EZOM: Auto-triggering GC before allocation\n");
        ezom_trigger_garbage_collection();
    }
    
    uint24_t ptr;
    
    // Use free list allocator if enabled
    if (g_heap.use_free_lists) {
        ptr = ezom_freelist_allocate(size);
    } else {
        ptr = ezom_allocate(size);
    }
    
    if (ptr) {
        ezom_track_allocation(object_type, size);
    }
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

// ============================================================================
// PHASE 3: ENHANCED MEMORY MANAGEMENT
// ============================================================================

// Track allocation by object type
void ezom_track_allocation(uint8_t object_type, uint16_t size) {
    switch (object_type) {
        case EZOM_TYPE_INTEGER:
            g_heap.integer_objects++;
            break;
        case EZOM_TYPE_STRING:
            g_heap.string_objects++;
            break;
        case EZOM_TYPE_ARRAY:
            g_heap.array_objects++;
            break;
        case EZOM_TYPE_BLOCK:
            g_heap.block_objects++;
            break;
        default:
            g_heap.other_objects++;
            break;
    }
}

// Detailed memory statistics
void ezom_detailed_memory_stats(void) {
    uint16_t used = g_heap.bytes_allocated;
    uint32_t total = EZOM_HEAP_SIZE;
    uint16_t available = g_heap.largest_free_block;
    
    printf("\n=== EZOM Memory Statistics ===\n");
    printf("Heap Usage: %d/%d bytes used (%.1f%%)\n",
           used, (int)total, (used * 100.0) / total);
    printf("Peak Usage: %d bytes (%.1f%%)\n",
           g_heap.peak_bytes_used, (g_heap.peak_bytes_used * 100.0) / total);
    printf("Available: %d bytes (largest block)\n", available);
    printf("Total Allocations: %d\n", g_heap.total_allocations);
    printf("Allocation Failures: %d\n", g_heap.allocation_failures);
    printf("Objects Alive: %d\n", g_heap.objects_allocated);
    
    printf("\nObject Type Breakdown:\n");
    printf("  Integers: %d\n", g_heap.integer_objects);
    printf("  Strings:  %d\n", g_heap.string_objects);
    printf("  Arrays:   %d\n", g_heap.array_objects);
    printf("  Blocks:   %d\n", g_heap.block_objects);
    printf("  Other:    %d\n", g_heap.other_objects);
    
    printf("\nGC Status:\n");
    printf("  GC Enabled: %s\n", g_heap.gc_enabled ? "Yes" : "No");
    printf("  GC Threshold: %d bytes\n", g_heap.gc_threshold);
    printf("  Bytes since last GC: %d\n", g_heap.bytes_since_last_gc);
    printf("  Should trigger GC: %s\n", ezom_should_trigger_gc() ? "Yes" : "No");
    printf("==============================\n\n");
}

// Memory fragmentation report
void ezom_memory_fragmentation_report(void) {
    uint16_t used = g_heap.bytes_allocated;
    uint16_t available = g_heap.largest_free_block;
    uint16_t total_free = EZOM_HEAP_SIZE - used;
    
    printf("=== Memory Fragmentation Report ===\n");
    printf("Total free space: %d bytes\n", total_free);
    printf("Largest free block: %d bytes\n", available);
    
    if (total_free > 0) {
        float fragmentation = 1.0f - ((float)available / total_free);
        printf("Fragmentation: %.1f%%\n", fragmentation * 100);
        
        if (fragmentation > 0.5f) {
            printf("WARNING: High fragmentation detected!\n");
        }
    }
    
    printf("Free block count: %d\n", g_heap.free_block_count);
    printf("==================================\n");
}

// Get memory pressure (0-100, higher means more pressure)
uint16_t ezom_get_memory_pressure(void) {
    uint16_t used = g_heap.bytes_allocated;
    uint32_t total = EZOM_HEAP_SIZE;
    return (used * 100) / total;
}

// Set GC threshold
void ezom_set_gc_threshold(uint16_t threshold) {
    g_heap.gc_threshold = threshold;
    printf("EZOM: GC threshold set to %d bytes\n", threshold);
}

// Check if GC should be triggered
bool ezom_should_trigger_gc(void) {
    if (!g_heap.gc_enabled) return false;
    
    // Trigger GC if we've allocated too much since last GC
    if (g_heap.bytes_since_last_gc >= g_heap.gc_threshold) return true;
    
    // Trigger GC if memory pressure is high
    if (ezom_get_memory_pressure() >= 80) return true;
    
    // Trigger GC if fragmentation is high (simplified check)
    uint16_t available = g_heap.largest_free_block;
    uint16_t total_free = EZOM_HEAP_SIZE - g_heap.bytes_allocated;
    if (total_free > 0 && available < (total_free / 2)) return true;
    
    return false;
}

// ============================================================================
// PHASE 3: FREE LIST ALLOCATOR IMPLEMENTATION
// ============================================================================

// Map size to size class index
uint8_t ezom_size_to_class(uint16_t size) {
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
    if (size <= 384) return 10;    // 384 bytes
    if (size <= 512) return 11;    // 512 bytes
    if (size <= 768) return 12;    // 768 bytes
    if (size <= 1024) return 13;   // 1024 bytes
    if (size <= 1536) return 14;   // 1536 bytes
    return 15;                     // 2048+ bytes (largest class)
}

// Get actual size for size class
uint16_t ezom_class_to_size(uint8_t class_index) {
    static const uint16_t sizes[] = {
        8, 16, 24, 32, 48, 64, 96, 128, 192, 256,
        384, 512, 768, 1024, 1536, 2048
    };
    
    if (class_index >= EZOM_SIZE_CLASSES) {
        return 2048;  // Maximum size
    }
    
    return sizes[class_index];
}

// Initialize free lists
void ezom_init_free_lists(void) {
    // Clear all free lists
    for (int i = 0; i < EZOM_SIZE_CLASSES; i++) {
        g_heap.free_lists[i] = 0;
        g_heap.free_counts[i] = 0;
    }
    
    g_heap.large_object_list = 0;
    g_heap.large_object_count = 0;
    g_heap.use_free_lists = false;  // Start disabled
    
    printf("EZOM: Free list allocator initialized\n");
}

// Enable/disable free list allocator
void ezom_enable_free_lists(bool enable) {
    g_heap.use_free_lists = enable;
    printf("EZOM: Free list allocator %s\n", enable ? "enabled" : "disabled");
}

// Allocate using free list system
uint24_t ezom_freelist_allocate(uint16_t requested_size) {
    // Align size to 2-byte boundary
    uint16_t size = (requested_size + 1) & ~1;
    
    // For large objects, use bump allocator for now
    if (size > EZOM_LARGE_OBJECT_THRESHOLD) {
        printf("EZOM: Large object allocation (%d bytes) - using bump allocator\n", size);
        return ezom_allocate(size);
    }
    
    uint8_t class_index = ezom_size_to_class(size);
    uint16_t actual_size = ezom_class_to_size(class_index);
    
    // Check if we have a free block of this size class
    uint24_t block_ptr = g_heap.free_lists[class_index];
    
    if (block_ptr) {
        // Remove from free list
        ezom_free_block_t* block = (ezom_free_block_t*)ezom_ptr_to_native(block_ptr);
        g_heap.free_lists[class_index] = (uint24_t)block->next;
        g_heap.free_counts[class_index]--;
        
        // Clear the block and return
        memset(block, 0, actual_size);
        
        printf("EZOM: Reused free block 0x%06X (class %d, %d bytes)\n", 
               block_ptr, class_index, actual_size);
        
        return block_ptr;
    }
    
    // No free block available, allocate new memory
    // Make sure we allocate the full size class size for future reuse
    uint24_t new_ptr = ezom_allocate(actual_size);
    
    if (new_ptr) {
        printf("EZOM: New allocation 0x%06X (class %d, %d bytes)\n", 
               new_ptr, class_index, actual_size);
    }
    
    return new_ptr;
}

// Deallocate to free list
void ezom_freelist_deallocate(uint24_t ptr, uint16_t size) {
    if (!ptr || !g_heap.use_free_lists) {
        return;  // Free lists disabled or invalid pointer
    }
    
    // Align size to match what was allocated
    uint16_t aligned_size = (size + 1) & ~1;
    
    // Large objects not supported yet
    if (aligned_size > EZOM_LARGE_OBJECT_THRESHOLD) {
        printf("EZOM: Large object deallocation not implemented\n");
        return;
    }
    
    uint8_t class_index = ezom_size_to_class(aligned_size);
    
    // Add to free list
    ezom_free_block_t* block = (ezom_free_block_t*)ezom_ptr_to_native(ptr);
    block->next = (ezom_free_block_t*)g_heap.free_lists[class_index];
    block->size = ezom_class_to_size(class_index);
    
    g_heap.free_lists[class_index] = ptr;
    g_heap.free_counts[class_index]++;
    
    printf("EZOM: Deallocated to free list 0x%06X (class %d, %d bytes)\n", 
           ptr, class_index, block->size);
}

// Print free list statistics
void ezom_free_list_stats(void) {
    printf("\n=== Free List Statistics ===\n");
    printf("Free list allocator: %s\n", g_heap.use_free_lists ? "Enabled" : "Disabled");
    
    uint16_t total_free_blocks = 0;
    uint32_t total_free_bytes = 0;
    
    for (int i = 0; i < EZOM_SIZE_CLASSES; i++) {
        if (g_heap.free_counts[i] > 0) {
            uint16_t class_size = ezom_class_to_size(i);
            uint32_t class_bytes = g_heap.free_counts[i] * class_size;
            
            printf("  Class %2d (%4d bytes): %3d blocks (%d bytes)\n", 
                   i, class_size, g_heap.free_counts[i], class_bytes);
            
            total_free_blocks += g_heap.free_counts[i];
            total_free_bytes += class_bytes;
        }
    }
    
    printf("Total free blocks: %d (%d bytes)\n", total_free_blocks, total_free_bytes);
    printf("Large objects: %d\n", g_heap.large_object_count);
    printf("============================\n\n");
}

// ============================================================================
// PHASE 3 STEP 3: OBJECT MARKING SYSTEM
// ============================================================================

// Global GC roots
ezom_gc_roots_t g_gc_roots;

// Initialize the marking system
void ezom_init_marking_system(void) {
    g_gc_roots.count = 0;
    g_gc_roots.gc_in_progress = false;
    memset(g_gc_roots.roots, 0, sizeof(g_gc_roots.roots));
    printf("EZOM: Object marking system initialized\n");
}

// Check if an object is marked
bool ezom_is_marked(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) {
        return false;
    }
    
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    return (object->flags & EZOM_FLAG_MARKED) != 0;
}

// Mark an object as reachable
void ezom_mark_object(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) {
        return;
    }
    
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    
    // Skip if already marked (prevents infinite recursion)
    if (object->flags & EZOM_FLAG_MARKED) {
        return;
    }
    
    // Set the mark bit
    object->flags |= EZOM_FLAG_MARKED;
    
    // Recursively mark all references from this object
    ezom_mark_object_references(obj);
}

// Unmark an object
void ezom_unmark_object(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) {
        return;
    }
    
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    object->flags &= ~EZOM_FLAG_MARKED;
}

// Clear all mark bits in the heap
void ezom_clear_all_marks(void) {
    printf("EZOM: Clearing all object marks...\n");
    
    uint24_t current = EZOM_HEAP_START;
    uint16_t cleared = 0;
    
    while (current < g_heap.next_free) {
        if (ezom_is_valid_object(current)) {
            ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(current);
            if (obj->flags & EZOM_FLAG_MARKED) {
                obj->flags &= ~EZOM_FLAG_MARKED;
                cleared++;
            }
            
            // Move to next object (estimate size)
            uint16_t obj_size = 16; // Default estimate
            uint8_t type = obj->flags & 0xF0;
            
            switch (type) {
                case EZOM_TYPE_INTEGER:
                    obj_size = sizeof(ezom_object_t) + sizeof(int16_t);
                    break;
                case EZOM_TYPE_STRING:
                case EZOM_TYPE_SYMBOL: {
                    ezom_string_t* str = (ezom_string_t*)obj;
                    obj_size = sizeof(ezom_string_t) + str->length + 1;
                    break;
                }
                case EZOM_TYPE_ARRAY: {
                    ezom_array_t* arr = (ezom_array_t*)obj;
                    obj_size = sizeof(ezom_array_t) + (arr->size * sizeof(uint24_t));
                    break;
                }
                default:
                    obj_size = 16; // Safe default
                    break;
            }
            
            current += (obj_size + 1) & ~1; // Align to even boundary
        } else {
            current += 2; // Skip invalid objects
        }
    }
    
    printf("EZOM: Cleared marks on %d objects\n", cleared);
}

// Mark all references from an object
void ezom_mark_object_references(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) {
        return;
    }
    
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    uint8_t type = object->flags & 0xF0;
    
    // Mark the class pointer
    if (object->class_ptr) {
        ezom_mark_object(object->class_ptr);
    }
    
    // Mark type-specific references
    switch (type) {
        case EZOM_TYPE_ARRAY: {
            ezom_array_t* array = (ezom_array_t*)EZOM_OBJECT_PTR(obj);
            
            // Mark all array elements
            for (uint16_t i = 0; i < array->size; i++) {
                if (array->elements[i]) {
                    ezom_mark_object(array->elements[i]);
                }
            }
            break;
        }
        
        case EZOM_TYPE_CLASS: {
            ezom_class_t* class_obj = (ezom_class_t*)EZOM_OBJECT_PTR(obj);
            
            // Mark superclass
            if (class_obj->superclass) {
                ezom_mark_object(class_obj->superclass);
            }
            
            // Mark method dictionary
            if (class_obj->method_dict) {
                ezom_mark_object(class_obj->method_dict);
            }
            break;
        }
        
        case EZOM_TYPE_BLOCK: {
            ezom_block_t* block = (ezom_block_t*)EZOM_OBJECT_PTR(obj);
            
            // Mark outer context (if any)
            if (block->outer_context) {
                ezom_mark_object(block->outer_context);
            }
            
            // Mark code pointer (if any)
            if (block->code) {
                ezom_mark_object(block->code);
            }
            
            // Note: captured_vars array would need more complex handling
            // For now, we'll skip it as it's a variable-length array
            break;
        }
        
        // Primitive types (integer, string, symbol) have no references
        case EZOM_TYPE_INTEGER:
        case EZOM_TYPE_STRING:
        case EZOM_TYPE_SYMBOL:
        case EZOM_TYPE_BOOLEAN:
        case EZOM_TYPE_NIL:
            // No references to mark
            break;
        
        default:
            // Unknown type - be conservative and don't mark anything
            break;
    }
}

// Mark from all GC roots
void ezom_mark_from_roots(void) {
    printf("EZOM: Marking from %d GC roots...\n", g_gc_roots.count);
    
    for (uint8_t i = 0; i < g_gc_roots.count; i++) {
        uint24_t root = g_gc_roots.roots[i];
        if (root) {
            printf("  Marking from root %d: 0x%06X\n", i, root);
            ezom_mark_object(root);
        }
    }
}

// Count marked objects
uint16_t ezom_count_marked_objects(void) {
    uint24_t current = EZOM_HEAP_START;
    uint16_t marked_count = 0;
    
    while (current < g_heap.next_free) {
        if (ezom_is_valid_object(current) && ezom_is_marked(current)) {
            marked_count++;
        }
        current += 2; // Move by minimum alignment
    }
    
    return marked_count;
}

// Count unmarked objects
uint16_t ezom_count_unmarked_objects(void) {
    uint24_t current = EZOM_HEAP_START;
    uint16_t unmarked_count = 0;
    
    while (current < g_heap.next_free) {
        if (ezom_is_valid_object(current) && !ezom_is_marked(current)) {
            unmarked_count++;
        }
        current += 2; // Move by minimum alignment
    }
    
    return unmarked_count;
}

// Add an object to the GC root set
void ezom_add_gc_root(uint24_t obj) {
    if (!obj || g_gc_roots.count >= EZOM_MAX_GC_ROOTS) {
        return;
    }
    
    // Check if already in root set
    for (uint8_t i = 0; i < g_gc_roots.count; i++) {
        if (g_gc_roots.roots[i] == obj) {
            return; // Already a root
        }
    }
    
    g_gc_roots.roots[g_gc_roots.count] = obj;
    g_gc_roots.count++;
    
    printf("EZOM: Added GC root 0x%06X (total: %d)\n", obj, g_gc_roots.count);
}

// Remove an object from the GC root set
void ezom_remove_gc_root(uint24_t obj) {
    for (uint8_t i = 0; i < g_gc_roots.count; i++) {
        if (g_gc_roots.roots[i] == obj) {
            // Shift remaining roots down
            for (uint8_t j = i; j < g_gc_roots.count - 1; j++) {
                g_gc_roots.roots[j] = g_gc_roots.roots[j + 1];
            }
            g_gc_roots.count--;
            g_gc_roots.roots[g_gc_roots.count] = 0;
            
            printf("EZOM: Removed GC root 0x%06X (total: %d)\n", obj, g_gc_roots.count);
            return;
        }
    }
}

// Clear all GC roots
void ezom_clear_gc_roots(void) {
    memset(g_gc_roots.roots, 0, sizeof(g_gc_roots.roots));
    g_gc_roots.count = 0;
    printf("EZOM: Cleared all GC roots\n");
}

// List all GC roots
void ezom_list_gc_roots(void) {
    printf("\n=== GC Root Set ===\n");
    printf("Total roots: %d/%d\n", g_gc_roots.count, EZOM_MAX_GC_ROOTS);
    
    for (uint8_t i = 0; i < g_gc_roots.count; i++) {
        uint24_t root = g_gc_roots.roots[i];
        bool marked = ezom_is_marked(root);
        printf("  Root %d: 0x%06X %s\n", i, root, marked ? "(marked)" : "(unmarked)");
    }
    printf("===================\n\n");
}

// Execute the mark phase
void ezom_mark_phase(void) {
    if (g_gc_roots.gc_in_progress) {
        printf("EZOM: GC already in progress, skipping mark phase\n");
        return;
    }
    
    printf("EZOM: Starting mark phase...\n");
    g_gc_roots.gc_in_progress = true;
    
    // Step 1: Clear all marks
    ezom_clear_all_marks();
    
    // Step 2: Mark from all roots
    ezom_mark_from_roots();
    
    // Step 3: Count results
    uint16_t marked = ezom_count_marked_objects();
    uint16_t unmarked = ezom_count_unmarked_objects();
    
    printf("EZOM: Mark phase complete - %d marked, %d unmarked\n", marked, unmarked);
    
    g_gc_roots.gc_in_progress = false;
}

// Print marking statistics
void ezom_marking_stats(void) {
    uint16_t marked = ezom_count_marked_objects();
    uint16_t unmarked = ezom_count_unmarked_objects();
    uint16_t total = marked + unmarked;
    
    printf("\n=== Object Marking Statistics ===\n");
    printf("Total objects: %d\n", total);
    printf("Marked objects: %d (%.1f%%)\n", marked, total ? (marked * 100.0f / total) : 0.0f);
    printf("Unmarked objects: %d (%.1f%%)\n", unmarked, total ? (unmarked * 100.0f / total) : 0.0f);
    printf("GC roots: %d/%d\n", g_gc_roots.count, EZOM_MAX_GC_ROOTS);
    printf("GC in progress: %s\n", g_gc_roots.gc_in_progress ? "Yes" : "No");
    printf("==================================\n\n");
}

// Identify garbage objects (unmarked objects)
uint16_t ezom_identify_garbage(uint24_t* garbage_list, uint16_t max_objects) {
    uint24_t current = EZOM_HEAP_START;
    uint16_t garbage_count = 0;
    
    printf("EZOM: Identifying garbage objects...\n");
    
    while (current < g_heap.next_free && garbage_count < max_objects) {
        if (ezom_is_valid_object(current) && !ezom_is_marked(current)) {
            garbage_list[garbage_count] = current;
            garbage_count++;
            
            printf("  Found garbage: 0x%06X\n", current);
        }
        current += 2; // Move by minimum alignment
    }
    
    printf("EZOM: Found %d garbage objects\n", garbage_count);
    return garbage_count;
}

// Print sweep detection statistics
void ezom_sweep_detection_stats(void) {
    uint24_t garbage_list[256]; // Buffer for garbage objects
    uint16_t garbage_count = ezom_identify_garbage(garbage_list, 256);
    
    printf("\n=== Sweep Detection Statistics ===\n");
    printf("Garbage objects found: %d\n", garbage_count);
    
    if (garbage_count > 0) {
        printf("Garbage objects (first 10):\n");
        for (uint16_t i = 0; i < garbage_count && i < 10; i++) {
            printf("  0x%06X\n", garbage_list[i]);
        }
        if (garbage_count > 10) {
            printf("  ... and %d more\n", garbage_count - 10);
        }
    }
    printf("===================================\n");
}

// ============================================================================
// PHASE 3 STEP 4: GARBAGE COLLECTION
// ============================================================================

// Global GC statistics
ezom_gc_stats_t g_gc_stats;

// Initialize the garbage collector
void ezom_init_garbage_collector(void) {
    memset(&g_gc_stats, 0, sizeof(g_gc_stats));
    
    // Enable GC by default
    g_heap.gc_enabled = true;
    
    printf("EZOM: Garbage collector initialized\n");
}

// Trigger garbage collection based on thresholds
bool ezom_trigger_garbage_collection(void) {
    if (!g_heap.gc_enabled) {
        printf("EZOM: GC disabled, skipping collection\n");
        return false;
    }
    
    if (g_gc_roots.gc_in_progress) {
        printf("EZOM: GC already in progress\n");
        return false;
    }
    
    printf("EZOM: Triggering garbage collection...\n");
    g_gc_stats.collections_triggered++;
    
    return ezom_full_garbage_collection();
}

// Perform a full mark-and-sweep garbage collection
bool ezom_full_garbage_collection(void) {
    if (!g_heap.gc_enabled || g_gc_roots.gc_in_progress) {
        return false;
    }
    
    printf("EZOM: Starting full garbage collection cycle\n");
    g_gc_roots.gc_in_progress = true;
    
    // Record state before GC
    g_gc_stats.objects_before_gc = g_heap.objects_allocated;
    g_gc_stats.fragmentation_before_gc = ezom_calculate_fragmentation();
    
    uint16_t objects_before = g_heap.objects_allocated;
    uint16_t bytes_before = g_heap.bytes_allocated;
    
    // Phase 1: Mark all reachable objects
    printf("EZOM: GC Phase 1 - Marking reachable objects\n");
    ezom_mark_phase();
    
    // Phase 2: Sweep unreachable objects
    printf("EZOM: GC Phase 2 - Sweeping unreachable objects\n");
    uint16_t objects_collected = ezom_sweep_phase();
    
    // Phase 3: Compact free lists
    printf("EZOM: GC Phase 3 - Compacting memory\n");
    ezom_compact_free_lists();
    
    // Update statistics
    g_gc_stats.collections_performed++;
    g_gc_stats.objects_collected += objects_collected;
    g_gc_stats.bytes_collected += (bytes_before - g_heap.bytes_allocated);
    g_gc_stats.objects_after_gc = g_heap.objects_allocated;
    g_gc_stats.fragmentation_after_gc = ezom_calculate_fragmentation();
    
    // Reset GC trigger
    g_heap.bytes_since_last_gc = 0;
    
    printf("EZOM: GC cycle complete - collected %d objects, freed %d bytes\n",
           objects_collected, bytes_before - g_heap.bytes_allocated);
    
    g_gc_roots.gc_in_progress = false;
    return true;
}

// Sweep phase - reclaim memory from unmarked objects
uint16_t ezom_sweep_phase(void) {
    uint24_t current = EZOM_HEAP_START;
    uint16_t objects_swept = 0;
    uint16_t bytes_swept = 0;
    
    printf("EZOM: Starting sweep phase\n");
    
    while (current < g_heap.next_free) {
        if (ezom_is_valid_object(current)) {
            ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(current);
            
            if (!(obj->flags & EZOM_FLAG_MARKED)) {
                // Object is unmarked - it's garbage
                uint16_t obj_size = ezom_calculate_object_size(current);
                
                printf("  Sweeping garbage object 0x%06X (size: %d)\n", current, obj_size);
                
                // Add to free list if free list allocator is enabled
                if (g_heap.use_free_lists) {
                    ezom_freelist_deallocate(current, obj_size);
                }
                
                // Update statistics
                objects_swept++;
                bytes_swept += obj_size;
                
                // Update heap counters
                g_heap.objects_allocated--;
                g_heap.bytes_allocated -= obj_size;
                
                // Update object type counters
                uint8_t type = obj->flags & 0xF0;
                switch (type) {
                    case EZOM_TYPE_INTEGER:
                        g_heap.integer_objects--;
                        break;
                    case EZOM_TYPE_STRING:
                    case EZOM_TYPE_SYMBOL:
                        g_heap.string_objects--;
                        break;
                    case EZOM_TYPE_ARRAY:
                        g_heap.array_objects--;
                        break;
                    case EZOM_TYPE_BLOCK:
                        g_heap.block_objects--;
                        break;
                    default:
                        g_heap.other_objects--;
                        break;
                }
                
                // Zero out the object memory for debugging
                memset(obj, 0, obj_size);
                
                current += obj_size;
            } else {
                // Object is marked - keep it, but clear the mark for next GC
                obj->flags &= ~EZOM_FLAG_MARKED;
                
                uint16_t obj_size = ezom_calculate_object_size(current);
                current += obj_size;
            }
        } else {
            current += 2; // Skip invalid memory
        }
    }
    
    printf("EZOM: Sweep phase complete - swept %d objects (%d bytes)\n", 
           objects_swept, bytes_swept);
    
    return objects_swept;
}

// Calculate the size of an object
uint16_t ezom_calculate_object_size(uint24_t obj_ptr) {
    if (!ezom_is_valid_object(obj_ptr)) {
        return 2; // Minimum size
    }
    
    ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(obj_ptr);
    uint8_t type = obj->flags & 0xF0;
    
    switch (type) {
        case EZOM_TYPE_INTEGER:
            return sizeof(ezom_integer_t);
            
        case EZOM_TYPE_STRING:
        case EZOM_TYPE_SYMBOL: {
            ezom_string_t* str = (ezom_string_t*)obj;
            return sizeof(ezom_string_t) + str->length + 1;
        }
        
        case EZOM_TYPE_ARRAY: {
            ezom_array_t* arr = (ezom_array_t*)obj;
            return sizeof(ezom_array_t) + (arr->size * sizeof(uint24_t));
        }
        
        case EZOM_TYPE_BLOCK:
            return sizeof(ezom_block_t);
            
        case EZOM_TYPE_CLASS: {
            ezom_class_t* cls = (ezom_class_t*)obj;
            return sizeof(ezom_class_t) + (cls->instance_size * sizeof(uint24_t));
        }
        
        default:
            return sizeof(ezom_object_t); // Base object size
    }
}

// Compact free lists after GC
void ezom_compact_free_lists(void) {
    if (!g_heap.use_free_lists) {
        return;
    }
    
    printf("EZOM: Compacting free lists\n");
    
    // Coalesce adjacent free blocks (simplified implementation)
    // This is a basic implementation - a full implementation would merge adjacent blocks
    
    uint16_t total_blocks_before = 0;
    uint16_t total_blocks_after = 0;
    
    // Count blocks before
    for (int i = 0; i < EZOM_SIZE_CLASSES; i++) {
        total_blocks_before += g_heap.free_counts[i];
    }
    
    // For now, just report the compaction
    total_blocks_after = total_blocks_before; // No actual compaction yet
    
    printf("EZOM: Free list compaction complete - %d blocks before, %d blocks after\n",
           total_blocks_before, total_blocks_after);
}

// Calculate memory fragmentation percentage
float ezom_calculate_fragmentation(void) {
    if (g_heap.bytes_allocated == 0) {
        return 0.0f;
    }
    
    uint16_t total_free = EZOM_HEAP_SIZE - g_heap.bytes_allocated;
    if (total_free == 0) {
        return 0.0f;
    }
    
    // Simple fragmentation calculation based on largest free block
    uint16_t largest_free = g_heap.largest_free_block;
    if (largest_free == 0) {
        return 100.0f; // Complete fragmentation
    }
    
    float fragmentation = (1.0f - (float)largest_free / (float)total_free) * 100.0f;
    return fragmentation < 0.0f ? 0.0f : fragmentation;
}

// Enable/disable garbage collection
void ezom_enable_gc(bool enable) {
    g_heap.gc_enabled = enable;
    printf("EZOM: Garbage collection %s\n", enable ? "enabled" : "disabled");
}

// Set automatic GC triggering
void ezom_set_gc_auto_trigger(bool auto_trigger) {
    // For now, just report the setting
    printf("EZOM: GC auto-trigger %s\n", auto_trigger ? "enabled" : "disabled");
}

// Check if GC should run now
bool ezom_should_gc_now(void) {
    if (!g_heap.gc_enabled) {
        return false;
    }
    
    if (g_gc_roots.gc_in_progress) {
        return false;
    }
    
    // Check if we've exceeded the threshold
    if (g_heap.gc_threshold > 0 && g_heap.bytes_since_last_gc >= g_heap.gc_threshold) {
        return true;
    }
    
    // Check if we're running low on memory
    uint16_t available = EZOM_HEAP_SIZE - g_heap.bytes_allocated;
    if (available < (EZOM_HEAP_SIZE / 10)) { // Less than 10% available
        return true;
    }
    
    return false;
}

// Create a GC checkpoint for debugging
void ezom_gc_checkpoint(void) {
    printf("EZOM: GC Checkpoint - %d objects, %d bytes allocated\n",
           g_heap.objects_allocated, g_heap.bytes_allocated);
}

// Generate comprehensive GC statistics report
void ezom_gc_stats_report(void) {
    printf("\n=== Garbage Collection Statistics ===\n");
    printf("Collections performed: %d\n", g_gc_stats.collections_performed);
    printf("Objects collected: %d\n", g_gc_stats.objects_collected);
    printf("Bytes collected: %d\n", g_gc_stats.bytes_collected);
    printf("GC triggers: %d\n", g_gc_stats.collections_triggered);
    
    if (g_gc_stats.collections_performed > 0) {
        printf("Average objects per collection: %.1f\n", 
               (float)g_gc_stats.objects_collected / g_gc_stats.collections_performed);
        printf("Average bytes per collection: %.1f\n", 
               (float)g_gc_stats.bytes_collected / g_gc_stats.collections_performed);
    }
    
    printf("\nLast GC cycle:\n");
    printf("  Objects before: %d\n", g_gc_stats.objects_before_gc);
    printf("  Objects after: %d\n", g_gc_stats.objects_after_gc);
    printf("  Fragmentation before: %.1f%%\n", g_gc_stats.fragmentation_before_gc);
    printf("  Fragmentation after: %.1f%%\n", g_gc_stats.fragmentation_after_gc);
    
    printf("\nCurrent GC status:\n");
    printf("  GC enabled: %s\n", g_heap.gc_enabled ? "Yes" : "No");
    printf("  GC threshold: %d bytes\n", g_heap.gc_threshold);
    printf("  Bytes since last GC: %d\n", g_heap.bytes_since_last_gc);
    printf("  Should trigger GC: %s\n", ezom_should_gc_now() ? "Yes" : "No");
    printf("  GC efficiency: %.1f%%\n", ezom_gc_efficiency());
    
    printf("======================================\n\n");
}

// Reset GC statistics
void ezom_reset_gc_stats(void) {
    memset(&g_gc_stats, 0, sizeof(g_gc_stats));
    printf("EZOM: GC statistics reset\n");
}

// Calculate GC efficiency
float ezom_gc_efficiency(void) {
    if (g_gc_stats.collections_performed == 0) {
        return 0.0f;
    }
    
    uint32_t total_processed = g_gc_stats.objects_before_gc * g_gc_stats.collections_performed;
    if (total_processed == 0) {
        return 0.0f;
    }
    
    return ((float)g_gc_stats.objects_collected / total_processed) * 100.0f;
}

// Calculate GC pressure
uint16_t ezom_gc_pressure(void) {
    if (g_heap.gc_threshold == 0) {
        return 0;
    }
    
    return (g_heap.bytes_since_last_gc * 100) / g_heap.gc_threshold;
}