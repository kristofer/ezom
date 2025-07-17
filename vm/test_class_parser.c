#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"

// Test function to parse and validate class definitions
void test_class_definition_parsing() {
    printf("=========================================\n");
    printf("TESTING CLASS DEFINITION PARSER\n");
    printf("=========================================\n");
    
    // Test 1: Basic class definition
    printf("\n--- Test 1: Basic class definition ---\n");
    const char* basic_class = "Counter = Object (\n"
                             "    initialize = (\n"
                             "        ^self\n"
                             "    )\n"
                             ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)basic_class);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_def = ezom_parse_class_definition(&parser);
    
    if (class_def) {
        printf("✓ Basic class parsing successful\n");
        printf("  Class name: %s\n", class_def->data.class_def.name);
        printf("  Superclass: %s\n", class_def->data.class_def.superclass ? 
               class_def->data.class_def.superclass->data.variable.name : "None");
        printf("  Has instance vars: %s\n", 
               class_def->data.class_def.instance_vars ? "Yes" : "No");
        printf("  Has instance methods: %s\n", 
               class_def->data.class_def.instance_methods ? "Yes" : "No");
        printf("  Has class methods: %s\n", 
               class_def->data.class_def.class_methods ? "Yes" : "No");
    } else {
        printf("✗ Basic class parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 2: Class with instance variables
    printf("\n--- Test 2: Class with instance variables ---\n");
    const char* class_with_vars = "Counter = Object (\n"
                                 "    | value counter_id |\n"
                                 "    \n"
                                 "    initialize = (\n"
                                 "        value := 0.\n"
                                 "        ^self\n"
                                 "    )\n"
                                 ")\n";
    
    ezom_lexer_t lexer2;
    ezom_lexer_init(&lexer2, (char*)class_with_vars);
    
    ezom_parser_t parser2;
    ezom_parser_init(&parser2, &lexer2);
    
    ezom_ast_node_t* class_def2 = ezom_parse_class_definition(&parser2);
    
    if (class_def2) {
        printf("✓ Class with instance variables parsing successful\n");
        printf("  Class name: %s\n", class_def2->data.class_def.name);
        printf("  Has instance vars: %s\n", 
               class_def2->data.class_def.instance_vars ? "Yes" : "No");
        
        // Count instance variables
        if (class_def2->data.class_def.instance_vars) {
            int var_count = 0;
            ezom_ast_node_t* current = class_def2->data.class_def.instance_vars;
            while (current) {
                var_count++;
                current = current->next;
            }
            printf("  Instance variable count: %d\n", var_count);
        }
    } else {
        printf("✗ Class with instance variables parsing failed\n");
        printf("  Error: %s\n", parser2.error_message);
    }
    
    // Test 3: Class with class methods
    printf("\n--- Test 3: Class with class methods ---\n");
    const char* class_with_class_methods = "Counter = Object (\n"
                                          "    | value |\n"
                                          "    \n"
                                          "    initialize = (\n"
                                          "        value := 0.\n"
                                          "        ^self\n"
                                          "    )\n"
                                          "    \n"
                                          "    ----\n"
                                          "    \n"
                                          "    new = (\n"
                                          "        ^super new initialize\n"
                                          "    )\n"
                                          ")\n";
    
    ezom_lexer_t lexer3;
    ezom_lexer_init(&lexer3, (char*)class_with_class_methods);
    
    ezom_parser_t parser3;
    ezom_parser_init(&parser3, &lexer3);
    
    ezom_ast_node_t* class_def3 = ezom_parse_class_definition(&parser3);
    
    if (class_def3) {
        printf("✓ Class with class methods parsing successful\n");
        printf("  Class name: %s\n", class_def3->data.class_def.name);
        printf("  Has class methods: %s\n", 
               class_def3->data.class_def.class_methods ? "Yes" : "No");
    } else {
        printf("✗ Class with class methods parsing failed\n");
        printf("  Error: %s\n", parser3.error_message);
    }
    
    printf("\n=========================================\n");
    printf("CLASS DEFINITION PARSER TESTS COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_class_definition_parsing();
    return 0;
}
