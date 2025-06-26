// ============================================================================
// File: src/vm/primitives.c
// Primitive operations implementation
// ============================================================================

#include "../include/ezom_primitives.h"
#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Enhanced global class pointers
extern uint24_t g_array_class;
extern uint24_t g_block_class;
extern uint24_t g_boolean_class;
extern uint24_t g_true_class;
extern uint24_t g_false_class;
extern uint24_t g_nil_class;
extern uint24_t g_context_class;

// Global singleton objects
extern uint24_t g_nil;
extern uint24_t g_true;
extern uint24_t g_false;

// Primitive function table
ezom_primitive_fn g_primitives[MAX_PRIMITIVES];

// Forward declarations (ENHANCED)
uint24_t prim_object_class(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_equals(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_hash(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_println(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_is_nil(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_object_not_nil(uint24_t receiver, uint24_t* args, uint8_t arg_count);

uint24_t prim_integer_add(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_sub(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_mul(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_div(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_mod(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_lt(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_gt(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_lte(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_gte(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_eq(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_neq(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_as_string(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_abs(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_to_do(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_integer_times_repeat(uint24_t receiver, uint24_t* args, uint8_t arg_count);

uint24_t prim_string_length(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_string_concat(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_string_equals(uint24_t receiver, uint24_t* args, uint8_t arg_count);

uint24_t prim_array_new(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_array_at(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_array_at_put(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_array_length(uint24_t receiver, uint24_t* args, uint8_t arg_count);

uint24_t prim_true_if_true(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_true_if_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_false_if_true(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_false_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_false_if_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_boolean_not(uint24_t receiver, uint24_t* args, uint8_t arg_count);

uint24_t prim_block_value(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_block_value_with(uint24_t receiver, uint24_t* args, uint8_t arg_count);

void ezom_init_primitives(void) {
    // Clear primitive table
    memset(g_primitives, 0, sizeof(g_primitives));
    
    // Install Object primitives (ENHANCED)
    g_primitives[PRIM_OBJECT_CLASS] = prim_object_class;
    g_primitives[PRIM_OBJECT_EQUALS] = prim_object_equals;
    g_primitives[PRIM_OBJECT_HASH] = prim_object_hash;
    g_primitives[PRIM_OBJECT_PRINTLN] = prim_object_println;
    g_primitives[PRIM_OBJECT_IS_NIL] = prim_object_is_nil;
    g_primitives[PRIM_OBJECT_NOT_NIL] = prim_object_not_nil;
    
    // Install Integer primitives (ENHANCED)
    g_primitives[PRIM_INTEGER_ADD] = prim_integer_add;
    g_primitives[PRIM_INTEGER_SUB] = prim_integer_sub;
    g_primitives[PRIM_INTEGER_MUL] = prim_integer_mul;
    g_primitives[PRIM_INTEGER_DIV] = prim_integer_div;
    g_primitives[PRIM_INTEGER_MOD] = prim_integer_mod;
    g_primitives[PRIM_INTEGER_LT] = prim_integer_lt;
    g_primitives[PRIM_INTEGER_GT] = prim_integer_gt;
    g_primitives[PRIM_INTEGER_LTE] = prim_integer_lte;
    g_primitives[PRIM_INTEGER_GTE] = prim_integer_gte;
    g_primitives[PRIM_INTEGER_EQ] = prim_integer_eq;
    g_primitives[PRIM_INTEGER_NEQ] = prim_integer_neq;
    g_primitives[PRIM_INTEGER_AS_STRING] = prim_integer_as_string;
    g_primitives[PRIM_INTEGER_ABS] = prim_integer_abs;
    g_primitives[PRIM_INTEGER_TO_DO] = prim_integer_to_do;
    g_primitives[PRIM_INTEGER_TIMES_REPEAT] = prim_integer_times_repeat;
    
    // Install String primitives
    g_primitives[PRIM_STRING_LENGTH] = prim_string_length;
    g_primitives[PRIM_STRING_CONCAT] = prim_string_concat;
    g_primitives[PRIM_STRING_EQUALS] = prim_string_equals;
    
    // Install Array primitives (NEW)
    g_primitives[PRIM_ARRAY_NEW] = prim_array_new;
    g_primitives[PRIM_ARRAY_AT] = prim_array_at;
    g_primitives[PRIM_ARRAY_AT_PUT] = prim_array_at_put;
    g_primitives[PRIM_ARRAY_LENGTH] = prim_array_length;
    
    // Install Boolean primitives (NEW)
    g_primitives[PRIM_TRUE_IF_TRUE] = prim_true_if_true;
    g_primitives[PRIM_TRUE_IF_FALSE] = prim_true_if_false;
    g_primitives[PRIM_TRUE_IF_TRUE_IF_FALSE] = prim_true_if_true_if_false;
    g_primitives[PRIM_FALSE_IF_TRUE] = prim_false_if_true;
    g_primitives[PRIM_FALSE_IF_FALSE] = prim_false_if_false;
    g_primitives[PRIM_FALSE_IF_TRUE_IF_FALSE] = prim_false_if_true_if_false;
    g_primitives[PRIM_BOOLEAN_NOT] = prim_boolean_not;
    
    // Install Block primitives (NEW)
    g_primitives[PRIM_BLOCK_VALUE] = prim_block_value;
    g_primitives[PRIM_BLOCK_VALUE_WITH] = prim_block_value_with;
    
    printf("EZOM: Enhanced primitives initialized (%d total)\n", MAX_PRIMITIVES);
}

// Object>>class
uint24_t prim_object_class(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_object_t* obj = (ezom_object_t*)receiver;
    return obj->class_ptr;
}

// Object>>=
uint24_t prim_object_equals(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    // Simple identity comparison for now
    return (receiver == args[0]) ? g_true : g_false;
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

// Object>>isNil
uint24_t prim_object_is_nil(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    return (receiver == g_nil) ? g_true : g_false;
}

// Object>>notNil  
uint24_t prim_object_not_nil(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    return (receiver != g_nil) ? g_true : g_false;
}

// Integer>>+
uint24_t prim_integer_add(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    // Type check
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in integer addition\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value + arg->value);
}

// Integer>>-
uint24_t prim_integer_sub(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in integer subtraction\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value - arg->value);
}

// Integer>>*
uint24_t prim_integer_mul(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in integer multiplication\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value * arg->value);
}

// Integer>>/
uint24_t prim_integer_div(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in integer division\n");
        return g_nil;
    }
    
    if (arg->value == 0) {
        printf("Division by zero\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value / arg->value);
}

// Integer>>\\  (modulo)
uint24_t prim_integer_mod(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in integer modulo\n");
        return g_nil;
    }
    
    if (arg->value == 0) {
        printf("Division by zero in modulo\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value % arg->value);
}

// Integer>><
uint24_t prim_integer_lt(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value < arg->value) ? g_true : g_false;
}

// Integer>>>
uint24_t prim_integer_gt(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value > arg->value) ? g_true : g_false;
}

// Integer>><=
uint24_t prim_integer_lte(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value <= arg->value) ? g_true : g_false;
}

// Integer>>>=
uint24_t prim_integer_gte(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value >= arg->value) ? g_true : g_false;
}

// Integer>>=
uint24_t prim_integer_eq(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value == arg->value) ? g_true : g_false;
}

// Integer>>~=
uint24_t prim_integer_neq(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_true; // Different types are not equal
    }
    
    return (recv->value != arg->value) ? g_true : g_false;
}

// Integer>>asString
// Integer>>asString
uint24_t prim_integer_as_string(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_integer(receiver)) {
        printf("Type error: asString sent to non-integer\n");
        return g_nil;
    }
    
    ezom_integer_t* int_obj = (ezom_integer_t*)receiver;
    char buffer[16];
    sprintf(buffer, "%d", int_obj->value);
    return ezom_create_string(buffer, strlen(buffer));
}

// Integer>>abs
uint24_t prim_integer_abs(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_integer(receiver)) {
        printf("Type error: abs sent to non-integer\n");
        return g_nil;
    }
    
    ezom_integer_t* int_obj = (ezom_integer_t*)receiver;
    int16_t value = int_obj->value;
    return ezom_create_integer(value < 0 ? -value : value);
}

// Integer>>to:do:
uint24_t prim_integer_to_do(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2) return receiver;
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0]) || !ezom_is_block(args[1])) {
        printf("Type error in to:do:\n");
        return receiver;
    }
    
    ezom_integer_t* start = (ezom_integer_t*)receiver;
    ezom_integer_t* end = (ezom_integer_t*)args[0];
    uint24_t block = args[1];
    
    // Execute loop: start to: end do: block
    for (int16_t i = start->value; i <= end->value; i++) {
        uint24_t index_obj = ezom_create_integer(i);
        uint24_t block_args[] = {index_obj};
        
        // Call block value: index
        g_primitives[PRIM_BLOCK_VALUE_WITH](block, block_args, 1);
    }
    
    return receiver;
}

// Integer>>timesRepeat:
uint24_t prim_integer_times_repeat(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return receiver;
    
    if (!ezom_is_integer(receiver) || !ezom_is_block(args[0])) {
        printf("Type error in timesRepeat:\n");
        return receiver;
    }
    
    ezom_integer_t* count = (ezom_integer_t*)receiver;
    uint24_t block = args[0];
    
    // Execute block count times
    for (int16_t i = 0; i < count->value; i++) {
        g_primitives[PRIM_BLOCK_VALUE](block, NULL, 0);
    }
    
    return receiver;
}

// String>>length
// String>>length
uint24_t prim_string_length(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_string_t* str = (ezom_string_t*)receiver;
    
    if (!ezom_is_string(receiver)) {
        printf("Type error: length sent to non-string\n");
        return g_nil;
    }
    
    return ezom_create_integer(str->length);
}

// String>>+
uint24_t prim_string_concat(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_string_t* str1 = (ezom_string_t*)receiver;
    ezom_string_t* str2 = (ezom_string_t*)args[0];
    
    if (!ezom_is_string(receiver) || !ezom_is_string(args[0])) {
        printf("Type error in string concatenation\n");
        return g_nil;
    }
    
    uint16_t new_length = str1->length + str2->length;
    uint24_t result = ezom_allocate(sizeof(ezom_string_t) + new_length + 1);
    
    if (!result) return g_nil;
    
    ezom_init_object(result, g_string_class, EZOM_TYPE_STRING);
    
    ezom_string_t* result_str = (ezom_string_t*)result;
    result_str->length = new_length;
    
    memcpy(result_str->data, str1->data, str1->length);
    memcpy(result_str->data + str1->length, str2->data, str2->length);
    result_str->data[new_length] = '\0';
    
    return result;
}

// String>>=
uint24_t prim_string_equals(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_string_t* str1 = (ezom_string_t*)receiver;
    ezom_string_t* str2 = (ezom_string_t*)args[0];
    
    if (!ezom_is_string(receiver) || !ezom_is_string(args[0])) {
        return g_false;
    }
    
    if (str1->length != str2->length) {
        return g_false;
    }
    
    return (strncmp(str1->data, str2->data, str1->length) == 0) ? g_true : g_false;
}

// ============================================================================
// ARRAY PRIMITIVES
// ============================================================================

// Array class>>new:
uint24_t prim_array_new(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !ezom_is_integer(args[0])) {
        printf("Type error in Array new:\n");
        return 0;
    }
    
    ezom_integer_t* size_obj = (ezom_integer_t*)args[0];
    if (size_obj->value < 0) {
        printf("Negative array size\n");
        return 0;
    }
    
    return ezom_create_array(size_obj->value);
}

// Array>>at:
uint24_t prim_array_at(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !ezom_is_array(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in Array at:\n");
        return 0;
    }
    
    ezom_array_t* array = (ezom_array_t*)receiver;
    ezom_integer_t* index_obj = (ezom_integer_t*)args[0];
    
    // SOM uses 1-based indexing
    int16_t index = index_obj->value - 1;
    
    if (index < 0 || index >= array->size) {
        printf("Array index out of bounds: %d (size: %d)\n", index_obj->value, array->size);
        return 0;
    }
    
    return array->elements[index];
}

// Array>>at:put:
uint24_t prim_array_at_put(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2 || !ezom_is_array(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in Array at:put:\n");
        return 0;
    }
    
    ezom_array_t* array = (ezom_array_t*)receiver;
    ezom_integer_t* index_obj = (ezom_integer_t*)args[0];
    uint24_t value = args[1];
    
    // SOM uses 1-based indexing
    int16_t index = index_obj->value - 1;
    
    if (index < 0 || index >= array->size) {
        printf("Array index out of bounds: %d (size: %d)\n", index_obj->value, array->size);
        return 0;
    }
    
    array->elements[index] = value;
    return value;
}

// Array>>length
uint24_t prim_array_length(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_array(receiver)) {
        printf("Type error: length sent to non-array\n");
        return 0;
    }
    
    ezom_array_t* array = (ezom_array_t*)receiver;
    return ezom_create_integer(array->size);
}

// ============================================================================
// BOOLEAN PRIMITIVES
// ============================================================================

// True>>ifTrue:
uint24_t prim_true_if_true(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !ezom_is_block(args[0])) {
        return g_nil;
    }
    
    // Evaluate the block
    return g_primitives[PRIM_BLOCK_VALUE](args[0], NULL, 0);
}

// True>>ifFalse:
uint24_t prim_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    // True object ignores false block
    return g_nil;
}

// True>>ifTrue:ifFalse:
uint24_t prim_true_if_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2 || !ezom_is_block(args[0])) {
        return g_nil;
    }
    
    // Evaluate the true block (first argument)
    return g_primitives[PRIM_BLOCK_VALUE](args[0], NULL, 0);
}

// False>>ifTrue:
uint24_t prim_false_if_true(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    // False object ignores true block
    return g_nil;
}

// False>>ifFalse:
uint24_t prim_false_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !ezom_is_block(args[0])) {
        return g_nil;
    }
    
    // Evaluate the block
    return g_primitives[PRIM_BLOCK_VALUE](args[0], NULL, 0);
}

// False>>ifTrue:ifFalse:
uint24_t prim_false_if_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2 || !ezom_is_block(args[1])) {
        return g_nil;
    }
    
    // Evaluate the false block (second argument)
    return g_primitives[PRIM_BLOCK_VALUE](args[1], NULL, 0);
}

// Boolean>>not
uint24_t prim_boolean_not(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (receiver == g_true) {
        return g_false;
    } else if (receiver == g_false) {
        return g_true;
    }
    
    printf("Type error: not sent to non-boolean\n");
    return g_nil;
}

// ============================================================================
// BLOCK PRIMITIVES
// ============================================================================

// Block>>value
uint24_t prim_block_value(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_block(receiver)) {
        printf("Type error: value sent to non-block\n");
        return g_nil;
    }
    
    ezom_block_t* block = (ezom_block_t*)receiver;
    
    if (block->param_count != 0) {
        printf("Block expects %d parameters, got 0\n", block->param_count);
        return g_nil;
    }
    
    // For Phase 1.5, we'll simulate block execution
    printf("Executing block with 0 parameters\n");
    
    // Return nil for now - will be enhanced with proper execution in Phase 2
    return g_nil;
}

// Block>>value:
uint24_t prim_block_value_with(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_block(receiver)) {
        printf("Type error: value: sent to non-block\n");
        return g_nil;
    }
    
    if (arg_count != 1) {
        printf("Block value: expects 1 argument, got %d\n", arg_count);
        return g_nil;
    }
    
    ezom_block_t* block = (ezom_block_t*)receiver;
    
    if (block->param_count != 1) {
        printf("Block expects %d parameters, got 1\n", block->param_count);
        return g_nil;
    }
    
    // For Phase 1.5, we'll simulate block execution with argument
    printf("Executing block with 1 parameter\n");
    
    // Return the argument for now - will be enhanced with proper execution
    return args[0];
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool ezom_is_integer(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)obj;
    return (object->flags & 0xF0) == EZOM_TYPE_INTEGER;
}

bool ezom_is_string(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)obj;
    return (object->flags & 0xF0) == EZOM_TYPE_STRING;
}

bool ezom_is_array(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)obj;
    return (object->flags & 0xF0) == EZOM_TYPE_ARRAY;
}

bool ezom_is_block(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)obj;
    return (object->flags & 0xF0) == EZOM_TYPE_BLOCK;
}

bool ezom_is_boolean(uint24_t obj) {
    return (obj == g_true || obj == g_false);
}