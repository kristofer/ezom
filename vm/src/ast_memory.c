// ============================================================================
// File: src/ast_memory.c
// Simple memory pool for AST nodes to avoid malloc conflicts
// ============================================================================

#include "../include/ezom_ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple memory pool for AST nodes
#define AST_POOL_SIZE 8192  // 8KB pool
#define MAX_AST_NODES 256   // Maximum number of AST nodes

static char ast_memory_pool[AST_POOL_SIZE];
static size_t ast_pool_offset = 0;
static bool ast_pool_initialized = false;

// Simple node tracking for cleanup
static ezom_ast_node_t* ast_node_list[MAX_AST_NODES];
static size_t ast_node_count = 0;

void ezom_ast_memory_init() {
    if (!ast_pool_initialized) {
        ast_pool_offset = 0;
        ast_node_count = 0;
        memset(ast_node_list, 0, sizeof(ast_node_list));
        ast_pool_initialized = true;
        printf("DEBUG: AST memory pool initialized (%d bytes)\n", AST_POOL_SIZE);
    }
}

void* ezom_ast_alloc(size_t size) {
    if (!ast_pool_initialized) {
        ezom_ast_memory_init();
    }
    
    // Align to 8-byte boundary
    size = (size + 7) & ~7;
    
    if (ast_pool_offset + size > AST_POOL_SIZE) {
        printf("ERROR: AST memory pool exhausted (requested %zu bytes, %zu available)\n", 
               size, AST_POOL_SIZE - ast_pool_offset);
        return NULL;
    }
    
    void* ptr = &ast_memory_pool[ast_pool_offset];
    ast_pool_offset += size;
    
    printf("DEBUG: AST alloc %zu bytes at offset %zu\n", size, ast_pool_offset - size);
    return ptr;
}

char* ezom_ast_strdup(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char* copy = (char*)ezom_ast_alloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

ezom_ast_node_t* ezom_ast_create_node(ezom_ast_type_t type) {
    if (ast_node_count >= MAX_AST_NODES) {
        printf("ERROR: Maximum AST nodes exceeded\n");
        return NULL;
    }
    
    ezom_ast_node_t* node = (ezom_ast_node_t*)ezom_ast_alloc(sizeof(ezom_ast_node_t));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(ezom_ast_node_t));
    node->type = type;
    
    // Track the node for cleanup
    ast_node_list[ast_node_count++] = node;
    
    printf("DEBUG: Created AST node type %d (total: %zu)\n", type, ast_node_count);
    return node;
}

void ezom_ast_memory_reset() {
    printf("DEBUG: Resetting AST memory pool\n");
    ast_pool_offset = 0;
    ast_node_count = 0;
    memset(ast_node_list, 0, sizeof(ast_node_list));
}

void ezom_ast_memory_stats() {
    printf("AST Memory Pool Stats:\n");
    printf("  Pool size: %d bytes\n", AST_POOL_SIZE);
    printf("  Used: %zu bytes (%.1f%%)\n", ast_pool_offset, 
           (float)ast_pool_offset / AST_POOL_SIZE * 100.0f);
    printf("  Available: %zu bytes\n", AST_POOL_SIZE - ast_pool_offset);
    printf("  Nodes created: %zu/%d\n", ast_node_count, MAX_AST_NODES);
}

// Alias for the stats function
void ezom_ast_print_stats() {
    ezom_ast_memory_stats();
}
