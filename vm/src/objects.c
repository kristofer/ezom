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

// NEW: Create array object
uint24_t ezom_create_array(uint16_t size) {
    uint16_t total_size = sizeof(ezom_array_t) + (size * sizeof(uint24_t));
    uint24_t ptr = ezom_allocate(total_size);
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

// NEW: Create block object
uint24_t ezom_create_block(uint8_t param_count, uint8_t local_count, uint24_t outer_context) {
    uint16_t captured_size = local_count * sizeof(uint24_t);
    uint24_t ptr = ezom_allocate(sizeof(ezom_block_t) + captured_size);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_block_class, EZOM_TYPE_BLOCK);
    
    ezom_block_t* obj = (ezom_block_t*)ptr;
    obj->outer_context = outer_context;
    obj->code = 0; // Will be set by parser
    obj->param_count = param_count;
    obj->local_count = local_count;
    
    // Initialize captured variables to nil
    for (uint8_t i = 0; i < local_count; i++) {
        obj->captured_vars[i] = g_nil;
    }
    
    return ptr;
}

// NEW: Create execution context
uint24_t ezom_create_context(uint24_t outer_context, uint8_t local_count) {
    uint16_t total_size = sizeof(ezom_context_t) + (local_count * sizeof(uint24_t));
    uint24_t ptr = ezom_allocate(total_size);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_context_class, EZOM_TYPE_OBJECT);
    
    ezom_context_t* obj = (ezom_context_t*)ptr;
    obj->outer_context = outer_context;
    obj->method = 0;
    obj->receiver = 0;
    obj->sender = 0;
    obj->pc = 0;
    obj->local_count = local_count;
    
    // Initialize locals to nil
    for (uint8_t i = 0; i < local_count; i++) {
        obj->locals[i] = g_nil;
    }
    
    return ptr;
}

// Enhanced object to string conversion
uint24_t ezom_object_to_string(uint24_t obj_ptr) {
    if (!obj_ptr || obj_ptr == g_nil) {
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
            
        case EZOM_TYPE_BOOLEAN:
            if (obj_ptr == g_true) {
                return ezom_create_string("true", 4);
            } else if (obj_ptr == g_false) {
                return ezom_create_string("false", 5);
            }
            break;
            
        case EZOM_TYPE_ARRAY: {
            ezom_array_t* array = (ezom_array_t*)obj_ptr;
            char buffer[64];
            sprintf(buffer, "Array[%d]", array->size);
            return ezom_create_string(buffer, strlen(buffer));
        }
            
        case EZOM_TYPE_BLOCK: {
            char buffer[32];
            sprintf(buffer, "Block@0x%06X", obj_ptr);
            return ezom_create_string(buffer, strlen(buffer));
        }
            
        default: {
            char buffer[32];
            sprintf(buffer, "Object@0x%06X", obj_ptr);
            return ezom_create_string(buffer, strlen(buffer));
        }
    }
    
    return ezom_create_string("Unknown", 7);
}