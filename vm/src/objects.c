// ============================================================================
// File: src/vm/objects.c
// Object creation functions
// ============================================================================

#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include <stdio.h>
#include <string.h>

// Create integer object
uint24_t ezom_create_integer(int16_t value) {
    uint24_t ptr = ezom_allocate(sizeof(ezom_integer_t));
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_integer_class, EZOM_TYPE_INTEGER);
    
    ezom_integer_t* obj = (ezom_integer_t*)ptr;
    obj->value = value;
    
    return ptr;
}

// Create string object
uint24_t ezom_create_string(const char* data, uint16_t length) {
    uint24_t ptr = ezom_allocate(sizeof(ezom_string_t) + length + 1);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_string_class, EZOM_TYPE_STRING);
    
    ezom_string_t* obj = (ezom_string_t*)ptr;
    obj->length = length;
    memcpy(obj->data, data, length);
    obj->data[length] = '\0';  // Null terminate for C compatibility
    
    return ptr;
}

// Create symbol (interned string)
uint24_t ezom_create_symbol(const char* data, uint16_t length) {
    // For Phase 1, just create like a string - interning comes later
    uint24_t ptr = ezom_allocate(sizeof(ezom_symbol_t) + length + 1);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_symbol_class, EZOM_TYPE_OBJECT);
    
    ezom_symbol_t* obj = (ezom_symbol_t*)ptr;
    obj->length = length;
    obj->hash_cache = ezom_compute_hash(ptr); // Simple hash for now
    memcpy(obj->data, data, length);
    obj->data[length] = '\0';
    
    return ptr;
}

// Create method dictionary
uint24_t ezom_create_method_dictionary(uint16_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = 8;
    
    uint16_t size = sizeof(ezom_method_dict_t) + 
                    (initial_capacity * sizeof(ezom_method_t));
    
    uint24_t ptr = ezom_allocate(size);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_object_class, EZOM_TYPE_OBJECT);
    
    ezom_method_dict_t* dict = (ezom_method_dict_t*)ptr;
    dict->size = 0;
    dict->capacity = initial_capacity;
    
    return ptr;
}

// Object to string conversion (for debugging)
uint24_t ezom_object_to_string(uint24_t obj_ptr) {
    if (!obj_ptr) {
        return ezom_create_string("nil", 3);
    }
    
    ezom_object_t* obj = (ezom_object_t*)obj_ptr;
    
    switch (obj->flags & 0xF0) {
        case EZOM_TYPE_INTEGER: {
            ezom_integer_t* int_obj = (ezom_integer_t*)obj_ptr;
            char buffer[16];
            sprintf(buffer, "%d", int_obj->value);
            return ezom_create_string(buffer, strlen(buffer));
        }
        
        case EZOM_TYPE_STRING:
            return obj_ptr; // Strings represent themselves
            
        default: {
            char buffer[32];
            sprintf(buffer, "Object@0x%06X", obj_ptr);
            return ezom_create_string(buffer, strlen(buffer));
        }
    }
}