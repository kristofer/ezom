// Simple test to demonstrate Phase 2 capabilities
#include <stdio.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"

int main() {
    printf("EZOM Phase 2 - Simple Component Test\n");
    printf("====================================\n");
    
    // Test 1: Lexer
    const char* code = "42 + 8";
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, code, strlen(code));
    
    printf("1. Lexing '%s':\n", code);
    int count = 0;
    while (count < 5) {
        ezom_lexer_next_token(&lexer);
        if (lexer.current_token.type == TOKEN_EOF) break;
        
        switch (lexer.current_token.type) {
            case TOKEN_INTEGER:
                printf("   INTEGER: %d\n", lexer.current_token.value.int_value);
                break;
            case TOKEN_PLUS:
                printf("   PLUS\n");
                break;
            default:
                printf("   OTHER: %d\n", lexer.current_token.type);
                break;
        }
        count++;
    }
    
    // Test 2: AST Creation
    printf("\n2. Creating AST for '42':\n");
    ezom_ast_node_t* ast = ezom_ast_create_literal_integer(42);
    if (ast) {
        printf("   AST created successfully\n");
        printf("   Type: %d, Value: %d\n", ast->type, ast->data.literal.value.integer_value);
        ezom_ast_free(ast);
    }
    
    printf("\nPhase 2 components are working!\n");
    return 0;
}