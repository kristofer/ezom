#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezom_memory.h"
#include "ezom_object.h"
#include "ezom_parser.h"
#include "ezom_evaluator.h"
#include "ezom_lexer.h"
#include "ezom_ast.h"

// Simple block test to debug segfault
void test_simple_block() {
    printf("=========================================\n");
    printf("SIMPLE BLOCK TEST FOR DEBUGGING\n");
    printf("=========================================\n");
    
    // Initialize memory and runtime
    ezom_init_memory();
    ezom_bootstrap_classes();
    
    // Test 1: Just parse a simple block
    printf("\n--- Test 1: Parse simple block ---\n");
    const char* block_source = "[ 42 ]";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)block_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    printf("Parsing: %s\n", block_source);
    ezom_ast_node_t* block_ast = ezom_parse_block(&parser);
    
    if (block_ast) {
        printf("✓ Block parsing successful\n");
        printf("  Type: %d (should be %d for AST_BLOCK)\n", block_ast->type, AST_BLOCK);
        printf("  Parameters: %d\n", block_ast->data.block.param_count);
        printf("  Locals: %d\n", block_ast->data.block.local_count);
        
        // Test 2: Create block object without evaluating
        printf("\n--- Test 2: Create block object ---\n");
        uint24_t block_obj = ezom_create_ast_block(block_ast, 0);
        if (block_obj) {
            printf("✓ Block object created: 0x%06X\n", block_obj);
            
            // Test 3: Check block structure
            printf("\n--- Test 3: Examine block structure ---\n");
            ezom_block_t* block = (ezom_block_t*)EZOM_OBJECT_PTR(block_obj);
            printf("  Block param_count: %d\n", block->param_count);
            printf("  Block local_count: %d\n", block->local_count);
            printf("  Block outer_context: 0x%06X\n", block->outer_context);
            printf("  Block code: %p\n", block->code);
            
            // Test 4: Try to examine the AST
            printf("\n--- Test 4: Examine AST pointer ---\n");
            if (block->code) {
                ezom_ast_node_t* ast = (ezom_ast_node_t*)block->code;
                printf("  AST type: %d\n", ast->type);
                if (ast->type == AST_BLOCK) {
                    printf("  AST body pointer: %p\n", ast->data.block.body);
                    if (ast->data.block.body) {
                        printf("  Body type: %d\n", ast->data.block.body->type);
                    }
                }
            }
            
            printf("\n--- Test 5: Simple block evaluation ---\n");
            printf("About to call ezom_block_evaluate...\n");
            
            // This might be where the segfault occurs
            uint24_t result = ezom_block_evaluate(block_obj, NULL, 0);
            printf("✓ Block evaluation result: 0x%06X\n", result);
        } else {
            printf("✗ Block object creation failed\n");
        }
    } else {
        printf("✗ Block parsing failed\n");
    }
    
    printf("\n=========================================\n");
    printf("SIMPLE BLOCK TEST COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_simple_block();
    return 0;
}
