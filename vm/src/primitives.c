// ============================================================================
// File: src/vm/primitives.c
// Primitive operations implementation
// ============================================================================

#include "../include/ezom_primitives.h"
#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include <stdio.h>
#include <string.h>

// Primitive function table
ezom_primitive_fn g_primitives[MAX_PRIMITIVES];

// Forward declarations
uint24_t prim_object_class(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_equals(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_hash(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_println(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_add(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_sub(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_mul(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_eq(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_string_length(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_string_concat(uint24_t receiver, uint24_t* args, uint8_t arg_count);

void ezom_init_primitives(void) {
    // Clear primitive table
    memset(g_primitives, 0, sizeof(g_primitives));
    
    // Install Object primitives
    g_primitives[PRIM_OBJECT_CLASS] = prim_object_class;
    g_primitives[PRIM_OBJECT_EQUALS] = prim_object_equals;
    g_primitives[PRIM_OBJECT_HASH] = prim_object_hash;
    g_primitives[PRIM_OBJECT_PRINTLN] = prim_object_println;
    
    // Install Integer primitives
    g_primitives[PRIM_INTEGER_ADD] = prim_integer_add;
    g_primitives[PRIM_INTEGER_SUB] = prim_integer_sub;
    g_primitives[PRIM_INTEGER_MUL] = prim_integer_mul;
    g_primitives[PRIM_INTEGER_EQ] = prim_integer_eq;
    
    // Install String primitives
    g_primitives[PRIM_STRING_LENGTH] = prim_string_length;
    g_primitives[PRIM_STRING_CONCAT] = prim_string_concat;
    
    printf("EZOM: Primitives initialized\n");
}

// Object>>class
uint24_t prim_object_class(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_object_t* obj = (ezom_object_t*)receiver;
    return obj->class_ptr;
}

// Object>>=
uint24_t prim_object_equals(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return 0;
    
    // Simple identity comparison for now
    return (receiver == args[0]) ? 1 : 0; // Return 1 for true, 0 for false
}

// Object>>hash
uint24_t prim_object_hash(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_object_t* obj = (ezom_object_t*)receiver;
    return ezom_create_integer(obj->hash);
}

// Object>>println
uint24_t prim_object_println(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    // Convert object to string and print
    uint24_t str = ezom_object_to_string(receiver);
    if (str) {
        ezom_string_t* string_obj = (ezom_string_t*)str;
        printf("%.*s\n", string_obj->length, string_obj->data);
    } else {
        printf("nil\n");
    }
    
    return receiver;  // Return self
}

// Integer>>+
uint24_t prim_integer_add(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return 0;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    // Type check
    if ((recv->header.flags & 0xF0) != EZOM_TYPE_INTEGER ||
        (arg->header.flags & 0xF0) != EZOM_TYPE_INTEGER) {
        printf("Type error in integer addition\n");
        return 0;
    }
    
    return ezom_create_integer(recv->value + arg->value);
}

// Integer>>-
uint24_t prim_integer_sub(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return 0;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if ((recv->header.flags & 0xF0) != EZOM_TYPE_INTEGER ||
        (arg->header.flags & 0xF0) != EZOM_TYPE_INTEGER) {
        printf("Type error in integer subtraction\n");
        return 0;
    }
    
    return ezom_create_integer(recv->value - arg->value);
}

// Integer>>*
uint24_t prim_integer_mul(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return 0;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if ((recv->header.flags & 0xF0) != EZOM_TYPE_INTEGER ||
        (arg->header.flags & 0xF0) != EZOM_TYPE_INTEGER) {
        printf("Type error in integer multiplication\n");
        return 0;
    }
    
    return ezom_create_integer(recv->value * arg->value);
}

// Integer>>=
uint24_t prim_integer_eq(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return 0;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if ((recv->header.flags & 0xF0) != EZOM_TYPE_INTEGER ||
        (arg->header.flags & 0xF0) != EZOM_TYPE_INTEGER) {
        return 0; // Not equal if not both integers
    }
    
    return (recv->value == arg->value) ? 1 : 0;
}

// String>>length
uint24_t prim_string_length(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_string_t* str = (ezom_string_t*)receiver;
    
    if ((str->header.flags & 0xF0) != EZOM_TYPE_STRING) {
        printf("Type error: length sent to non-string\n");
        return 0;
    }
    
    return ezom_create_integer(str->length);
}

// String>>+
uint24_t prim_string_concat(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return 0;
    
    ezom_string_t* str1 = (ezom_string_t*)receiver;
    ezom_string_t* str2 = (ezom_string_t*)args[0];
    
    if ((str1->header.flags & 0xF0) != EZOM_TYPE_STRING ||
        (str2->header.flags & 0xF0) != EZOM_TYPE_STRING) {
        printf("Type error in string concatenation\n");
        return 0;
    }
    
    uint16_t new_length = str1->length + str2->length;
    uint24_t result = ezom_allocate(sizeof(ezom_string_t) + new_length + 1);
    
    if (!result) return 0;
    
    ezom_init_object(result, g_string_class, EZOM_TYPE_STRING);
    
    ezom_string_t* result_str = (ezom_string_t*)result;
    result_str->length = new_length;
    
    memcpy(result_str->data, str1->data, str1->length);
    memcpy(result_str->data + str1->length, str2->data, str2->length);
    result_str->data[new_length] = '\0';
    
    return result;
}