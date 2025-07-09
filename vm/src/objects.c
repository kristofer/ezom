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
    printf("DEBUG: ezom_create_integer called with value=%d\n", value);
    printf("DEBUG: g_integer_class=0x%06X\n", g_integer_class);
    
    uint24_t ptr = ezom_allocate(sizeof(ezom_integer_t));
    printf("DEBUG: ezom_allocate returned ptr=0x%06X\n", ptr);
    if (!ptr) return 0;
    
    printf("DEBUG: About to call ezom_init_object...\n");
    
    // Bootstrap safety: use Object class if Integer class not ready
    uint24_t class_ptr = g_integer_class ? g_integer_class : g_object_class;
    if (!class_ptr) {
        // Ultra-bootstrap: create minimal object without class
        ezom_object_t* header = (ezom_object_t*)ptr;
        header->class_ptr = 0; // Bootstrap: no class initially
        header->hash = (uint16_t)(value & 0xFFFF);
        header->flags = EZOM_TYPE_INTEGER;
        printf("DEBUG: Ultra-bootstrap mode - no class available\n");
    } else {
        ezom_init_object(ptr, class_ptr, EZOM_TYPE_INTEGER);
        printf("DEBUG: ezom_init_object completed with class=0x%06X\n", class_ptr);
    }
    
    ezom_integer_t* obj = (ezom_integer_t*)ptr;
    printf("DEBUG: Setting obj->value = %d\n", value);
    obj->value = value;
    
    printf("DEBUG: ezom_create_integer returning ptr=0x%06X\n", ptr);
    return ptr;
}

// Create string object
uint24_t ezom_create_string(const char* data, uint16_t length) {
    printf("DEBUG: ezom_create_string entry, data='%.*s', length=%d\n", length, data, length);
    
    uint24_t ptr = ezom_allocate(sizeof(ezom_string_t) + length + 1);
    printf("DEBUG: ezom_allocate returned ptr=0x%06X\n", ptr);
    if (!ptr) return 0;
    
    // Bootstrap safety: use Object class if String class not ready
    uint24_t class_ptr = g_string_class ? g_string_class : g_object_class;
    printf("DEBUG: class_ptr=0x%06X (g_string_class=0x%06X, g_object_class=0x%06X)\n", 
           class_ptr, g_string_class, g_object_class);
    
    if (class_ptr) {
        printf("DEBUG: Calling ezom_init_object with class\n");
        ezom_init_object(ptr, class_ptr, EZOM_TYPE_STRING);
        printf("DEBUG: ezom_init_object completed\n");
    } else {
        printf("DEBUG: Using ultra-bootstrap mode\n");
        // Ultra-bootstrap: create minimal object without class
        ezom_object_t* header = (ezom_object_t*)ptr;
        header->class_ptr = 0;
        header->hash = ezom_compute_hash(ptr);
        header->flags = EZOM_TYPE_STRING;
        printf("DEBUG: Ultra-bootstrap completed\n");
    }
    
    printf("DEBUG: Setting up string data\n");
    ezom_string_t* obj = (ezom_string_t*)ptr;
    obj->length = length;
    
    printf("DEBUG: About to memcpy data\n");
    memcpy(obj->data, data, length);
    obj->data[length] = '\0';  // Null terminate for C compatibility
    
    printf("DEBUG: ezom_create_string returning ptr=0x%06X\n", ptr);
    return ptr;
}

// Create symbol (interned string)
uint24_t ezom_create_symbol(const char* data, uint16_t length) {
    // For Phase 1, just create like a string - interning comes later
    printf("DEBUG: ezom_create_symbol called: data='%.*s' length=%d\n", length, data, length);
    
    uint24_t ptr = ezom_allocate(sizeof(ezom_symbol_t) + length + 1);
    printf("DEBUG: ezom_allocate returned ptr=0x%06X\n", ptr);
    if (!ptr) return 0;
    
    printf("DEBUG: About to call ezom_init_object with ptr=0x%06X, g_symbol_class=0x%06X\n", ptr, g_symbol_class);
    ezom_init_object(ptr, g_symbol_class, EZOM_TYPE_OBJECT);
    printf("DEBUG: ezom_init_object completed\n");
    
    ezom_symbol_t* obj = (ezom_symbol_t*)ptr;
    printf("DEBUG: obj pointer = 0x%06X\n", (uint24_t)obj);
    
    // FIXED: Use explicit pointer arithmetic instead of flexible array member
    // Calculate data pointer manually to avoid ez80 compiler issues
    char* data_ptr = (char*)(ptr + sizeof(ezom_object_t) + sizeof(uint16_t) + sizeof(uint16_t));
    printf("DEBUG: Calculated data_ptr = 0x%06X\n", (uint24_t)data_ptr);
    printf("DEBUG: Expected offset = %d bytes\n", sizeof(ezom_object_t) + sizeof(uint16_t) + sizeof(uint16_t));
    
    obj->length = length;
    obj->hash_cache = ezom_compute_hash(ptr); // Simple hash for now
    
    printf("DEBUG: About to memcpy to address 0x%06X\n", (uint24_t)data_ptr);
    memcpy(data_ptr, data, length);
    data_ptr[length] = '\0';
    
    printf("DEBUG: ezom_create_symbol returning ptr=0x%06X\n", ptr);
    return ptr;
}

// Create method dictionary
uint24_t ezom_create_method_dictionary(uint16_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = 8;
    
    uint16_t size = sizeof(ezom_method_dict_t) + 
                    (initial_capacity * sizeof(ezom_method_t));
    
    uint24_t ptr = ezom_allocate(size);
    if (!ptr) return 0;
    
    // Bootstrap safety: only initialize if Object class exists
    if (g_object_class) {
        ezom_init_object(ptr, g_object_class, EZOM_TYPE_OBJECT);
    } else {
        // Ultra-bootstrap: create minimal object
        ezom_object_t* header = (ezom_object_t*)ptr;
        header->class_ptr = 0;
        header->hash = ezom_compute_hash(ptr);
        header->flags = EZOM_TYPE_OBJECT;
    }
    
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
            sprintf(buffer, "Block@0x%06lX", obj_ptr);
            return ezom_create_string(buffer, strlen(buffer));
        }
            
        default: {
            char buffer[32];
            sprintf(buffer, "Object@0x%06lX", obj_ptr);
            return ezom_create_string(buffer, strlen(buffer));
        }
    }
    
    return ezom_create_string("Unknown", 7);
}