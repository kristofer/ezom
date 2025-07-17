#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"

// Simple test to debug assignment parsing
void debug_assignment_parsing() {
    printf("Debug assignment parsing...\n");
    
    // Test simple assignment
    const char* simple_assignment = "value := 0";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)simple_assignment);
    
    printf("Tokenizing: %s\n", simple_assignment);
    
    // Tokenize to see what we get
    ezom_lexer_next_token(&lexer);
    while (lexer.current_token.type != TOKEN_EOF) {
        printf("Token: %s = '%.*s'\n", 
               ezom_token_type_name(lexer.current_token.type),
               lexer.current_token.length,
               lexer.current_token.text);
        ezom_lexer_next_token(&lexer);
    }
    
    // Reset and try parsing
    ezom_lexer_init(&lexer, (char*)simple_assignment);
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    printf("\nParsing as statement...\n");
    ezom_ast_node_t* stmt = ezom_parse_statement(&parser);
    
    if (stmt) {
        printf("✓ Assignment parsed successfully\n");
        printf("  Type: %d\n", stmt->type);
    } else {
        printf("✗ Assignment parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
}

int main() {
    debug_assignment_parsing();
    return 0;
}
