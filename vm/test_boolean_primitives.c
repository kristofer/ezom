#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ezom_memory.h"
#include "ezom_object.h"
#include "ezom_context.h"
#include "ezom_lexer.h"
#include "ezom_parser.h"
#include "ezom_ast.h"
#include "ezom_evaluator.h"
#include "ezom_primitives.h"
#include "ezom_dispatch.h"

// Helper function to parse and evaluate a simple expression
ezom_eval_result_t parse_and_evaluate(const char* code) {
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)code);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* ast = ezom_parse_expression(&parser);
    if (!ast) {
        printf("   Parse error\n");
        return (ezom_eval_result_t){ .value = g_nil, .is_error = true };
    }
    
    return ezom_evaluate_ast(ast, 0);
}

// Helper function to check if result is an integer with specific value
bool is_integer_with_value(uint24_t obj, int expected_value) {
    if (!obj) return false;
    
    ezom_object_t* obj_ptr = (ezom_object_t*)EZOM_OBJECT_PTR(obj);
    if (!obj_ptr || obj_ptr->flags != EZOM_TYPE_INTEGER) return false;
    
    ezom_integer_t* int_obj = (ezom_integer_t*)obj_ptr;
    return int_obj->value == expected_value;
}

// Helper function to check if result is true
bool is_true(uint24_t obj) {
    return obj == g_true;
}

// Helper function to check if result is false
bool is_false(uint24_t obj) {
    return obj == g_false;
}

// Helper function to check if result is nil
bool is_nil(uint24_t obj) {
    return obj == g_nil;
}

int main() {
    printf("===========================================\n");
    printf("TESTING BOOLEAN PRIMITIVES (COMPREHENSIVE)\n");
    printf("===========================================\n");
    
    // Initialize VM
    ezom_init_memory();
    ezom_bootstrap_classes();
    ezom_bootstrap_enhanced_classes();
    ezom_init_primitives();
    
    printf("Global values after bootstrap:\n");
    printf("  g_true = 0x%06X\n", g_true);
    printf("  g_false = 0x%06X\n", g_false);
    printf("  g_nil = 0x%06X\n", g_nil);
    
    // Test 1: Basic boolean values
    printf("\n=== Test 1: Basic Boolean Values ===\n");
    
    ezom_eval_result_t result_true = parse_and_evaluate("true");
    printf("true -> 0x%06X ", result_true.value);
    if (is_true(result_true.value)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected g_true)\n");
    }
    
    ezom_eval_result_t result_false = parse_and_evaluate("false");
    printf("false -> 0x%06X ", result_false.value);
    if (is_false(result_false.value)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected g_false)\n");
    }
    
    // Test 2: Boolean not operations
    printf("\n=== Test 2: Boolean Not Operations ===\n");
    
    ezom_eval_result_t true_not = parse_and_evaluate("true not");
    printf("true not -> 0x%06X ", true_not.value);
    if (is_false(true_not.value)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected g_false)\n");
    }
    
    ezom_eval_result_t false_not = parse_and_evaluate("false not");
    printf("false not -> 0x%06X ", false_not.value);
    if (is_true(false_not.value)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected g_true)\n");
    }
    
    // Test 3: Simple block expressions for debugging
    printf("\n=== Test 3: Simple Block Expressions ===\n");
    
    ezom_eval_result_t block_42 = parse_and_evaluate("[42]");
    printf("[42] -> 0x%06X ", block_42.value);
    if (block_42.value != g_nil) {
        printf("✓ PASS (block object created)\n");
    } else {
        printf("✗ FAIL (expected block object)\n");
    }
    
    // Test 4: true ifTrue: with simple block
    printf("\n=== Test 4: true ifTrue: Operations ===\n");
    
    ezom_eval_result_t true_if_true = parse_and_evaluate("true ifTrue: [42]");
    printf("true ifTrue: [42] -> 0x%06X ", true_if_true.value);
    if (is_integer_with_value(true_if_true.value, 42)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected integer 42)\n");
    }
    
    // Test 5: false ifTrue: with simple block
    printf("\n=== Test 5: false ifTrue: Operations ===\n");
    
    ezom_eval_result_t false_if_true = parse_and_evaluate("false ifTrue: [42]");
    printf("false ifTrue: [42] -> 0x%06X ", false_if_true.value);
    if (is_nil(false_if_true.value)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected nil)\n");
    }
    
    // Test 6: true ifFalse: with simple block
    printf("\n=== Test 6: true ifFalse: Operations ===\n");
    
    ezom_eval_result_t true_if_false = parse_and_evaluate("true ifFalse: [42]");
    printf("true ifFalse: [42] -> 0x%06X ", true_if_false.value);
    if (is_nil(true_if_false.value)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected nil)\n");
    }
    
    // Test 7: false ifFalse: with simple block
    printf("\n=== Test 7: false ifFalse: Operations ===\n");
    
    ezom_eval_result_t false_if_false = parse_and_evaluate("false ifFalse: [42]");
    printf("false ifFalse: [42] -> 0x%06X ", false_if_false.value);
    if (is_integer_with_value(false_if_false.value, 42)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected integer 42)\n");
    }
    
    // Test 8: true ifTrue:ifFalse: with simple blocks
    printf("\n=== Test 8: true ifTrue:ifFalse: Operations ===\n");
    
    ezom_eval_result_t true_if_true_if_false = parse_and_evaluate("true ifTrue: [100] ifFalse: [200]");
    printf("true ifTrue: [100] ifFalse: [200] -> 0x%06X ", true_if_true_if_false.value);
    if (is_integer_with_value(true_if_true_if_false.value, 100)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected integer 100)\n");
    }
    
    // Test 9: false ifTrue:ifFalse: with simple blocks
    printf("\n=== Test 9: false ifTrue:ifFalse: Operations ===\n");
    
    ezom_eval_result_t false_if_true_if_false = parse_and_evaluate("false ifTrue: [100] ifFalse: [200]");
    printf("false ifTrue: [100] ifFalse: [200] -> 0x%06X ", false_if_true_if_false.value);
    if (is_integer_with_value(false_if_true_if_false.value, 200)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected integer 200)\n");
    }
    
    // Test 10: Nested conditionals
    printf("\n=== Test 10: Nested Conditionals ===\n");
    
    ezom_eval_result_t nested = parse_and_evaluate("true ifTrue: [false ifTrue: [1] ifFalse: [2]]");
    printf("true ifTrue: [false ifTrue: [1] ifFalse: [2]] -> 0x%06X ", nested.value);
    if (is_integer_with_value(nested.value, 2)) {
        printf("✓ PASS\n");
    } else {
        printf("✗ FAIL (expected integer 2)\n");
    }
    
    printf("\n===========================================\n");
    printf("BOOLEAN PRIMITIVES TEST COMPLETE\n");
    printf("===========================================\n");
    
    return 0;
}
