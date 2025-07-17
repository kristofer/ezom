#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"

// Test method compilation pipeline
void test_method_compilation() {
    printf("=========================================\n");
    printf("TESTING METHOD COMPILATION PIPELINE\n");
    printf("=========================================\n");
    
    // Test 1: Simple method with assignment
    printf("\n--- Test 1: Simple method with assignment ---\n");
    const char* simple_method = 
        "initialize = (\n"
        "    value := 0.\n"
        "    ^self\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)simple_method);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* method = ezom_parse_method_definition(&parser, false);
    
    if (method) {
        printf("✓ Simple method parsing successful\n");
        printf("  Selector: %s\n", method->data.method_def.selector);
        printf("  Has locals: %s\n", method->data.method_def.locals ? "Yes" : "No");
        printf("  Has body: %s\n", method->data.method_def.body ? "Yes" : "No");
        
        // Count statements in body
        if (method->data.method_def.body) {
            int stmt_count = 0;
            ezom_ast_node_t* current = method->data.method_def.body;
            while (current) {
                stmt_count++;
                current = current->next;
            }
            printf("  Statement count: %d\n", stmt_count);
        }
    } else {
        printf("✗ Simple method parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 2: Method with parameters
    printf("\n--- Test 2: Method with parameters ---\n");
    const char* method_with_params = 
        "add: x to: y = (\n"
        "    | result |\n"
        "    result := x + y.\n"
        "    ^result\n"
        ")\n";
    
    ezom_lexer_t lexer2;
    ezom_lexer_init(&lexer2, (char*)method_with_params);
    
    ezom_parser_t parser2;
    ezom_parser_init(&parser2, &lexer2);
    
    ezom_ast_node_t* method2 = ezom_parse_method_definition(&parser2, false);
    
    if (method2) {
        printf("✓ Method with parameters parsing successful\n");
        printf("  Selector: %s\n", method2->data.method_def.selector);
        printf("  Has parameters: %s\n", method2->data.method_def.parameters ? "Yes" : "No");
        printf("  Has locals: %s\n", method2->data.method_def.locals ? "Yes" : "No");
        printf("  Has body: %s\n", method2->data.method_def.body ? "Yes" : "No");
    } else {
        printf("✗ Method with parameters parsing failed\n");
        printf("  Error: %s\n", parser2.error_message);
    }
    
    // Test 3: Assignment statements
    printf("\n--- Test 3: Assignment statements ---\n");
    const char* assignment_test = "value := 42";
    
    ezom_lexer_t lexer3;
    ezom_lexer_init(&lexer3, (char*)assignment_test);
    
    ezom_parser_t parser3;
    ezom_parser_init(&parser3, &lexer3);
    
    ezom_ast_node_t* assignment = ezom_parse_statement(&parser3);
    
    if (assignment) {
        printf("✓ Assignment parsing successful\n");
        printf("  Type: %d\n", assignment->type);
        printf("  Is assignment: %s\n", assignment->type == AST_ASSIGNMENT ? "Yes" : "No");
    } else {
        printf("✗ Assignment parsing failed\n");
        printf("  Error: %s\n", parser3.error_message);
    }
    
    // Test 4: Complex expressions
    printf("\n--- Test 4: Complex expressions ---\n");
    const char* complex_expr = "x + y * 2";
    
    ezom_lexer_t lexer4;
    ezom_lexer_init(&lexer4, (char*)complex_expr);
    
    ezom_parser_t parser4;
    ezom_parser_init(&parser4, &lexer4);
    
    ezom_ast_node_t* expr = ezom_parse_expression(&parser4);
    
    if (expr) {
        printf("✓ Complex expression parsing successful\n");
        printf("  Type: %d\n", expr->type);
    } else {
        printf("✗ Complex expression parsing failed\n");
        printf("  Error: %s\n", parser4.error_message);
    }
    
    printf("\n=========================================\n");
    printf("METHOD COMPILATION PIPELINE TESTS COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_method_compilation();
    return 0;
}
