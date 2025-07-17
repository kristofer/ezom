#include <stdio.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"
#include "include/ezom_evaluator.h"

// Test instance variable system with context
void test_instance_variable_system_with_context() {
    printf("=========================================\n");
    printf("TESTING INSTANCE VARIABLE SYSTEM WITH CONTEXT\n");
    printf("=========================================\n");
    
    // Test 1: Parse class and set up context
    printf("\n--- Test 1: Class parsing with context setup ---\n");
    const char* class_with_ivars = 
        "Counter = Object (\n"
        "    | value counter_id |\n"
        "    \n"
        "    initialize = (\n"
        "        value := 0.\n"
        "        counter_id := 'counter'.\n"
        "        ^self\n"
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
                printf("    %d: %s\n", i, class_ast->data.class_def.instance_vars->data.variable_list.names[i]);
            }
        }
        
        // Test variable resolution
        printf("\n--- Test 2: Variable resolution ---\n");
        ezom_variable_context_t context;
        context.class_def = class_ast;
        context.method_def = NULL;
        context.current_locals = NULL;
        context.current_parameters = NULL;
        
        // Test resolving instance variables
        printf("Testing variable resolution:\n");
        ezom_variable_type_t type1 = ezom_resolve_variable_type("value", &context);
        printf("  'value' -> type: %d (0=instance, 1=local, 2=param, 3=unknown)\n", type1);
        
        ezom_variable_type_t type2 = ezom_resolve_variable_type("counter_id", &context);
        printf("  'counter_id' -> type: %d\n", type2);
        
        ezom_variable_type_t type3 = ezom_resolve_variable_type("unknown_var", &context);
        printf("  'unknown_var' -> type: %d\n", type3);
        
        // Test instance variable index lookup
        printf("\nInstance variable indices:\n");
        int index1 = ezom_find_instance_variable_index("value", class_ast);
        int index2 = ezom_find_instance_variable_index("counter_id", class_ast);
        int index3 = ezom_find_instance_variable_index("unknown", class_ast);
        
        printf("  'value' index: %d\n", index1);
        printf("  'counter_id' index: %d\n", index2);
        printf("  'unknown' index: %d\n", index3);
        
        // Test context-aware variable creation
        printf("\n--- Test 3: Context-aware variable creation ---\n");
        ezom_ast_node_t* var1 = ezom_create_variable_with_context("value", &context);
        ezom_ast_node_t* var2 = ezom_create_variable_with_context("counter_id", &context);
        ezom_ast_node_t* var3 = ezom_create_variable_with_context("unknown", &context);
        
        if (var1) {
            printf("  'value' -> instance_var: %s, local: %s, index: %d\n", 
                   var1->data.variable.is_instance_var ? "true" : "false",
                   var1->data.variable.is_local ? "true" : "false",
                   var1->data.variable.index);
        }
        
        if (var2) {
            printf("  'counter_id' -> instance_var: %s, local: %s, index: %d\n", 
                   var2->data.variable.is_instance_var ? "true" : "false",
                   var2->data.variable.is_local ? "true" : "false",
                   var2->data.variable.index);
        }
        
        if (var3) {
            printf("  'unknown' -> instance_var: %s, local: %s, index: %d\n", 
                   var3->data.variable.is_instance_var ? "true" : "false",
                   var3->data.variable.is_local ? "true" : "false",
                   var3->data.variable.index);
        }
        
        // Test with method context
        printf("\n--- Test 4: Method context with locals ---\n");
        
        // Get the first method (initialize)
        if (class_ast->data.class_def.instance_methods && 
            class_ast->data.class_def.instance_methods->data.statement_list.count > 0) {
            
            ezom_ast_node_t* method = class_ast->data.class_def.instance_methods->data.statement_list.statements;
            printf("  Method found: %s\n", method->data.method_def.selector);
            
            // Update context with method information
            context.method_def = method;
            context.current_locals = method->data.method_def.locals;
            context.current_parameters = method->data.method_def.parameters;
            
            // Test variable resolution with method context
            ezom_variable_type_t type_in_method = ezom_resolve_variable_type("value", &context);
            printf("  'value' in method context -> type: %d\n", type_in_method);
        }
        
    } else {
        printf("✗ Class parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    printf("\n=========================================\n");
    printf("INSTANCE VARIABLE SYSTEM WITH CONTEXT COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_instance_variable_system_with_context();
    return 0;
}
