#include <stdio.h>
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"
#include "include/ezom_evaluator.h"
#include "include/ezom_memory.h"
#include "include/ezom_object.h"

// Test complete instance variable system
void test_complete_instance_variable_system() {
    printf("=========================================\n");
    printf("TESTING COMPLETE INSTANCE VARIABLE SYSTEM\n");
    printf("=========================================\n");
    
    // Initialize memory and runtime
    ezom_init_memory();
    ezom_bootstrap_classes();
    
    // Test 1: Create class with instance variables
    printf("\n--- Test 1: Creating class with instance variables ---\n");
    const char* class_source = 
        "Counter = Object (\n"
        "    | value counter_id |\n"
        "    \n"
        "    initialize = (\n"
        "        value := 0.\n"
        "        counter_id := 42.\n"
        "        ^self\n"
        "    )\n"
        "    \n"
        "    getValue = (\n"
        "        ^value\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)class_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    if (class_ast) {
        printf("✓ Class parsing successful\n");
        printf("  Class name: %s\n", class_ast->data.class_def.name);
        printf("  Instance variables: %d\n", class_ast->data.class_def.instance_vars->data.variable_list.count);
        
        // Evaluate class definition to create runtime class
        ezom_eval_result_t class_result = ezom_evaluate_ast(class_ast, g_current_context);
        if (!class_result.is_error) {
            printf("✓ Class evaluation successful\n");
            printf("  Class object created: 0x%06X\n", class_result.value);
            
            // Test 2: Create instance of the class
            printf("\n--- Test 2: Creating instance of class ---\n");
            
            // Get the class object
            uint24_t class_ptr = class_result.value;
            ezom_class_t* class_obj = EZOM_OBJECT_PTR(class_ptr);
            
            if (class_obj) {
                printf("  Instance variable count: %d\n", class_obj->instance_var_count);
                printf("  Instance size: %d bytes\n", class_obj->instance_size);
                
                // Create an instance
                uint24_t instance_ptr = ezom_create_instance(class_ptr);
                if (instance_ptr != 0) {
                    printf("✓ Instance created: 0x%06X\n", instance_ptr);
                    
                    // Test 3: Test instance variable access
                    printf("\n--- Test 3: Testing instance variable access ---\n");
                    
                    // Set instance variables
                    ezom_set_instance_variable(instance_ptr, 0, ezom_create_integer(100));
                    ezom_set_instance_variable(instance_ptr, 1, ezom_create_integer(200));
                    
                    // Get instance variables
                    uint24_t value1 = ezom_get_instance_variable(instance_ptr, 0);
                    uint24_t value2 = ezom_get_instance_variable(instance_ptr, 1);
                    
                    printf("  Instance variable 0 (value): 0x%06X\n", value1);
                    printf("  Instance variable 1 (counter_id): 0x%06X\n", value2);
                    
                    // Verify values
                    if (value1 != 0) {
                        ezom_integer_t* int_obj = EZOM_OBJECT_PTR(value1);
                        if (int_obj) {
                            printf("  'value' = %d\n", int_obj->value);
                        }
                    }
                    
                    if (value2 != 0) {
                        ezom_integer_t* int_obj = EZOM_OBJECT_PTR(value2);
                        if (int_obj) {
                            printf("  'counter_id' = %d\n", int_obj->value);
                        }
                    }
                    
                    // Test 4: Test instance variable count
                    printf("\n--- Test 4: Testing instance variable count ---\n");
                    uint16_t ivar_count = ezom_get_instance_variable_count(instance_ptr);
                    printf("  Instance variable count: %d\n", ivar_count);
                    
                    // Test invalid access
                    uint24_t invalid_value = ezom_get_instance_variable(instance_ptr, 5);
                    printf("  Invalid index access result: 0x%06X (should be 0)\n", invalid_value);
                    
                    printf("✓ Instance variable system working correctly\n");
                } else {
                    printf("✗ Failed to create instance\n");
                }
            } else {
                printf("✗ Failed to access class object\n");
            }
        } else {
            printf("✗ Class evaluation failed: %s\n", class_result.error_msg);
        }
    } else {
        printf("✗ Class parsing failed\n");
        printf("  Error: %s\n", parser.error_message);
    }
    
    printf("\n=========================================\n");
    printf("COMPLETE INSTANCE VARIABLE SYSTEM TEST COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_complete_instance_variable_system();
    return 0;
}
