// Debug test for AST evaluation crash
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock definitions for testing
typedef struct {
    void* class_ptr;
    unsigned short hash;
    unsigned char flags;
} ezom_object_t;

typedef struct {
    ezom_object_t header;
    short value;
} ezom_integer_t;

typedef struct {
    ezom_object_t header;
    unsigned short length;
    char data[];
} ezom_string_t;

// Mock AST types
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

// Mock evaluation result
typedef struct {
    void* value;
    int is_error;
    int is_return;
    char error_msg[256];
} ezom_eval_result_t;

// Mock globals
void* g_nil = NULL;

// Mock functions
void* ezom_create_integer(short value) {
    ezom_integer_t* obj = malloc(sizeof(ezom_integer_t));
    if (!obj) return NULL;
    obj->value = value;
    return obj;
}

void* ezom_create_string(const char* data, int length) {
    ezom_string_t* obj = malloc(sizeof(ezom_string_t) + length + 1);
    if (!obj) return NULL;
    obj->length = length;
    memcpy(obj->data, data, length);
    obj->data[length] = '\0';
    return obj;
}

ezom_ast_node_t* ezom_ast_create(ezom_ast_type_t type) {
    ezom_ast_node_t* node = malloc(sizeof(ezom_ast_node_t));
    if (!node) return NULL;
    memset(node, 0, sizeof(ezom_ast_node_t));
    node->type = type;
    return node;
}

ezom_ast_node_t* ezom_ast_create_literal_integer(short value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_LITERAL);
    if (!node) return NULL;
    
    node->data.literal.type = LITERAL_INTEGER;
    node->data.literal.value.integer_value = value;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_literal_string(const char* value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_LITERAL);
    if (!node) return NULL;
    if (!value) {
        free(node);
        return NULL;
    }
    
    node->data.literal.type = LITERAL_STRING;
    node->data.literal.value.string_value = strdup(value);
    if (!node->data.literal.value.string_value) {
        free(node);
        return NULL;
    }
    
    return node;
}

void ezom_ast_free(ezom_ast_node_t* node) {
    if (!node) return;
    
    if (node->type == AST_LITERAL && 
        node->data.literal.type == LITERAL_STRING) {
        free(node->data.literal.value.string_value);
    }
    
    free(node);
}

ezom_eval_result_t ezom_make_result(void* value) {
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

ezom_eval_result_t ezom_evaluate_literal(ezom_ast_node_t* node, void* context) {
    printf("  Evaluating literal node: %p\n", (void*)node);
    
    if (!node || node->type != AST_LITERAL) {
        printf("  ERROR: Invalid literal node\n");
        return ezom_make_error_result("Invalid literal node");
    }
    
    printf("  Literal type: %d\n", node->data.literal.type);
    
    switch (node->data.literal.type) {
        case LITERAL_INTEGER:
            {
                printf("  Creating integer: %d\n", node->data.literal.value.integer_value);
                void* int_obj = ezom_create_integer(node->data.literal.value.integer_value);
                printf("  Integer created: %p\n", int_obj);
                return ezom_make_result(int_obj);
            }
            
        case LITERAL_STRING:
            {
                const char* str_val = node->data.literal.value.string_value;
                printf("  Creating string: '%s'\n", str_val ? str_val : "NULL");
                if (!str_val) {
                    return ezom_make_error_result("NULL string value");
                }
                void* str_obj = ezom_create_string(str_val, strlen(str_val));
                printf("  String created: %p\n", str_obj);
                return ezom_make_result(str_obj);
            }
            
        default:
            printf("  ERROR: Unknown literal type: %d\n", node->data.literal.type);
            return ezom_make_error_result("Unknown literal type");
    }
}

ezom_eval_result_t ezom_evaluate_ast(ezom_ast_node_t* node, void* context) {
    printf("Evaluating AST node: %p\n", (void*)node);
    
    if (!node) {
        printf("NULL node, returning nil\n");
        return ezom_make_result(g_nil);
    }
    
    printf("Node type: %d\n", node->type);
    
    switch (node->type) {
        case AST_LITERAL:
            return ezom_evaluate_literal(node, context);
            
        default:
            printf("ERROR: Unknown AST node type: %d\n", node->type);
            return ezom_make_error_result("Unknown AST node type");
    }
}

int main() {
    printf("Debug test for AST evaluation\n");
    printf("==============================\n");
    
    // Test 1: Integer literal
    printf("\n1. Testing integer literal creation:\n");
    ezom_ast_node_t* int_ast = ezom_ast_create_literal_integer(123);
    printf("   AST created: %p\n", (void*)int_ast);
    
    if (int_ast) {
        printf("   AST type: %d\n", int_ast->type);
        printf("   Literal type: %d\n", int_ast->data.literal.type);
        printf("   Integer value: %d\n", int_ast->data.literal.value.integer_value);
        
        ezom_eval_result_t result = ezom_evaluate_ast(int_ast, NULL);
        if (!result.is_error && result.value) {
            ezom_integer_t* int_obj = (ezom_integer_t*)result.value;
            printf("   Result: %d\n", int_obj->value);
        } else {
            printf("   ERROR: %s\n", result.error_msg);
        }
        ezom_ast_free(int_ast);
    }
    
    // Test 2: String literal
    printf("\n2. Testing string literal creation:\n");
    ezom_ast_node_t* str_ast = ezom_ast_create_literal_string("test");
    printf("   AST created: %p\n", (void*)str_ast);
    
    if (str_ast) {
        printf("   AST type: %d\n", str_ast->type);
        printf("   Literal type: %d\n", str_ast->data.literal.type);
        printf("   String value: '%s'\n", str_ast->data.literal.value.string_value);
        
        ezom_eval_result_t result = ezom_evaluate_ast(str_ast, NULL);
        if (!result.is_error && result.value) {
            ezom_string_t* str_obj = (ezom_string_t*)result.value;
            printf("   Result: '%.*s'\n", str_obj->length, str_obj->data);
        } else {
            printf("   ERROR: %s\n", result.error_msg);
        }
        ezom_ast_free(str_ast);
    }
    
    printf("\nDebug test complete!\n");
    return 0;
}
