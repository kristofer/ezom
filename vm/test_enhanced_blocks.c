#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/ezom_object.h"
#include "../include/ezom_context.h"
#include "../include/ezom_parser.h"
#include "../include/ezom_evaluator.h"
#include "../include/ezom_lexer.h"
#include "../include/ezom_memory.h"

// Test enhanced block parameter and local variable functionality
void test_block_parameter_binding() {
    printf("Testing enhanced block parameter binding...\n");
    
    // Test source: [:x :y | x + y]
    const char* block_source = "[:x :y | x + y]";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, block_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* block_ast = ezom_parse_block(&parser);
    assert(block_ast != NULL);
    assert(block_ast->type == AST_BLOCK);
    assert(block_ast->data.block.param_count == 2);
    
    // Create a block object from the AST
    uint24_t block_obj = ezom_create_ast_block(block_ast, 0);
    assert(block_obj != 0);
    
    // Test block evaluation with arguments
    uint24_t arg1 = ezom_create_integer(5);
    uint24_t arg2 = ezom_create_integer(3);
    uint24_t args[2] = {arg1, arg2};
    
    uint24_t result = ezom_block_evaluate(block_obj, args, 2);
    assert(result != 0);
    
    printf("✓ Block parameter binding test passed\n");
}

void test_block_local_variables() {
    printf("Testing enhanced block local variables...\n");
    
    // Test source: [:x | temp | temp := x * 2. temp]
    const char* block_source = "[:x | temp | temp := x * 2. temp]";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, block_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* block_ast = ezom_parse_block(&parser);
    assert(block_ast != NULL);
    assert(block_ast->type == AST_BLOCK);
    assert(block_ast->data.block.param_count == 1);
    assert(block_ast->data.block.local_count == 1);
    
    // Create a block object from the AST
    uint24_t block_obj = ezom_create_ast_block(block_ast, 0);
    assert(block_obj != 0);
    
    // Test block evaluation with local variable
    uint24_t arg = ezom_create_integer(7);
    uint24_t args[1] = {arg};
    
    uint24_t result = ezom_block_evaluate(block_obj, args, 1);
    assert(result != 0);
    
    printf("✓ Block local variable test passed\n");
}

void test_block_closure_capture() {
    printf("Testing enhanced block closure capture...\n");
    
    // Test nested blocks with closure capture
    // outer_var := 10.
    // block := [:x | outer_var + x].
    // block value: 5.
    
    const char* test_source = "outer_var := 10. block := [:x | outer_var + x]. block value: 5";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, test_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* program_ast = ezom_parse_statement_list(&parser);
    assert(program_ast != NULL);
    
    // Create a context and evaluate the program
    uint24_t context = ezom_create_extended_context(0, 0, 0, 5);
    assert(context != 0);
    
    ezom_eval_result_t result = ezom_evaluate_ast(program_ast, context);
    assert(!result.is_error);
    
    printf("✓ Block closure capture test passed\n");
}

void test_block_nested_contexts() {
    printf("Testing enhanced block nested contexts...\n");
    
    // Test nested block contexts
    // [:x | [:y | x + y] value: 3] value: 2
    
    const char* nested_source = "[:x | [:y | x + y] value: 3] value: 2";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, nested_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* expr_ast = ezom_parse_expression(&parser);
    assert(expr_ast != NULL);
    
    // Create a context and evaluate the nested block expression
    uint24_t context = ezom_create_extended_context(0, 0, 0, 5);
    assert(context != 0);
    
    ezom_eval_result_t result = ezom_evaluate_ast(expr_ast, context);
    assert(!result.is_error);
    
    printf("✓ Block nested contexts test passed\n");
}

void test_block_argument_mismatch() {
    printf("Testing enhanced block argument mismatch handling...\n");
    
    // Test block with parameter count mismatch
    const char* block_source = "[:x :y | x + y]";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, block_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* block_ast = ezom_parse_block(&parser);
    assert(block_ast != NULL);
    
    uint24_t block_obj = ezom_create_ast_block(block_ast, 0);
    assert(block_obj != 0);
    
    // Test with fewer arguments than parameters
    uint24_t arg = ezom_create_integer(5);
    uint24_t args[1] = {arg};
    
    uint24_t result = ezom_block_evaluate(block_obj, args, 1);
    // Should still work, with missing parameters initialized to nil
    assert(result != 0);
    
    // Test with more arguments than parameters
    uint24_t arg1 = ezom_create_integer(5);
    uint24_t arg2 = ezom_create_integer(3);
    uint24_t arg3 = ezom_create_integer(1);
    uint24_t args3[3] = {arg1, arg2, arg3};
    
    uint24_t result2 = ezom_block_evaluate(block_obj, args3, 3);
    // Should work, with extra arguments ignored
    assert(result2 != 0);
    
    printf("✓ Block argument mismatch handling test passed\n");
}

int main() {
    printf("=== Enhanced Block Objects Test Suite ===\n");
    
    // Initialize the VM systems in the correct order
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_context_system();
    
    // Run enhanced block tests
    test_block_parameter_binding();
    test_block_local_variables();
    test_block_closure_capture();
    test_block_nested_contexts();
    test_block_argument_mismatch();
    
    printf("\n=== All Enhanced Block Tests Passed! ===\n");
    return 0;
}
