// Minimal test for AST evaluation crash
// This focuses on the exact issue that was causing the crash

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Simulate the exact types and functions from EZOM VM
typedef uint32_t uint24_t; // Use 32-bit for testing

// Mock globals that were causing the crash
uint24_t g_nil = 0;         // This was the problem - being used before initialization
uint24_t g_integer_class = 0;
uint24_t g_string_class = 0;

// Mock AST types (simplified)
typedef enum {
    AST_LITERAL = 1
} ezom_ast_type_t;

typedef enum {
    LITERAL_INTEGER = 1,
    LITERAL_STRING = 2
} literal_type_t;

typedef struct ezom_ast_node {
    ezom_ast_type_t type;
    struct ezom_ast_node* next;
    union {
        struct {
            literal_type_t type;
            union {
                short integer_value;
                char* string_value;
            } value;
        } literal;
    } data;
} ezom_ast_node_t;

typedef struct {
    uint24_t value;
    int is_error;
    int is_return;
    char error_msg[256];
} ezom_eval_result_t;

// Mock object creation functions
uint24_t ezom_create_integer(short value) {
    if (g_integer_class == 0) {
        printf("ERROR: g_integer_class is NULL!\n");
        return 0;
    }
    printf("Creating integer %d (class=0x%08X)\n", value, g_integer_class);
    return 0x100000 + value; // Mock object pointer
}

// Mock AST functions
ezom_ast_node_t* ezom_ast_create_literal_integer(short value) {
    ezom_ast_node_t* node = malloc(sizeof(ezom_ast_node_t));
    if (!node) return NULL;
    memset(node, 0, sizeof(ezom_ast_node_t));
    node->type = AST_LITERAL;
    node->data.literal.type = LITERAL_INTEGER;
    node->data.literal.value.integer_value = value;
    return node;
}

void ezom_ast_free(ezom_ast_node_t* node) {
    if (node) free(node);
}

// Mock evaluation functions
ezom_eval_result_t ezom_make_result(uint24_t value) {
    ezom_eval_result_t result;
    result.value = value;
    result.is_return = 0;
    result.is_error = 0;
    result.error_msg[0] = '\0';
    return result;
}

ezom_eval_result_t ezom_make_error_result(const char* message) {
    ezom_eval_result_t result;
    result.value = g_nil;
    result.is_return = 0;
    result.is_error = 1;
    strncpy(result.error_msg, message, sizeof(result.error_msg) - 1);
    result.error_msg[sizeof(result.error_msg) - 1] = '\0';
    return result;
}

ezom_eval_result_t ezom_evaluate_literal(ezom_ast_node_t* node, uint24_t context) {
    printf("  Evaluating literal node: %p\n", (void*)node);
    
    if (!node || node->type != AST_LITERAL) {
        return ezom_make_error_result("Invalid literal node");
    }
    
    switch (node->data.literal.type) {
        case LITERAL_INTEGER:
            {
                short value = node->data.literal.value.integer_value;
                printf("  Creating integer object for value: %d\n", value);
                uint24_t int_obj = ezom_create_integer(value);
                if (int_obj == 0) {
                    return ezom_make_error_result("Failed to create integer object");
                }
                return ezom_make_result(int_obj);
            }
            
        default:
            return ezom_make_error_result("Unknown literal type");
    }
}

ezom_eval_result_t ezom_evaluate_ast(ezom_ast_node_t* node, uint24_t context) {
    printf("Evaluating AST node: %p\n", (void*)node);
    
    if (!node) {
        return ezom_make_result(g_nil);
    }
    
    switch (node->type) {
        case AST_LITERAL:
            return ezom_evaluate_literal(node, context);
            
        default:
            return ezom_make_error_result("Unknown AST node type");
    }
}

int main() {
    printf("EZOM AST Evaluation Crash Test\n");
    printf("===============================\n");
    
    // Simulate the crash scenario
    printf("\n1. Testing with uninitialized globals (crash scenario):\n");
    g_nil = 0;
    g_integer_class = 0;
    
    ezom_ast_node_t* ast = ezom_ast_create_literal_integer(42);
    if (ast) {
        ezom_eval_result_t result = ezom_evaluate_ast(ast, 0);
        printf("   Result: is_error=%d, error='%s'\n", result.is_error, result.error_msg);
        ezom_ast_free(ast);
    }
    
    printf("\n2. Testing with initialized globals (fixed scenario):\n");
    g_nil = 1;              // Non-zero safe value
    g_integer_class = 0x1000; // Mock class pointer
    
    ast = ezom_ast_create_literal_integer(42);
    if (ast) {
        ezom_eval_result_t result = ezom_evaluate_ast(ast, 0);
        if (result.is_error) {
            printf("   Error: %s\n", result.error_msg);
        } else {
            printf("   Success: Created object at 0x%08X\n", result.value);
        }
        ezom_ast_free(ast);
    }
    
    printf("\nTest complete!\n");
    return 0;
}
