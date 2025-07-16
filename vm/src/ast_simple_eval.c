// ============================================================================
// File: src/ast_simple_eval.c
// Simple AST evaluation for testing (uses EZOM object system)
// ============================================================================

#include "../include/ezom_ast.h"
#include "../include/ezom_ast_memory.h"
#include "../include/ezom_object.h"
#include "../include/ezom_dispatch.h"
#include <stdio.h>
#include <string.h>

// Simple AST evaluation function
uint24_t ezom_ast_simple_evaluate(ezom_ast_node_t* node) {
    if (!node) {
        printf("ERROR: NULL AST node\n");
        return 0;
    }
    
    printf("DEBUG: Evaluating AST node type %d\n", node->type);
    
    switch (node->type) {
        case AST_LITERAL:
            switch (node->data.literal.type) {
                case LITERAL_INTEGER:
                    printf("DEBUG: Evaluating integer literal: %d\n", node->data.literal.value.integer_value);
                    return ezom_create_integer(node->data.literal.value.integer_value);
                    
                case LITERAL_STRING:
                    printf("DEBUG: Evaluating string literal: '%s'\n", node->data.literal.value.string_value);
                    return ezom_create_string(node->data.literal.value.string_value, 
                                            strlen(node->data.literal.value.string_value));
                    
                default:
                    printf("ERROR: Unknown literal type\n");
                    return 0;
            }
            break;
            
        case AST_BINARY_MESSAGE: {
            printf("DEBUG: Evaluating binary message: %s\n", node->data.message_send.selector);
            
            // Evaluate receiver
            uint24_t receiver = ezom_ast_simple_evaluate(node->data.message_send.receiver);
            if (!receiver) {
                printf("ERROR: Failed to evaluate receiver\n");
                return 0;
            }
            
            // For binary messages, there should be exactly one argument
            if (!node->data.message_send.arguments) {
                printf("ERROR: Binary message missing argument\n");
                return 0;
            }
            
            // Evaluate argument
            uint24_t argument = ezom_ast_simple_evaluate(node->data.message_send.arguments);
            if (!argument) {
                printf("ERROR: Failed to evaluate argument\n");
                return 0;
            }
            
            // Create selector symbol
            uint24_t selector = ezom_create_symbol(node->data.message_send.selector, 
                                                 strlen(node->data.message_send.selector));
            if (!selector) {
                printf("ERROR: Failed to create selector symbol\n");
                return 0;
            }
            
            printf("DEBUG: Sending message %s to receiver 0x%06X with argument 0x%06X\n",
                   node->data.message_send.selector, receiver, argument);
            
            // Send the message
            return ezom_send_binary_message(receiver, selector, argument);
        }
        
        default:
            printf("ERROR: Unsupported AST node type %d\n", node->type);
            return 0;
    }
}

// Test function to create and evaluate a simple AST
uint24_t ezom_ast_test_simple_expression() {
    printf("DEBUG: Testing simple AST expression: 5 + 3\n");
    
    // Initialize AST memory pool
    ezom_ast_memory_init();
    
    // Create AST nodes: 5 + 3
    ezom_ast_node_t* five = ezom_ast_create_integer_literal(5);
    ezom_ast_node_t* three = ezom_ast_create_integer_literal(3);
    ezom_ast_node_t* expr = ezom_ast_create_binary_message(five, "+", three);
    
    if (!five || !three || !expr) {
        printf("ERROR: Failed to create AST nodes\n");
        return 0;
    }
    
    printf("DEBUG: AST created successfully\n");
    ezom_ast_memory_stats();
    
    // Evaluate the expression
    uint24_t result = ezom_ast_simple_evaluate(expr);
    
    if (result) {
        ezom_integer_t* res = (ezom_integer_t*)EZOM_OBJECT_PTR(result);
        printf("DEBUG: AST evaluation result: %d\n", res->value);
    } else {
        printf("ERROR: AST evaluation failed\n");
    }
    
    return result;
}

// Test nested arithmetic: (10 + 5) * 2 = 30
uint24_t ezom_ast_test_nested_arithmetic() {
    printf("DEBUG: Testing nested arithmetic: (10 + 5) * 2\n");
    
    // Create AST nodes for: (10 + 5) * 2
    ezom_ast_node_t* ten = ezom_ast_create_integer_literal(10);
    ezom_ast_node_t* five = ezom_ast_create_integer_literal(5);
    ezom_ast_node_t* two = ezom_ast_create_integer_literal(2);
    
    if (!ten || !five || !two) {
        printf("ERROR: Failed to create integer literals\n");
        return 0;
    }
    
    // Create (10 + 5) 
    ezom_ast_node_t* add_expr = ezom_ast_create_binary_message(ten, "+", five);
    if (!add_expr) {
        printf("ERROR: Failed to create addition expression\n");
        return 0;
    }
    
    // Create (10 + 5) * 2
    ezom_ast_node_t* mult_expr = ezom_ast_create_binary_message(add_expr, "*", two);
    if (!mult_expr) {
        printf("ERROR: Failed to create multiplication expression\n");
        return 0;
    }
    
    printf("DEBUG: Nested AST created successfully\n");
    ezom_ast_memory_stats();
    
    uint24_t result = ezom_ast_simple_evaluate(mult_expr);
    if (result) {
        ezom_integer_t* res = (ezom_integer_t*)EZOM_OBJECT_PTR(result);
        printf("DEBUG: Nested evaluation result: %d\n", res->value);
    }
    
    return result;
}

// Test chain operations: 20 - 5 + 3 = 18
uint24_t ezom_ast_test_chain_operations() {
    printf("DEBUG: Testing chain operations: 20 - 5 + 3\n");
    
    ezom_ast_node_t* twenty = ezom_ast_create_integer_literal(20);
    ezom_ast_node_t* five = ezom_ast_create_integer_literal(5);
    ezom_ast_node_t* three = ezom_ast_create_integer_literal(3);
    
    if (!twenty || !five || !three) {
        printf("ERROR: Failed to create integer literals\n");
        return 0;
    }
    
    // Create 20 - 5
    ezom_ast_node_t* sub_expr = ezom_ast_create_binary_message(twenty, "-", five);
    if (!sub_expr) {
        printf("ERROR: Failed to create subtraction expression\n");
        return 0;
    }
    
    // Create (20 - 5) + 3
    ezom_ast_node_t* add_expr = ezom_ast_create_binary_message(sub_expr, "+", three);
    if (!add_expr) {
        printf("ERROR: Failed to create addition expression\n");
        return 0;
    }
    
    printf("DEBUG: Chain AST created successfully\n");
    ezom_ast_memory_stats();
    
    uint24_t result = ezom_ast_simple_evaluate(add_expr);
    if (result) {
        ezom_integer_t* res = (ezom_integer_t*)EZOM_OBJECT_PTR(result);
        printf("DEBUG: Chain evaluation result: %d\n", res->value);
    }
    
    return result;
}

// Test mixed operations: 100 / 4 + 15 * 2 = 25 + 30 = 55
uint24_t ezom_ast_test_mixed_operations() {
    printf("DEBUG: Testing mixed operations: 100 / 4 + 15 * 2\n");
    
    ezom_ast_node_t* hundred = ezom_ast_create_integer_literal(100);
    ezom_ast_node_t* four = ezom_ast_create_integer_literal(4);
    ezom_ast_node_t* fifteen = ezom_ast_create_integer_literal(15);
    ezom_ast_node_t* two = ezom_ast_create_integer_literal(2);
    
    if (!hundred || !four || !fifteen || !two) {
        printf("ERROR: Failed to create integer literals\n");
        return 0;
    }
    
    // Create 100 / 4
    ezom_ast_node_t* div_expr = ezom_ast_create_binary_message(hundred, "/", four);
    if (!div_expr) {
        printf("ERROR: Failed to create division expression\n");
        return 0;
    }
    
    // Create 15 * 2
    ezom_ast_node_t* mult_expr = ezom_ast_create_binary_message(fifteen, "*", two);
    if (!mult_expr) {
        printf("ERROR: Failed to create multiplication expression\n");
        return 0;
    }
    
    // Create (100 / 4) + (15 * 2)
    ezom_ast_node_t* final_expr = ezom_ast_create_binary_message(div_expr, "+", mult_expr);
    if (!final_expr) {
        printf("ERROR: Failed to create final addition expression\n");
        return 0;
    }
    
    printf("DEBUG: Mixed operations AST created successfully\n");
    ezom_ast_memory_stats();
    
    uint24_t result = ezom_ast_simple_evaluate(final_expr);
    if (result) {
        ezom_integer_t* res = (ezom_integer_t*)EZOM_OBJECT_PTR(result);
        printf("DEBUG: Mixed operations result: %d\n", res->value);
    }
    
    return result;
}

// Test comparison chain: (5 < 10) = true
uint24_t ezom_ast_test_comparison_chain() {
    printf("DEBUG: Testing comparison chain: (5 < 10) = true\n");
    
    ezom_ast_node_t* five = ezom_ast_create_integer_literal(5);
    ezom_ast_node_t* ten = ezom_ast_create_integer_literal(10);
    
    if (!five || !ten) {
        printf("ERROR: Failed to create integer literals\n");
        return 0;
    }
    
    // Create 5 < 10
    ezom_ast_node_t* less_expr = ezom_ast_create_binary_message(five, "<", ten);
    if (!less_expr) {
        printf("ERROR: Failed to create less-than expression\n");
        return 0;
    }
    
    printf("DEBUG: Comparison AST created successfully\n");
    ezom_ast_memory_stats();
    
    uint24_t result = ezom_ast_simple_evaluate(less_expr);
    if (result) {
        if (result == g_true) {
            printf("DEBUG: Comparison result: true\n");
        } else if (result == g_false) {
            printf("DEBUG: Comparison result: false\n");
        } else {
            printf("DEBUG: Comparison result: unknown\n");
        }
    }
    
    return result;
}

// Test deep nesting: ((8 + 2) * 3) - (5 + 1) = (10 * 3) - 6 = 30 - 6 = 24
uint24_t ezom_ast_test_deep_nesting() {
    printf("DEBUG: Testing deep nesting: ((8 + 2) * 3) - (5 + 1)\n");
    
    // Left side: (8 + 2) * 3
    ezom_ast_node_t* eight = ezom_ast_create_integer_literal(8);
    ezom_ast_node_t* two = ezom_ast_create_integer_literal(2);
    ezom_ast_node_t* three = ezom_ast_create_integer_literal(3);
    
    // Right side: 5 + 1
    ezom_ast_node_t* five = ezom_ast_create_integer_literal(5);
    ezom_ast_node_t* one = ezom_ast_create_integer_literal(1);
    
    if (!eight || !two || !three || !five || !one) {
        printf("ERROR: Failed to create integer literals\n");
        return 0;
    }
    
    // Create (8 + 2)
    ezom_ast_node_t* left_add = ezom_ast_create_binary_message(eight, "+", two);
    if (!left_add) {
        printf("ERROR: Failed to create left addition\n");
        return 0;
    }
    
    // Create (8 + 2) * 3
    ezom_ast_node_t* left_mult = ezom_ast_create_binary_message(left_add, "*", three);
    if (!left_mult) {
        printf("ERROR: Failed to create left multiplication\n");
        return 0;
    }
    
    // Create (5 + 1)
    ezom_ast_node_t* right_add = ezom_ast_create_binary_message(five, "+", one);
    if (!right_add) {
        printf("ERROR: Failed to create right addition\n");
        return 0;
    }
    
    // Create ((8 + 2) * 3) - (5 + 1)
    ezom_ast_node_t* final_expr = ezom_ast_create_binary_message(left_mult, "-", right_add);
    if (!final_expr) {
        printf("ERROR: Failed to create final subtraction\n");
        return 0;
    }
    
    printf("DEBUG: Deep nesting AST created successfully\n");
    ezom_ast_memory_stats();
    
    uint24_t result = ezom_ast_simple_evaluate(final_expr);
    if (result) {
        ezom_integer_t* res = (ezom_integer_t*)EZOM_OBJECT_PTR(result);
        printf("DEBUG: Deep nesting result: %d\n", res->value);
    }
    
    return result;
}
