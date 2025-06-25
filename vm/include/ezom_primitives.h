// ============================================================================
// File: src/include/ezom_primitives.h
// Primitive function support
// ============================================================================

#pragma once
#include <stdint.h>

typedef uint24_t (*ezom_primitive_fn)(uint24_t receiver, uint24_t* args, uint8_t arg_count);

// Primitive numbers
#define PRIM_OBJECT_CLASS       1
#define PRIM_OBJECT_EQUALS      2
#define PRIM_OBJECT_HASH        3
#define PRIM_OBJECT_PRINTLN     4

#define PRIM_INTEGER_ADD        10
#define PRIM_INTEGER_SUB        11
#define PRIM_INTEGER_MUL        12
#define PRIM_INTEGER_DIV        13
#define PRIM_INTEGER_LT         14
#define PRIM_INTEGER_GT         15
#define PRIM_INTEGER_EQ         16

#define PRIM_STRING_LENGTH      20
#define PRIM_STRING_AT          21
#define PRIM_STRING_CONCAT      22
#define PRIM_STRING_EQUALS      23

#define MAX_PRIMITIVES          64

// Primitive function table
extern ezom_primitive_fn g_primitives[MAX_PRIMITIVES];

// Initialize primitives
void ezom_init_primitives(void);