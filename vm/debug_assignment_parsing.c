#include <stdio.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"

// Debug assignment parsing
void debug_assignment_parsing() {
    printf("=========================================\n");
    printf("DEBUG ASSIGNMENT PARSING\n");
    printf("=========================================\n");
    
    // Test 1: Simple assignment
    printf("\n--- Test 1: Simple assignment ---\n");
    const char* assignment = "value := 0";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)assignment);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    printf("Tokens:\n");
    while (lexer.current_token.type != TOKEN_EOF) {
        printf("  %d: '%.*s'\n", 
               (int)lexer.current_token.type,
               (int)lexer.current_token.length,
               lexer.current_token.text);
        ezom_lexer_next_token(&lexer);
    }
    
    // Reset and parse
    ezom_lexer_init(&lexer, (char*)assignment);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* stmt = ezom_parse_statement(&parser);
    if (stmt) {
        printf("✓ Assignment statement parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
        printf("  Is assignment: %s\n", stmt->type == AST_ASSIGNMENT ? "Yes" : "No");
    } else {
        printf("✗ Assignment statement parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 2: Return statement
    printf("\n--- Test 2: Return statement ---\n");
    const char* return_stmt = "^self";
    
    ezom_lexer_init(&lexer, (char*)return_stmt);
    ezom_parser_init(&parser, &lexer);
    
    printf("Tokens:\n");
    while (lexer.current_token.type != TOKEN_EOF) {
        printf("  %d: '%.*s'\n", 
               (int)lexer.current_token.type,
               (int)lexer.current_token.length,
               lexer.current_token.text);
        ezom_lexer_next_token(&lexer);
    }
    
    // Reset and parse
    ezom_lexer_init(&lexer, (char*)return_stmt);
    ezom_parser_init(&parser, &lexer);
    
    stmt = ezom_parse_statement(&parser);
    if (stmt) {
        printf("✓ Return statement parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
        printf("  Is return: %s\n", stmt->type == AST_RETURN ? "Yes" : "No");
    } else {
        printf("✗ Return statement parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 3: Assignment with period
    printf("\n--- Test 3: Assignment with period ---\n");
    const char* assignment_with_period = "value := 0.";
    
    ezom_lexer_init(&lexer, (char*)assignment_with_period);
    ezom_parser_init(&parser, &lexer);
    
    printf("Tokens:\n");
    while (lexer.current_token.type != TOKEN_EOF) {
        printf("  %d: '%.*s'\n", 
               (int)lexer.current_token.type,
               (int)lexer.current_token.length,
               lexer.current_token.text);
        ezom_lexer_next_token(&lexer);
    }
    
    // Reset and parse
    ezom_lexer_init(&lexer, (char*)assignment_with_period);
    ezom_parser_init(&parser, &lexer);
    
    stmt = ezom_parse_statement(&parser);
    if (stmt) {
        printf("✓ Assignment with period parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
        printf("  Is assignment: %s\n", stmt->type == AST_ASSIGNMENT ? "Yes" : "No");
    } else {
        printf("✗ Assignment with period parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    printf("\n=========================================\n");
    printf("DEBUG ASSIGNMENT PARSING COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    debug_assignment_parsing();
    return 0;
}
