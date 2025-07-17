#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezom_memory.h"
#include "ezom_object.h"
#include "ezom_parser.h"
#include "ezom_evaluator.h"
#include "ezom_lexer.h"
#include "ezom_ast.h"
#include "ezom_primitives.h"

// Test Phase 4.2.2 Block Objects and Closures
void test_block_system() {
    printf("=========================================\n");
    printf("TESTING PHASE 4.2.2 BLOCK SYSTEM\n");
    printf("=========================================\n");
    
    // Initialize memory and runtime
    ezom_init_memory();
    ezom_bootstrap_classes();
    
    // Test 1: Simple block without parameters
    printf("\n--- Test 1: Simple parameterless block ---\n");
    const char* block1_source = "[ 42 ]";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)block1_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* block1_ast = ezom_parse_block(&parser);
    if (block1_ast) {
        printf("✓ Block parsing successful\n");
        printf("  Parameters: %d\n", block1_ast->data.block.param_count);
        printf("  Locals: %d\n", block1_ast->data.block.local_count);
        
        // Evaluate the block literal
        ezom_eval_result_t result = ezom_evaluate_block_literal(block1_ast, 0);
        if (!result.is_error) {
            printf("✓ Block literal evaluation successful: 0x%06X\n", result.value);
            
            // Execute the block
            uint24_t block_result = ezom_block_evaluate(result.value, NULL, 0);
            printf("✓ Block execution result: 0x%06X\n", block_result);
            
            // Test the block via primitive
            uint24_t prim_result = prim_block_value(result.value, NULL, 0);
            printf("✓ Block primitive result: 0x%06X\n", prim_result);
        } else {
            printf("✗ Block literal evaluation failed: %s\n", result.error_msg);
        }
    } else {
        printf("✗ Block parsing failed\n");
    }
    
    // Test 2: Block with one parameter
    printf("\n--- Test 2: Block with one parameter ---\n");
    const char* block2_source = "[ :x | x + 1 ]";
    
    ezom_lexer_init(&lexer, (char*)block2_source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* block2_ast = ezom_parse_block(&parser);
    if (block2_ast) {
        printf("✓ Block with parameter parsing successful\n");
        printf("  Parameters: %d\n", block2_ast->data.block.param_count);
        printf("  Locals: %d\n", block2_ast->data.block.local_count);
        
        // Evaluate the block literal
        ezom_eval_result_t result = ezom_evaluate_block_literal(block2_ast, 0);
        if (!result.is_error) {
            printf("✓ Block literal evaluation successful: 0x%06X\n", result.value);
            
            // Execute the block with argument
            uint24_t arg = ezom_create_integer(5);
            uint24_t block_args[] = {arg};
            uint24_t block_result = ezom_block_evaluate(result.value, block_args, 1);
            printf("✓ Block execution with arg 5: 0x%06X\n", block_result);
            
            // Test the block via primitive
            uint24_t prim_result = prim_block_value_with(result.value, block_args, 1);
            printf("✓ Block primitive with arg 5: 0x%06X\n", prim_result);
        } else {
            printf("✗ Block literal evaluation failed: %s\n", result.error_msg);
        }
    } else {
        printf("✗ Block with parameter parsing failed\n");
    }
    
    // Test 3: Block with local variables
    printf("\n--- Test 3: Block with local variables ---\n");
    const char* block3_source = "[ | local | local := 10. local * 2 ]";
    
    ezom_lexer_init(&lexer, (char*)block3_source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* block3_ast = ezom_parse_block(&parser);
    if (block3_ast) {
        printf("✓ Block with locals parsing successful\n");
        printf("  Parameters: %d\n", block3_ast->data.block.param_count);
        printf("  Locals: %d\n", block3_ast->data.block.local_count);
        
        // Evaluate the block literal
        ezom_eval_result_t result = ezom_evaluate_block_literal(block3_ast, 0);
        if (!result.is_error) {
            printf("✓ Block literal evaluation successful: 0x%06X\n", result.value);
            
            // Execute the block
            uint24_t block_result = ezom_block_evaluate(result.value, NULL, 0);
            printf("✓ Block execution result: 0x%06X\n", block_result);
        } else {
            printf("✗ Block literal evaluation failed: %s\n", result.error_msg);
        }
    } else {
        printf("✗ Block with locals parsing failed\n");
    }
    
    // Test 4: Block with both parameters and locals
    printf("\n--- Test 4: Block with parameters and locals ---\n");
    const char* block4_source = "[ :x :y | | sum | sum := x + y. sum * 2 ]";
    
    ezom_lexer_init(&lexer, (char*)block4_source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* block4_ast = ezom_parse_block(&parser);
    if (block4_ast) {
        printf("✓ Block with params and locals parsing successful\n");
        printf("  Parameters: %d\n", block4_ast->data.block.param_count);
        printf("  Locals: %d\n", block4_ast->data.block.local_count);
        
        // Evaluate the block literal
        ezom_eval_result_t result = ezom_evaluate_block_literal(block4_ast, 0);
        if (!result.is_error) {
            printf("✓ Block literal evaluation successful: 0x%06X\n", result.value);
            
            // Execute the block with arguments
            uint24_t args[] = {ezom_create_integer(3), ezom_create_integer(4)};
            uint24_t block_result = ezom_block_evaluate(result.value, args, 2);
            printf("✓ Block execution with args 3,4: 0x%06X\n", block_result);
        } else {
            printf("✗ Block literal evaluation failed: %s\n", result.error_msg);
        }
    } else {
        printf("✗ Block with params and locals parsing failed\n");
    }
    
    // Test 5: Test control flow with blocks
    printf("\n--- Test 5: Control flow with blocks ---\n");
    const char* control_source = "true ifTrue: [ 'success' ] ifFalse: [ 'failure' ]";
    
    ezom_lexer_init(&lexer, (char*)control_source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* control_ast = ezom_parse_expression(&parser);
    if (control_ast) {
        printf("✓ Control flow parsing successful\n");
        
        // Evaluate the control flow
        ezom_eval_result_t result = ezom_evaluate_ast(control_ast, 0);
        if (!result.is_error) {
            printf("✓ Control flow evaluation successful: 0x%06X\n", result.value);
        } else {
            printf("✗ Control flow evaluation failed: %s\n", result.error_msg);
        }
    } else {
        printf("✗ Control flow parsing failed\n");
    }
    
    printf("\n=========================================\n");
    printf("PHASE 4.2.2 BLOCK SYSTEM TEST COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_block_system();
    return 0;
}
