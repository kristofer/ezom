// ============================================================================
// File: src/vm/primitives.c
// Primitive operations implementation
// ============================================================================

#include "../include/ezom_primitives.h"
#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_context.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Enhanced global class pointers
extern uint24_t g_object_class;
extern uint24_t g_integer_class;
extern uint24_t g_string_class;
extern uint24_t g_symbol_class;
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
uint24_t prim_block_while_true(uint24_t receiver, uint24_t* args, uint8_t arg_count);
uint24_t prim_block_while_false(uint24_t receiver, uint24_t* args, uint8_t arg_count);

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
    g_primitives[PRIM_BLOCK_WHILE_TRUE] = prim_block_while_true;
    g_primitives[PRIM_BLOCK_WHILE_FALSE] = prim_block_while_false;
    
    printf("EZOM: Enhanced primitives initialized (%d total)\n", MAX_PRIMITIVES);
}

// Object>>class
uint24_t prim_object_class(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
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
    ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    return ezom_create_integer(obj->hash);
}

// Object>>println
uint24_t prim_object_println(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    // Convert object to string and print
    uint24_t str = ezom_object_to_string(receiver);
    if (str) {
        ezom_string_t* string_obj = (ezom_string_t*)EZOM_OBJECT_PTR(str);
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
    // Immediate corruption check - return safe value if corrupted
    if (receiver == 0xffffff || (args && args[0] == 0xffffff)) {
        return ezom_create_integer(0); // Return 0 instead of crashing
    }
    
    if (arg_count != 1) return g_nil;
    if (!receiver || !args || !args[0]) return g_nil;
    
    // Simple type checking without validation calls that might crash
    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    ezom_object_t* arg_obj = (ezom_object_t*)EZOM_OBJECT_PTR(args[0]);
    
    // Direct type flag checking instead of calling ezom_is_integer
    if ((recv_obj->flags & 0xF0) != EZOM_TYPE_INTEGER || 
        (arg_obj->flags & 0xF0) != EZOM_TYPE_INTEGER) {
        return g_nil;
    }
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    return ezom_create_integer(recv->value + arg->value);
}

// Integer>>-
uint24_t prim_integer_sub(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in integer subtraction\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value - arg->value);
}

// Integer>>*
uint24_t prim_integer_mul(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        printf("Type error in integer multiplication\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value * arg->value);
}

// Integer>>/
uint24_t prim_integer_div(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
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
    if (!receiver || !args || !args[0]) return g_nil;
    
    // Direct type checking without validation calls that might crash
    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    ezom_object_t* arg_obj = (ezom_object_t*)EZOM_OBJECT_PTR(args[0]);
    
    if ((recv_obj->flags & 0xF0) != EZOM_TYPE_INTEGER || 
        (arg_obj->flags & 0xF0) != EZOM_TYPE_INTEGER) {
        printf("Type error in integer modulo\n");
        return g_nil;
    }
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    if (arg->value == 0) {
        printf("Division by zero in modulo\n");
        return g_nil;
    }
    
    return ezom_create_integer(recv->value % arg->value);
}

// Integer>><
uint24_t prim_integer_lt(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value < arg->value) ? g_true : g_false;
}

// Integer>>>
uint24_t prim_integer_gt(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value > arg->value) ? g_true : g_false;
}

// Integer>><=
uint24_t prim_integer_lte(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    if (!receiver || !args || !args[0]) return g_false;
    
    // Direct type checking without validation calls that might crash
    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    ezom_object_t* arg_obj = (ezom_object_t*)EZOM_OBJECT_PTR(args[0]);
    
    if ((recv_obj->flags & 0xF0) != EZOM_TYPE_INTEGER || 
        (arg_obj->flags & 0xF0) != EZOM_TYPE_INTEGER) {
        return g_false;
    }
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    return (recv->value <= arg->value) ? g_true : g_false;
}

// Integer>>>=
uint24_t prim_integer_gte(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    if (!ezom_is_integer(receiver) || !ezom_is_integer(args[0])) {
        return g_false;
    }
    
    return (recv->value >= arg->value) ? g_true : g_false;
}

// Integer>>=
uint24_t prim_integer_eq(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    if (!receiver || !args || !args[0]) return g_false;
    
    // Direct type checking without validation calls that might crash
    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    ezom_object_t* arg_obj = (ezom_object_t*)EZOM_OBJECT_PTR(args[0]);
    
    if ((recv_obj->flags & 0xF0) != EZOM_TYPE_INTEGER || 
        (arg_obj->flags & 0xF0) != EZOM_TYPE_INTEGER) {
        return g_false;
    }
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    return (recv->value == arg->value) ? g_true : g_false;
}

// Integer>>~=
uint24_t prim_integer_neq(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    if (!receiver || !args || !args[0]) return g_false;
    
    // Direct type checking without validation calls that might crash
    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    ezom_object_t* arg_obj = (ezom_object_t*)EZOM_OBJECT_PTR(args[0]);
    
    if ((recv_obj->flags & 0xF0) != EZOM_TYPE_INTEGER || 
        (arg_obj->flags & 0xF0) != EZOM_TYPE_INTEGER) {
        return g_true; // Different types are not equal
    }
    
    ezom_integer_t* recv = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* arg = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
    return (recv->value != arg->value) ? g_true : g_false;
}

// Integer>>asString
uint24_t prim_integer_as_string(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    printf("DEBUG: prim_integer_as_string entry, receiver=0x%06X\n", receiver);
    
    if (!receiver) {
        printf("DEBUG: Null receiver in asString\n");
        return g_nil;
    }
    
    // Direct type checking without validation calls that might crash
    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    
    printf("DEBUG: Checking object type, flags=0x%02X\n", recv_obj->flags);
    
    if ((recv_obj->flags & 0xF0) != EZOM_TYPE_INTEGER) {
        printf("Type error: asString sent to non-integer\n");
        return g_nil;
    }
    
    ezom_integer_t* int_obj = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    char buffer[16];
    sprintf(buffer, "%d", int_obj->value);
    printf("DEBUG: sprintf completed, buffer='%s', about to call ezom_create_string\n", buffer);
    
    uint24_t result = ezom_create_string(buffer, strlen(buffer));
    printf("DEBUG: ezom_create_string returned 0x%06X\n", result);
    
    return result;
}

// Integer>>abs
uint24_t prim_integer_abs(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_integer(receiver)) {
        printf("Type error: abs sent to non-integer\n");
        return g_nil;
    }
    
    ezom_integer_t* int_obj = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
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
    
    ezom_integer_t* start = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* end = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
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
    
    ezom_integer_t* count = (ezom_integer_t*)EZOM_OBJECT_PTR(receiver);
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
    ezom_string_t* str = (ezom_string_t*)EZOM_OBJECT_PTR(receiver);
    
    if (!ezom_is_string(receiver)) {
        printf("Type error: length sent to non-string\n");
        return g_nil;
    }
    
    return ezom_create_integer(str->length);
}

// String>>+
uint24_t prim_string_concat(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    printf("DEBUG: prim_string_concat entry\n");
    
    if (arg_count != 1) {
        printf("DEBUG: Wrong arg count in string concat\n");
        return g_nil;
    }
    if (!receiver || !args || !args[0]) {
        printf("DEBUG: Null pointers in string concat\n");
        return g_nil;
    }
    
    printf("DEBUG: Checking object types\n");
    
    // Direct type checking without validation calls that might crash
    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(receiver);
    ezom_object_t* arg_obj = (ezom_object_t*)EZOM_OBJECT_PTR(args[0]);
    
    if ((recv_obj->flags & 0xF0) != EZOM_TYPE_STRING || 
        (arg_obj->flags & 0xF0) != EZOM_TYPE_STRING) {
        printf("Type error in string concatenation\n");
        return g_nil;
    }
    
    printf("DEBUG: Type check passed, accessing string data\n");
    
    ezom_string_t* str1 = (ezom_string_t*)EZOM_OBJECT_PTR(receiver);
    ezom_string_t* str2 = (ezom_string_t*)EZOM_OBJECT_PTR(args[0]);
    
    printf("DEBUG: str1 length: %d, str2 length: %d\n", str1->length, str2->length);
    
    uint16_t new_length = str1->length + str2->length;
    printf("DEBUG: About to allocate %lu bytes\n", (unsigned long)(sizeof(ezom_string_t) + new_length + 1));
    
    uint24_t result = ezom_allocate(sizeof(ezom_string_t) + new_length + 1);
    
    if (!result) {
        printf("DEBUG: Allocation failed\n");
        return g_nil;
    }
    
    printf("DEBUG: Allocated string at 0x%06X, about to init object\n", result);
    
    ezom_init_object(result, g_string_class, EZOM_TYPE_STRING);
    
    printf("DEBUG: Object initialized, setting up result string\n");
    
    printf("DEBUG: Object initialized, setting up result string\n");
    
    ezom_string_t* result_str = (ezom_string_t*)EZOM_OBJECT_PTR(result);
    result_str->length = new_length;
    
    printf("DEBUG: About to copy string data\n");
    
    memcpy(result_str->data, str1->data, str1->length);
    memcpy(result_str->data + str1->length, str2->data, str2->length);
    result_str->data[new_length] = '\0';
    
    printf("DEBUG: String concatenation successful, returning result\n");
    
    return result;
}

// String>>=
uint24_t prim_string_equals(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_false;
    
    ezom_string_t* str1 = (ezom_string_t*)EZOM_OBJECT_PTR(receiver);
    ezom_string_t* str2 = (ezom_string_t*)EZOM_OBJECT_PTR(args[0]);
    
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
    
    ezom_integer_t* size_obj = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
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
    
    ezom_array_t* array = (ezom_array_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* index_obj = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
    
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
    
    ezom_array_t* array = (ezom_array_t*)EZOM_OBJECT_PTR(receiver);
    ezom_integer_t* index_obj = (ezom_integer_t*)EZOM_OBJECT_PTR(args[0]);
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
    
    ezom_array_t* array = (ezom_array_t*)EZOM_OBJECT_PTR(receiver);
    return ezom_create_integer(array->size);
}

// ============================================================================
// BOOLEAN PRIMITIVES (PHASE 4.3.1 ENHANCED)
// ============================================================================

// True>>ifTrue:
uint24_t prim_true_if_true(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !args || !args[0]) {
        return g_nil;
    }
    
    // Verify receiver is true
    if (receiver != g_true) {
        return g_nil;
    }
    
    // Check if argument is a block
    if (!ezom_is_block(args[0])) {
        printf("Type error: ifTrue: sent with non-block argument\n");
        return g_nil;
    }
    
    // Evaluate the true block and return its result
    return g_primitives[PRIM_BLOCK_VALUE](args[0], NULL, 0);
}

// True>>ifFalse:
uint24_t prim_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !args || !args[0]) {
        return g_nil;
    }
    
    // Verify receiver is true
    if (receiver != g_true) {
        return g_nil;
    }
    
    // Check if argument is a block (for consistency)
    if (!ezom_is_block(args[0])) {
        printf("Type error: ifFalse: sent with non-block argument\n");
        return g_nil;
    }
    
    // True object ignores false block - return nil (block not executed)
    return g_nil;
}

// True>>ifTrue:ifFalse:
uint24_t prim_true_if_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2 || !args || !args[0] || !args[1]) {
        return g_nil;
    }
    
    // Verify receiver is true
    if (receiver != g_true) {
        return g_nil;
    }
    
    // Check if both arguments are blocks
    if (!ezom_is_block(args[0]) || !ezom_is_block(args[1])) {
        printf("Type error: ifTrue:ifFalse: sent with non-block argument(s)\n");
        return g_nil;
    }
    
    // Evaluate the true block (first argument) and return its result
    return g_primitives[PRIM_BLOCK_VALUE](args[0], NULL, 0);
}

// False>>ifTrue:
uint24_t prim_false_if_true(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !args || !args[0]) {
        return g_nil;
    }
    
    // Verify receiver is false
    if (receiver != g_false) {
        return g_nil;
    }
    
    // Check if argument is a block (for consistency)
    if (!ezom_is_block(args[0])) {
        printf("Type error: ifTrue: sent with non-block argument\n");
        return g_nil;
    }
    
    // False object ignores true block - return nil (block not executed)
    return g_nil;
}

// False>>ifFalse:
uint24_t prim_false_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1 || !args || !args[0]) {
        return g_nil;
    }
    
    // Verify receiver is false
    if (receiver != g_false) {
        return g_nil;
    }
    
    // Check if argument is a block
    if (!ezom_is_block(args[0])) {
        printf("Type error: ifFalse: sent with non-block argument\n");
        return g_nil;
    }
    
    // Evaluate the false block and return its result
    return g_primitives[PRIM_BLOCK_VALUE](args[0], NULL, 0);
}

// False>>ifTrue:ifFalse:
uint24_t prim_false_if_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2 || !args || !args[0] || !args[1]) {
        return g_nil;
    }
    
    // Verify receiver is false
    if (receiver != g_false) {
        return g_nil;
    }
    
    // Check if both arguments are blocks
    if (!ezom_is_block(args[0]) || !ezom_is_block(args[1])) {
        printf("Type error: ifTrue:ifFalse: sent with non-block argument(s)\n");
        return g_nil;
    }
    
    // Evaluate the false block (second argument) and return its result
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
    
    ezom_block_t* block = (ezom_block_t*)EZOM_OBJECT_PTR(receiver);
    
    if (block->param_count != 0) {
        printf("Block expects %d parameters, got 0\n", block->param_count);
        return g_nil;
    }
    
    // Execute block with no parameters
    return ezom_block_evaluate(receiver, NULL, 0);
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
    
    ezom_block_t* block = (ezom_block_t*)EZOM_OBJECT_PTR(receiver);
    
    if (block->param_count != 1) {
        printf("Block expects %d parameters, got 1\n", block->param_count);
        return g_nil;
    }
    
    // Execute block with one parameter
    return ezom_block_evaluate(receiver, args, 1);
}

// Block>>whileTrue:
uint24_t prim_block_while_true(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_block(receiver)) {
        printf("Type error: whileTrue: sent to non-block\n");
        return g_nil;
    }
    
    if (arg_count != 1 || !ezom_is_block(args[0])) {
        printf("Type error: whileTrue: expects one block argument\n");
        return g_nil;
    }
    
    ezom_block_t* condition_block = (ezom_block_t*)EZOM_OBJECT_PTR(receiver);
    uint24_t body_block = args[0];
    
    if (condition_block->param_count != 0) {
        printf("Condition block must have no parameters\n");
        return g_nil;
    }
    
    // Execute while loop: [ condition ] whileTrue: [ body ]
    while (true) {
        // Evaluate condition block
        uint24_t result = ezom_block_evaluate(receiver, NULL, 0);
        
        // Check if result is true
        if (result == g_true) {
            // Execute body block
            g_primitives[PRIM_BLOCK_VALUE](body_block, NULL, 0);
        } else {
            break;
        }
    }
    
    return g_nil;
}

// Block>>whileFalse:
uint24_t prim_block_while_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (!ezom_is_block(receiver)) {
        printf("Type error: whileFalse: sent to non-block\n");
        return g_nil;
    }
    
    if (arg_count != 1 || !ezom_is_block(args[0])) {
        printf("Type error: whileFalse: expects one block argument\n");
        return g_nil;
    }
    
    ezom_block_t* condition_block = (ezom_block_t*)EZOM_OBJECT_PTR(receiver);
    uint24_t body_block = args[0];
    
    if (condition_block->param_count != 0) {
        printf("Condition block must have no parameters\n");
        return g_nil;
    }
    
    // Execute while loop: [ condition ] whileFalse: [ body ]
    while (true) {
        // Evaluate condition block
        uint24_t result = ezom_block_evaluate(receiver, NULL, 0);
        
        // Check if result is false
        if (result == g_false) {
            // Execute body block
            g_primitives[PRIM_BLOCK_VALUE](body_block, NULL, 0);
        } else {
            break;
        }
    }
    
    return g_nil;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool ezom_is_integer(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    return (object->flags & 0xF0) == EZOM_TYPE_INTEGER;
}

bool ezom_is_string(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    return (object->flags & 0xF0) == EZOM_TYPE_STRING;
}

bool ezom_is_array(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    return (object->flags & 0xF0) == EZOM_TYPE_ARRAY;
}

bool ezom_is_block(uint24_t obj) {
    if (!obj || !ezom_is_valid_object(obj)) return false;
    ezom_object_t* object = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    return (object->flags & 0xF0) == EZOM_TYPE_BLOCK;
}

bool ezom_is_boolean(uint24_t obj) {
    return (obj == g_true || obj == g_false);
}