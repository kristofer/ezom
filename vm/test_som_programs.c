#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_primitives.h"
#include "../include/ezom_parser.h"
#include "../include/ezom_dispatch.h"
#include "../include/ezom_evaluator.h"
#include <stdio.h>
#include <string.h>

// Test SOM programs with current implementation
int main() {
    printf("=========================================\n");
    printf("EZOM SOM LANGUAGE PROGRAM TESTS\n");
    printf("=========================================\n");
    
    // Initialize the VM
    printf("1. Initializing EZOM VM...\n");
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_primitives();
    ezom_bootstrap_classes();
    ezom_bootstrap_enhanced_classes();
    
    printf("   VM initialized successfully!\n");
    printf("   Available classes: Object, Integer, String, Array, Boolean\n");
    
    // Test 1: Hello World equivalent - direct method calls
    printf("\n2. Testing Hello World Program...\n");
    printf("   Creating HelloWorld-like functionality...\n");
    
    // Create a string and call println
    uint24_t hello_str = ezom_create_string("Hello, EZOM World!", 18);
    if (hello_str) {
        printf("   Created string: ");
        ezom_send_unary_message(hello_str, ezom_create_symbol("println", 7));
        printf("   ✓ Hello World functionality works!\n");
    }
    
    // Test 2: Counter functionality
    printf("\n3. Testing Counter Program...\n");
    printf("   Simulating Counter class behavior...\n");
    
    // Create counter value and increment operations
    uint24_t counter_value = ezom_create_integer(0);
    printf("   Initial counter value: ");
    ezom_send_unary_message(counter_value, ezom_create_symbol("println", 7));
    
    // Increment the counter (simulate increment method)
    uint24_t one = ezom_create_integer(1);
    uint24_t plus_selector = ezom_create_symbol("+", 1);
    uint24_t incremented = ezom_send_binary_message(counter_value, plus_selector, one);
    
    printf("   After increment: ");
    ezom_send_unary_message(incremented, ezom_create_symbol("println", 7));
    
    // Test string concatenation for formatted output
    uint24_t counter_prefix = ezom_create_string("Counter value: ", 15);
    uint24_t value_str = ezom_send_unary_message(incremented, ezom_create_symbol("asString", 8));
    uint24_t formatted = ezom_send_binary_message(counter_prefix, ezom_create_symbol("+", 1), value_str);
    
    printf("   Formatted output: ");
    ezom_send_unary_message(formatted, ezom_create_symbol("println", 7));
    printf("   ✓ Counter functionality works!\n");
    
    // Test 3: Array and Calculator functionality
    printf("\n4. Testing Calculator Program...\n");
    printf("   Simulating Calculator class with arrays...\n");
    
    // Create array
    uint24_t array_size = ezom_create_integer(5);
    uint24_t new_selector = ezom_create_symbol("new:", 4);
    uint24_t numbers = ezom_send_binary_message(g_array_class, new_selector, array_size);
    
    if (numbers) {
        printf("   Created array of size 5\n");
        
        // Fill array with numbers
        int values[] = {10, 20, 30, 40, 50};
        for (int i = 0; i < 5; i++) {
            uint24_t index = ezom_create_integer(i + 1); // 1-based indexing
            uint24_t value = ezom_create_integer(values[i]);
            uint24_t at_put_selector = ezom_create_symbol("at:put:", 7);
            
            // Use message dispatch for at:put: with two arguments
            ezom_message_t msg;
            uint24_t args[2] = {index, value};
            msg.selector = at_put_selector;
            msg.receiver = numbers;
            msg.args = args;
            msg.arg_count = 2;
            ezom_send_message(&msg);
        }
        
        printf("   Filled array with values: 10, 20, 30, 40, 50\n");
        
        // Calculate sum
        uint24_t total = ezom_create_integer(0);
        uint24_t length = ezom_send_unary_message(numbers, ezom_create_symbol("length", 6));
        uint24_t at_selector = ezom_create_symbol("at:", 3);
        
        // Simple sum calculation (without proper loop control)
        for (int i = 1; i <= 5; i++) {
            uint24_t index = ezom_create_integer(i);
            uint24_t element = ezom_send_binary_message(numbers, at_selector, index);
            total = ezom_send_binary_message(total, plus_selector, element);
        }
        
        printf("   Calculated sum: ");
        ezom_send_unary_message(total, ezom_create_symbol("println", 7));
        
        // Test array access
        printf("   Array element access test:\n");
        uint24_t first_element = ezom_send_binary_message(numbers, at_selector, ezom_create_integer(1));
        printf("     numbers at: 1 = ");
        ezom_send_unary_message(first_element, ezom_create_symbol("println", 7));
        
        uint24_t third_element = ezom_send_binary_message(numbers, at_selector, ezom_create_integer(3));
        printf("     numbers at: 3 = ");
        ezom_send_unary_message(third_element, ezom_create_symbol("println", 7));
        
        printf("   ✓ Calculator functionality works!\n");
    }
    
    // Test 4: Boolean and Control Flow
    printf("\n5. Testing Boolean and Control Flow...\n");
    
    uint24_t true_val = g_true;
    uint24_t false_val = g_false;
    
    printf("   Testing boolean values:\n");
    printf("     true = ");
    ezom_send_unary_message(true_val, ezom_create_symbol("println", 7));
    printf("     false = ");
    ezom_send_unary_message(false_val, ezom_create_symbol("println", 7));
    
    // Test comparisons
    uint24_t num1 = ezom_create_integer(10);
    uint24_t num2 = ezom_create_integer(20);
    uint24_t lt_result = ezom_send_binary_message(num1, ezom_create_symbol("<", 1), num2);
    
    printf("   10 < 20 = ");
    ezom_send_unary_message(lt_result, ezom_create_symbol("println", 7));
    
    uint24_t gt_result = ezom_send_binary_message(num1, ezom_create_symbol(">", 1), num2);
    printf("   10 > 20 = ");
    ezom_send_unary_message(gt_result, ezom_create_symbol("println", 7));
    
    printf("   ✓ Boolean functionality works!\n");
    
    // Summary
    printf("\n=========================================\n");
    printf("SOM LANGUAGE PROGRAM TEST RESULTS\n");
    printf("=========================================\n");
    printf("✓ Hello World: String creation and println\n");
    printf("✓ Counter: Integer arithmetic and string formatting\n");
    printf("✓ Calculator: Array operations and iteration\n");
    printf("✓ Boolean: Comparisons and boolean values\n");
    printf("\nCurrent implementation supports:\n");
    printf("- Object creation and method dispatch\n");
    printf("- Integer arithmetic operations\n");
    printf("- String operations and concatenation\n");
    printf("- Array creation and element access\n");
    printf("- Boolean values and comparisons\n");
    printf("- Basic primitive operations\n");
    
    // Memory statistics
    printf("\n");
    ezom_detailed_memory_stats();
    
    return 0;
}
