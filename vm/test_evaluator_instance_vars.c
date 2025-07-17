#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezom_memory.h"
#include "ezom_object.h"
#include "ezom_parser.h"
#include "ezom_evaluator.h"
#include "ezom_ast.h"

// Test evaluator with instance variables
void test_evaluator_instance_variables() {
    printf("=========================================\n");
    printf("TESTING EVALUATOR WITH INSTANCE VARIABLES\n");
    printf("=========================================\n");
    
    // Initialize memory and runtime
    ezom_init_memory();
    ezom_bootstrap_classes();
    
    // Test 1: Create class with instance variables
    printf("\n--- Test 1: Class with instance variables ---\n");
    const char* class_source = 
        "TestClass = Object (\n"
        "    | x y |\n"
        "    \n"
        "    initialize = (\n"
        "        x := 10.\n"
        "        y := 20.\n"
        "    )\n"
        "    \n"
        "    getX = (\n"
        "        ^ x\n"
        "    )\n"
        "    \n"
        "    setX: newX = (\n"
        "        x := newX\n"
        "    )\n"
        ")\n";
    
    // Parse the class
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(class_source);
    if (class_ast) {
        printf("✓ Class parsing successful\n");
        
        // Evaluate the class definition
        ezom_eval_result_t result = ezom_evaluate_ast(class_ast, 0);
        if (result.success) {
            printf("✓ Class evaluation successful\n");
            
            // Get the class object
            uint24_t class_ptr = ezom_get_global_by_name("TestClass");
            if (class_ptr) {
                printf("✓ Class object found: 0x%06X\n", class_ptr);
                
                // Test 2: Create instance
                printf("\n--- Test 2: Creating instance ---\n");
                ezom_object_t* class_obj = EZOM_OBJECT_PTR(class_ptr);
                
                // Create instance
                uint24_t instance_ptr = ezom_create_instance(class_ptr);
                if (instance_ptr) {
                    printf("✓ Instance created: 0x%06X\n", instance_ptr);
                    
                    // Test 3: Call initialize method
                    printf("\n--- Test 3: Calling initialize method ---\n");
                    ezom_object_t* instance_obj = EZOM_OBJECT_PTR(instance_ptr);
                    
                    // Create context for method call
                    uint24_t context_ptr = ezom_create_context(instance_ptr, 0, 0);
                    if (context_ptr) {
                        printf("✓ Context created: 0x%06X\n", context_ptr);
                        
                        // Test 4: Access instance variables
                        printf("\n--- Test 4: Testing instance variable access ---\n");
                        uint24_t x_value = ezom_get_instance_variable(instance_ptr, 0);
                        uint24_t y_value = ezom_get_instance_variable(instance_ptr, 1);
                        
                        printf("  Instance variable 'x' (index 0): 0x%06X\n", x_value);
                        printf("  Instance variable 'y' (index 1): 0x%06X\n", y_value);
                        
                        // Test 5: Set instance variables
                        printf("\n--- Test 5: Setting instance variables ---\n");
                        ezom_set_instance_variable(instance_ptr, 0, ezom_create_integer(42));
                        ezom_set_instance_variable(instance_ptr, 1, ezom_create_integer(99));
                        
                        x_value = ezom_get_instance_variable(instance_ptr, 0);
                        y_value = ezom_get_instance_variable(instance_ptr, 1);
                        
                        printf("  Updated 'x': 0x%06X\n", x_value);
                        printf("  Updated 'y': 0x%06X\n", y_value);
                        
                        // Verify the values
                        if (x_value != 0 && y_value != 0) {
                            ezom_integer_t* x_int = EZOM_OBJECT_PTR(x_value);
                            ezom_integer_t* y_int = EZOM_OBJECT_PTR(y_value);
                            printf("  'x' value: %d\n", x_int->value);
                            printf("  'y' value: %d\n", y_int->value);
                            
                            if (x_int->value == 42 && y_int->value == 99) {
                                printf("✓ Instance variables working correctly\n");
                            } else {
                                printf("✗ Instance variable values incorrect\n");
                            }
                        } else {
                            printf("✗ Instance variables not set correctly\n");
                        }
                    } else {
                        printf("✗ Failed to create context\n");
                    }
                } else {
                    printf("✗ Failed to create instance\n");
                }
            } else {
                printf("✗ Class object not found\n");
            }
        } else {
            printf("✗ Class evaluation failed\n");
        }
    } else {
        printf("✗ Class parsing failed\n");
    }
    
    printf("\n=========================================\n");
    printf("EVALUATOR INSTANCE VARIABLE TEST COMPLETE\n");
    printf("=========================================\n");
}

int main() {
    test_evaluator_instance_variables();
    return 0;
}
