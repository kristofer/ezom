#include <stdio.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"

// Debug step by step statement parsing
void debug_step_by_step() {
    printf("=========================================\n");
    printf("DEBUG STEP BY STEP STATEMENT PARSING\n");
    printf("=========================================\n");
    
    // Test 1: Just assignment
    printf("\n--- Test 1: Just assignment ---\n");
    const char* just_assignment = "value := 0.";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)just_assignment);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* stmt = ezom_parse_statement(&parser);
    if (stmt) {
        printf("✓ Assignment parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
        printf("  Current token after parsing: %d\n", parser.lexer->current_token.type);
    } else {
        printf("✗ Assignment parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 2: Just return
    printf("\n--- Test 2: Just return ---\n");
    const char* just_return = "^self";
    
    ezom_lexer_init(&lexer, (char*)just_return);
    ezom_parser_init(&parser, &lexer);
    
    stmt = ezom_parse_statement(&parser);
    if (stmt) {
        printf("✓ Return parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
        printf("  Current token after parsing: %d\n", parser.lexer->current_token.type);
    } else {
        printf("✗ Return parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 3: Assignment then return in sequence
    printf("\n--- Test 3: Assignment then return in sequence ---\n");
    const char* assignment_then_return = "value := 0.\n^self";
    
    ezom_lexer_init(&lexer, (char*)assignment_then_return);
    ezom_parser_init(&parser, &lexer);
    
    printf("Parsing first statement...\n");
    stmt = ezom_parse_statement(&parser);
    if (stmt) {
        printf("✓ First statement parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
        printf("  Current token after first statement: %d\n", parser.lexer->current_token.type);
    } else {
        printf("✗ First statement parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Skip newlines
    printf("Current token before skipping: %d\n", parser.lexer->current_token.type);
    while (parser.lexer->current_token.type == TOKEN_NEWLINE) {
        printf("  Skipping newline token\n");
        ezom_parser_advance(&parser);
    }
    printf("After skipping newlines, current token: %d\n", parser.lexer->current_token.type);
    
    // Let's also try manually advancing past the period
    if (parser.lexer->current_token.type == TOKEN_DOT) {
        printf("  Found period, advancing past it\n");
        ezom_parser_advance(&parser);
        printf("  After advancing past period, current token: %d\n", parser.lexer->current_token.type);
        
        // Skip newlines again
        while (parser.lexer->current_token.type == TOKEN_NEWLINE) {
            printf("    Skipping newline token after period\n");
            ezom_parser_advance(&parser);
        }
        printf("  After skipping newlines after period, current token: %d\n", parser.lexer->current_token.type);
    }
    
    printf("Parsing second statement...\n");
    stmt = ezom_parse_statement(&parser);
    if (stmt) {
        printf("✓ Second statement parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
        printf("  Current token after second statement: %d\n", parser.lexer->current_token.type);
    } else {
        printf("✗ Second statement parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    printf("\n=========================================\n");
    printf("DEBUG STEP BY STEP PARSING COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    debug_step_by_step();
    return 0;
}
