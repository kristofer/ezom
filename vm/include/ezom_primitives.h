// ============================================================================
// File: src/include/ezom_primitives.h
// Primitive function support
// ============================================================================

#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint24_t (*ezom_primitive_fn)(uint24_t receiver, uint24_t* args, uint8_t arg_count);

// Primitive numbers
// Object primitives (ENHANCED)
#define PRIM_OBJECT_CLASS       1
#define PRIM_OBJECT_EQUALS      2
#define PRIM_OBJECT_HASH        3
#define PRIM_OBJECT_PRINTLN     4
#define PRIM_OBJECT_IS_NIL      5   // NEW
#define PRIM_OBJECT_NOT_NIL     6   // NEW

// Integer primitives (ENHANCED)
#define PRIM_INTEGER_ADD        10
#define PRIM_INTEGER_SUB        11
#define PRIM_INTEGER_MUL        12
#define PRIM_INTEGER_DIV        13
#define PRIM_INTEGER_MOD        14  // NEW
#define PRIM_INTEGER_LT         15
#define PRIM_INTEGER_GT         16
#define PRIM_INTEGER_LTE        17  // NEW
#define PRIM_INTEGER_GTE        18  // NEW
#define PRIM_INTEGER_EQ         19
#define PRIM_INTEGER_NEQ        20  // NEW
#define PRIM_INTEGER_AS_STRING  21  // NEW
#define PRIM_INTEGER_ABS        23  // NEW
#define PRIM_INTEGER_TO_DO      24  // NEW
#define PRIM_INTEGER_TIMES_REPEAT 25 // NEW

// String primitives
#define PRIM_STRING_LENGTH      30
#define PRIM_STRING_AT          31
#define PRIM_STRING_CONCAT      32
#define PRIM_STRING_EQUALS      33

// Array primitives (NEW)
#define PRIM_ARRAY_NEW          40
#define PRIM_ARRAY_AT           41
#define PRIM_ARRAY_AT_PUT       42
#define PRIM_ARRAY_LENGTH       43

// Boolean primitives (NEW)
#define PRIM_TRUE_IF_TRUE       50
#define PRIM_TRUE_IF_FALSE      51
#define PRIM_TRUE_IF_TRUE_IF_FALSE 52
#define PRIM_FALSE_IF_TRUE      53
#define PRIM_FALSE_IF_FALSE     54
#define PRIM_FALSE_IF_TRUE_IF_FALSE 55
#define PRIM_BOOLEAN_NOT        56

// Block primitives (NEW)
#define PRIM_BLOCK_VALUE        60
#define PRIM_BLOCK_VALUE_WITH   61

#define MAX_PRIMITIVES          80

// Primitive function table
extern ezom_primitive_fn g_primitives[MAX_PRIMITIVES];

// Initialize primitives
void ezom_init_primitives(void);

// NEW: Utility functions for primitives
bool ezom_is_integer(uint24_t obj);
bool ezom_is_string(uint24_t obj);
bool ezom_is_array(uint24_t obj);
bool ezom_is_block(uint24_t obj);
bool ezom_is_boolean(uint24_t obj);