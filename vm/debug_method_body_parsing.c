#include <stdio.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"

// Debug method body parsing
void debug_method_body_parsing() {
    printf("=========================================\n");
    printf("DEBUG METHOD BODY PARSING\n");
    printf("=========================================\n");
    
    // Test 1: Simple method body
    printf("\n--- Test 1: Simple method body ---\n");
    const char* method_body = "value := 0.\n^self";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)method_body);
    
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
    ezom_lexer_init(&lexer, (char*)method_body);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* stmt_list = ezom_parse_statement_list(&parser);
    if (stmt_list) {
        printf("✓ Method body parsed successfully\n");
        printf("  Type: %d\n", stmt_list->type);
        printf("  Statement count: %d\n", stmt_list->data.statement_list.count);
    } else {
        printf("✗ Method body parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 2: Full method definition
    printf("\n--- Test 2: Full method definition ---\n");
    const char* full_method = "initialize = (\n    value := 0.\n    ^self\n)";
    
    ezom_lexer_init(&lexer, (char*)full_method);
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
    ezom_lexer_init(&lexer, (char*)full_method);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* method = ezom_parse_method_definition(&parser, false);
    if (method) {
        printf("✓ Method definition parsed successfully\n");
        printf("  Type: %d\n", method->type);
        printf("  Selector: %s\n", method->data.method_def.selector);
        printf("  Has body: %s\n", method->data.method_def.body ? "Yes" : "No");
        if (method->data.method_def.body) {
            printf("  Body statement count: %d\n", method->data.method_def.body->data.statement_list.count);
        }
    } else {
        printf("✗ Method definition parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    printf("\n=========================================\n");
    printf("DEBUG METHOD BODY PARSING COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    debug_method_body_parsing();
    return 0;
}
