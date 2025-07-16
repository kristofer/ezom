// ============================================================================
// File: include/ezom_ast_memory.h
// Simple memory pool for AST nodes
// ============================================================================

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include "ezom_ast.h"

// AST memory pool functions
void ezom_ast_memory_init(void);
void* ezom_ast_alloc(size_t size);
char* ezom_ast_strdup(const char* str);
ezom_ast_node_t* ezom_ast_create_node(ezom_ast_type_t type);
void ezom_ast_memory_reset(void);
void ezom_ast_memory_stats(void);
void ezom_ast_print_stats(void);  // Alias for ezom_ast_memory_stats
