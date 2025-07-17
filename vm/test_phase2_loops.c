// ============================================================================
// File: test_phase2_loops.c  
// Test Phase 4.3.2 Loop Constructs
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "include/ezom_object.h"
#include "include/ezom_memory.h"
#include "include/ezom_evaluator.h"
#include "include/ezom_primitives.h"
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_context.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void test_loop_constructs(void);
void test_integer_to_do_loop(void);
void test_integer_times_repeat(void);
void test_block_while_true(void);
void test_block_while_false(void);
void test_complex_loop_scenarios(void);

// ============================================================================
// MAIN TEST PROGRAM
// ============================================================================

int main(void) {
    printf("============================================================================\n");
    printf("EZOM Phase 4.3.2 Loop Constructs Test\n");
    printf("============================================================================\n");
    
    // Initialize the EZOM system
    ezom_init_memory();
    ezom_init_primitives();
    ezom_init_bootstrap();
    
    // Create a test context
    ezom_init_context();
    
    test_loop_constructs();
    
    printf("\n============================================================================\n");
    printf("✅ All loop construct tests passed!\n");
    printf("============================================================================\n");
    
    return 0;
}

// ============================================================================
// LOOP CONSTRUCT TESTS
// ============================================================================

void test_loop_constructs(void) {
    printf("\n🔄 Testing Loop Constructs...\n");
    
    test_integer_to_do_loop();
    test_integer_times_repeat();
    test_block_while_true();
    test_block_while_false();
    test_complex_loop_scenarios();
    
    printf("✅ Loop constructs working correctly!\n");
}

void test_integer_to_do_loop(void) {
    printf("\n   Testing 'to:do:' loop...\n");
    
    // Test: 1 to: 5 do: [ :i | i println ]
    printf("   -> 1 to: 5 do: [ :i | i println ]\n");
    
    char* input = "1 to: 5 do: [ :i | i println ]";
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    uint24_t result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ to:do: loop executed successfully\n");
    
    // Test: 10 to: 15 do: [ :i | i println ]
    printf("   -> 10 to: 15 do: [ :i | i println ]\n");
    
    input = "10 to: 15 do: [ :i | i println ]";
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ to:do: loop with different range executed successfully\n");
}

void test_integer_times_repeat(void) {
    printf("\n   Testing 'timesRepeat:' loop...\n");
    
    // Test: 3 timesRepeat: [ 'Hello' println ]
    printf("   -> 3 timesRepeat: [ 'Hello' println ]\n");
    
    char* input = "3 timesRepeat: [ 'Hello' println ]";
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    uint24_t result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ timesRepeat: loop executed successfully\n");
    
    // Test: 5 timesRepeat: [ 42 println ]
    printf("   -> 5 timesRepeat: [ 42 println ]\n");
    
    input = "5 timesRepeat: [ 42 println ]";
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ timesRepeat: loop with different count executed successfully\n");
}

void test_block_while_true(void) {
    printf("\n   Testing 'whileTrue:' loop...\n");
    
    // Test: [ i < 5 ] whileTrue: [ i println. i = i + 1 ]
    // Note: We need to set up a variable first
    printf("   -> Setting up variable and whileTrue: loop\n");
    
    // First set up the variable i = 0
    char* setup = "i = 0";
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    
    ezom_lexer_init(&lexer, setup);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    uint24_t result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    // Now test the while loop: [ i < 3 ] whileTrue: [ i println. i = i + 1 ]
    printf("   -> [ i < 3 ] whileTrue: [ i println. i = i + 1 ]\n");
    
    char* input = "[ i < 3 ] whileTrue: [ i println. i = i + 1 ]";
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ whileTrue: loop executed successfully\n");
}

void test_block_while_false(void) {
    printf("\n   Testing 'whileFalse:' loop...\n");
    
    // Test: [ i >= 3 ] whileFalse: [ i println. i = i + 1 ]
    // Note: We need to set up a variable first
    printf("   -> Setting up variable and whileFalse: loop\n");
    
    // First set up the variable i = 0
    char* setup = "i = 0";
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    
    ezom_lexer_init(&lexer, setup);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    uint24_t result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    // Now test the while loop: [ i >= 3 ] whileFalse: [ i println. i = i + 1 ]
    printf("   -> [ i >= 3 ] whileFalse: [ i println. i = i + 1 ]\n");
    
    char* input = "[ i >= 3 ] whileFalse: [ i println. i = i + 1 ]";
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ whileFalse: loop executed successfully\n");
}

void test_complex_loop_scenarios(void) {
    printf("\n   Testing complex loop scenarios...\n");
    
    // Test nested loops: 1 to: 3 do: [ :i | 1 to: 2 do: [ :j | i println ] ]
    printf("   -> Testing nested loops\n");
    
    char* input = "1 to: 3 do: [ :i | 1 to: 2 do: [ :j | i println ] ]";
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    uint24_t result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ Nested loops work correctly\n");
    
    // Test combination: 3 timesRepeat: [ 'Repeat' println ]
    printf("   -> Testing combination of timesRepeat with string\n");
    
    input = "3 timesRepeat: [ 'Repeat' println ]";
    ezom_lexer_init(&lexer, input);
    ezom_parser_init(&parser, &lexer);
    
    ast = ezom_parser_parse_expression(&parser);
    assert(ast != NULL);
    
    result = ezom_evaluator_evaluate(ast);
    assert(result != 0);
    
    printf("   ✅ Complex loop scenarios work correctly\n");
}
