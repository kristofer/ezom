// ============================================================================
// File: src/vm/object.c
// Core object system implementation
// ============================================================================

#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include <stdio.h>
#include <string.h>

// Global class pointers (defined in bootstrap.c)
extern uint24_t g_object_class;
extern uint24_t g_class_class;
extern uint24_t g_integer_class;
extern uint24_t g_string_class;
extern uint24_t g_symbol_class;
extern uint24_t g_nil;

void ezom_init_object_system(void) {
    printf("EZOM: Initializing object system...\n");
    
    // Bootstrap will be implemented later
    // For now, just print initialization message
}

void ezom_init_object(uint24_t obj_ptr, uint24_t class_ptr, uint8_t type) {
    ezom_object_t* obj = (ezom_object_t*)obj_ptr;
    obj->class_ptr = class_ptr;
    obj->hash = ezom_compute_hash(obj_ptr);
    obj->flags = type;
}

uint16_t ezom_compute_hash(uint24_t obj_ptr) {
    // Simple hash based on address
    return (uint16_t)(obj_ptr ^ (obj_ptr >> 8) ^ (obj_ptr >> 16));
}

uint16_t ezom_get_object_size(uint24_t obj_ptr) {
    ezom_object_t* obj = (ezom_object_t*)obj_ptr;
    
    switch (obj->flags & 0xF0) {
        case EZOM_TYPE_INTEGER:
            return sizeof(ezom_integer_t);
            
        case EZOM_TYPE_STRING: {
            ezom_string_t* str = (ezom_string_t*)obj_ptr;
            return sizeof(ezom_string_t) + str->length + 1;
        }
        
        case EZOM_TYPE_CLASS:
            return sizeof(ezom_class_t);
            
        default:
            return sizeof(ezom_object_t);
    }
}

bool ezom_is_valid_object(uint24_t obj_ptr) {
    // Basic validation
    if (obj_ptr < EZOM_HEAP_START || obj_ptr >= EZOM_HEAP_END) {
        return false;
    }
    
    // Check alignment
    if (obj_ptr & 1) {
        return false;
    }
    
    return true;
}