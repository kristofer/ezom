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
        ezom_eval_result_t error_result = {0};
        error_result.is_error = true;
        strcpy(error_result.error_msg, "Parse failed");
        return error_result;
    }
    
    return ezom_evaluate_ast(ast, 0);
}

// Test Phase 4.3.1 Conditional Statements
int main() {
    printf("=========================================\n");
    printf("TESTING PHASE 4.3.1 CONDITIONAL STATEMENTS\n");
    printf("=========================================\n");
    
    // Initialize VM
    ezom_init_memory();
    ezom_bootstrap_classes();
    ezom_bootstrap_enhanced_classes();  // This creates g_true and g_false
    ezom_init_primitives();
    
    printf("\n--- Test 1: true ifTrue: [42] ---\n");
    
    const char* code1 = "true ifTrue: [42]";
    printf("Parsing: %s\n", code1);
    
    ezom_eval_result_t result1 = parse_and_evaluate(code1);
    
    if (result1.is_error) {
        printf("✗ Error: %s\n", result1.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result1.value);
        
        // Check if result is integer 42
        if (result1.value && ezom_is_integer(result1.value)) {
            ezom_object_t* obj = EZOM_OBJECT_PTR(result1.value);
            ezom_integer_t* int_obj = (ezom_integer_t*)obj;
            printf("✓ Result is integer: %d\n", int_obj->value);
        } else {
            printf("✗ Result is not integer: 0x%06X\n", result1.value);
        }
    }
    
    printf("\n--- Test 2: false ifTrue: [42] ---\n");
    
    const char* code2 = "false ifTrue: [42]";
    printf("Parsing: %s\n", code2);
    
    ezom_eval_result_t result2 = parse_and_evaluate(code2);
    
    if (result2.is_error) {
        printf("✗ Error: %s\n", result2.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result2.value);
        
        // Should be nil (block not executed)
        if (result2.value == g_nil) {
            printf("✓ Result is nil (correct - block not executed)\n");
        } else {
            printf("✗ Result should be nil but got: 0x%06X\n", result2.value);
        }
    }
    
    printf("\n--- Test 3: true ifFalse: [42] ---\n");
    
    const char* code3 = "true ifFalse: [42]";
    printf("Parsing: %s\n", code3);
    
    ezom_eval_result_t result3 = parse_and_evaluate(code3);
    
    if (result3.is_error) {
        printf("✗ Error: %s\n", result3.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result3.value);
        
        // Should be nil (block not executed)
        if (result3.value == g_nil) {
            printf("✓ Result is nil (correct - block not executed)\n");
        } else {
            printf("✗ Result should be nil but got: 0x%06X\n", result3.value);
        }
    }
    
    printf("\n--- Test 4: false ifFalse: [42] ---\n");
    
    const char* code4 = "false ifFalse: [42]";
    printf("Parsing: %s\n", code4);
    
    ezom_eval_result_t result4 = parse_and_evaluate(code4);
    
    if (result4.is_error) {
        printf("✗ Error: %s\n", result4.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result4.value);
        
        // Check if result is integer 42
        if (result4.value && ezom_is_integer(result4.value)) {
            ezom_object_t* obj = EZOM_OBJECT_PTR(result4.value);
            ezom_integer_t* int_obj = (ezom_integer_t*)obj;
            printf("✓ Result is integer: %d\n", int_obj->value);
        } else {
            printf("✗ Result is not integer: 0x%06X\n", result4.value);
        }
    }
    
    printf("\n--- Test 5: true ifTrue: [100] ifFalse: [200] ---\n");
    
    const char* code5 = "true ifTrue: [100] ifFalse: [200]";
    printf("Parsing: %s\n", code5);
    
    ezom_eval_result_t result5 = parse_and_evaluate(code5);
    
    if (result5.is_error) {
        printf("✗ Error: %s\n", result5.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result5.value);
        
        // Check if result is integer 100
        if (result5.value && ezom_is_integer(result5.value)) {
            ezom_object_t* obj = EZOM_OBJECT_PTR(result5.value);
            ezom_integer_t* int_obj = (ezom_integer_t*)obj;
            printf("✓ Result is integer: %d (should be 100)\n", int_obj->value);
        } else {
            printf("✗ Result is not integer: 0x%06X\n", result5.value);
        }
    }
    
    printf("\n--- Test 6: false ifTrue: [100] ifFalse: [200] ---\n");
    
    const char* code6 = "false ifTrue: [100] ifFalse: [200]";
    printf("Parsing: %s\n", code6);
    
    ezom_eval_result_t result6 = parse_and_evaluate(code6);
    
    if (result6.is_error) {
        printf("✗ Error: %s\n", result6.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result6.value);
        
        // Check if result is integer 200
        if (result6.value && ezom_is_integer(result6.value)) {
            ezom_object_t* obj = EZOM_OBJECT_PTR(result6.value);
            ezom_integer_t* int_obj = (ezom_integer_t*)obj;
            printf("✓ Result is integer: %d (should be 200)\n", int_obj->value);
        } else {
            printf("✗ Result is not integer: 0x%06X\n", result6.value);
        }
    }
    
    printf("\n--- Test 7: Boolean not operation ---\n");
    
    const char* code7 = "true not";
    printf("Parsing: %s\n", code7);
    
    ezom_eval_result_t result7 = parse_and_evaluate(code7);
    
    if (result7.is_error) {
        printf("✗ Error: %s\n", result7.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result7.value);
        
        // Check if result is false
        if (result7.value == g_false) {
            printf("✓ Result is false (correct)\n");
        } else {
            printf("✗ Result should be false but got: 0x%06X\n", result7.value);
        }
    }
    
    printf("\n--- Test 8: false not ---\n");
    
    const char* code8 = "false not";
    printf("Parsing: %s\n", code8);
    
    ezom_eval_result_t result8 = parse_and_evaluate(code8);
    
    if (result8.is_error) {
        printf("✗ Error: %s\n", result8.error_msg);
    } else {
        printf("✓ Evaluation result: 0x%06X\n", result8.value);
        
        // Check if result is true
        if (result8.value == g_true) {
            printf("✓ Result is true (correct)\n");
        } else {
            printf("✗ Result should be true but got: 0x%06X\n", result8.value);
        }
    }
    
    printf("\n=========================================\n");
    printf("PHASE 4.3.1 CONDITIONAL STATEMENTS TEST COMPLETE\n");
    printf("=========================================\n");
    
    return 0;
}
