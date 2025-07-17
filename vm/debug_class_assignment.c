#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"

// Debug the specific failing case
void debug_class_with_assignment() {
    printf("Debug class with assignment parsing...\n");
    
    // This is the exact problematic case from the test
    const char* class_code = 
        "Counter = Object (\n"
        "    | value counter_id |\n"
        "    \n"
        "    initialize = (\n"
        "        value := 0.\n"
        "        ^self\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)class_code);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    printf("Parsing class definition...\n");
    
    // Set a limit to avoid infinite loop
    int max_iterations = 50;
    int iteration = 0;
    
    ezom_ast_node_t* class_def = NULL;
    
    // Try to parse with protection against infinite loops
    while (iteration < max_iterations) {
        if (parser.has_error) {
            printf("Parser error after %d iterations: %s\n", iteration, parser.error_message);
            break;
        }
        
        if (lexer.current_token.type == TOKEN_EOF) {
            printf("Reached EOF after %d iterations\n", iteration);
            break;
        }
        
        printf("Iteration %d: Token = %s\n", iteration, ezom_token_type_name(lexer.current_token.type));
        
        if (iteration == 0) {
            class_def = ezom_parse_class_definition(&parser);
            if (class_def) {
                printf("✓ Class definition parsed successfully\n");
                return;
            }
        }
        
        iteration++;
        
        if (iteration > 10) {
            printf("Breaking to avoid infinite loop\n");
            break;
        }
    }
    
    printf("✗ Class definition parsing failed or timed out\n");
    if (parser.has_error) {
        printf("  Error: %s\n", parser.error_message);
    }
}

int main() {
    debug_class_with_assignment();
    return 0;
}
