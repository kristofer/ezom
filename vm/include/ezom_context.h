// ============================================================================
// File: include/ezom_context.h
// Execution context and block object definitions
// ============================================================================

#pragma once
#include "ezom_object.h"
#include "ezom_ast.h"
#include <stdint.h>
#include <stdbool.h>

// Use definitions from ezom_object.h for consistency
// ezom_block_t and ezom_context_t are defined there

// Boolean objects (singletons)
typedef struct ezom_boolean {
    ezom_object_t header;
    bool          value;
} ezom_boolean_t;

// Global context and object references
// (Most are now defined in ezom_object.h)
extern uint24_t g_current_context;

// Context management functions
void ezom_init_context_system(void);
uint24_t ezom_create_block_context(uint24_t outer_context, uint24_t block, uint8_t local_count);
void ezom_context_set_local(uint24_t context_ptr, uint8_t index, uint24_t value);
uint24_t ezom_context_get_local(uint24_t context_ptr, uint8_t index);
uint24_t ezom_context_lookup_variable(uint24_t context_ptr, const char* name);
void ezom_context_bind_parameters(uint24_t context_ptr, uint24_t* args, uint8_t arg_count);

// Block object functions (Phase 2 extensions)
uint24_t ezom_create_ast_block(ezom_ast_node_t* ast_node, uint24_t outer_context);
uint24_t ezom_block_evaluate(uint24_t block_ptr, uint24_t* args, uint8_t arg_count);

// Boolean object functions
void ezom_init_boolean_objects(void);
uint24_t ezom_create_boolean(bool value);
bool ezom_is_true_object(uint24_t object_ptr);
bool ezom_is_false_object(uint24_t object_ptr);

// Context stack management
void ezom_push_context(uint24_t context_ptr);
uint24_t ezom_pop_context(void);
uint24_t ezom_get_current_context(void);

// Utility functions
bool ezom_is_block_object(uint24_t object_ptr);
bool ezom_is_context_object(uint24_t object_ptr);