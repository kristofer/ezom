#include <stdio.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"
#include "include/ezom_evaluator.h"

// Test instance variable system
void test_instance_variable_system() {
    printf("=========================================\n");
    printf("TESTING INSTANCE VARIABLE SYSTEM\n");
    printf("=========================================\n");
    
    // Test 1: Parse class with instance variables
    printf("\n--- Test 1: Class with instance variables ---\n");
    const char* class_with_ivars = 
        "Counter = Object (\n"
        "    | value counter_id |\n"
        "    \n"
        "    initialize = (\n"
        "        value := 0.\n"
        "        counter_id := 'counter'.\n"
        "        ^self\n"
        "    )\n"
        "    \n"
        "    increment = (\n"
        "        value := value + 1.\n"
        "        ^value\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)class_with_ivars);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    if (class_ast) {
        printf("✓ Class parsing successful\n");
        printf("  Class name: %s\n", class_ast->data.class_def.name);
        
        if (class_ast->data.class_def.instance_vars) {
            printf("  Instance variables: %d\n", class_ast->data.class_def.instance_vars->data.variable_list.count);
            for (int i = 0; i < class_ast->data.class_def.instance_vars->data.variable_list.count; i++) {
                printf("    - %s\n", class_ast->data.class_def.instance_vars->data.variable_list.names[i]);
            }
        } else {
            printf("  No instance variables\n");
        }
    } else {
        printf("✗ Class parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 2: Check instance variable access in methods
    printf("\n--- Test 2: Instance variable access in methods ---\n");
    const char* instance_var_access = "value := 42";
    
    ezom_lexer_init(&lexer, (char*)instance_var_access);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* assignment = ezom_parse_statement(&parser);
    if (assignment) {
        printf("✓ Instance variable assignment parsed\n");
        printf("  Assignment type: %d\n", assignment->type);
        if (assignment->type == AST_ASSIGNMENT) {
            printf("  Variable name: %s\n", assignment->data.assignment.variable->data.variable.name);
            printf("  Variable type: %s\n", assignment->data.assignment.variable->data.variable.is_instance_var ? "instance" : "local");
        }
    } else {
        printf("✗ Instance variable assignment parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    // Test 3: Test instance variable resolution
    printf("\n--- Test 3: Instance variable resolution ---\n");
    const char* variable_ref = "value + 1";
    
    ezom_lexer_init(&lexer, (char*)variable_ref);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* expr = ezom_parse_expression(&parser);
    if (expr) {
        printf("✓ Expression with instance variable parsed\n");
        printf("  Expression type: %d\n", expr->type);
        
        // Check if it's a binary message with variable as receiver
        if (expr->type == AST_BINARY_MESSAGE) {
            printf("  Binary message with receiver type: %d\n", expr->data.message_send.receiver->type);
            if (expr->data.message_send.receiver->type == AST_VARIABLE_DEF) {
                printf("  Variable name: %s\n", expr->data.message_send.receiver->data.variable.name);
            }
        }
    } else {
        printf("✗ Expression parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    printf("\n=========================================\n");
    printf("INSTANCE VARIABLE SYSTEM TESTS COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_instance_variable_system();
    return 0;
}
