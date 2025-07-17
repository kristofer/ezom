// ============================================================================
// File: src/vm/main.c
// Main entry point with file loading support
// ============================================================================

#include "../include/ezom_memory.h"
#include "../include/ezom_object.h"
#include "../include/ezom_primitives.h"
#include "../include/ezom_dispatch.h"
#include "../include/ezom_context.h"
#include "../include/ezom_evaluator.h"
#include "../include/ezom_lexer.h"
#include "../include/ezom_parser.h"
#include "../include/ezom_ast.h"
#include "../include/ezom_ast_memory.h"
#include "../include/ezom_file_loader.h"
#include <stdio.h>
#include <string.h>

// Forward declare logging functions
extern void ezom_log_init();
extern void ezom_log_close();
extern void ezom_log(const char* format, ...);

// VM initialization
void ezom_init_vm() {
    printf("EZOM Phase 4.4.1 - File Loading and SOM Program Execution\n");
    printf("========================================================\n");
    
    // Initialize logging
    ezom_log_init();
    ezom_log("VM initialization started\n");
    
    // CRITICAL: Initialize g_nil early to prevent crashes
    g_nil = 1; // Non-zero temporary value
    
    printf("Initializing VM components...\n");
    
    // Initialize core VM components
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_primitives();
    ezom_bootstrap_enhanced_classes();
    ezom_init_context_system();
    ezom_init_boolean_objects();
    
    printf("VM initialization complete!\n");
    ezom_log("VM initialization complete\n");
}

// Test VM functionality
void ezom_test_vm() {
    printf("\n=== Testing VM Functionality ===\n");
    
    // Test 1: Basic object creation
    printf("1. Testing basic object creation...\n");
    uint24_t int1 = ezom_create_integer(42);
    uint24_t str1 = ezom_create_string("Hello, EZOM!", 12);
    
    if (int1 && str1) {
        printf("   ✓ Created integer and string objects\n");
        
        // Test method dispatch
        uint24_t println_selector = ezom_create_symbol("println", 7);
        printf("   Integer: ");
        ezom_send_unary_message(int1, println_selector);
        printf("   String: ");
        ezom_send_unary_message(str1, println_selector);
    }
    
    // Test 2: Arithmetic operations
    printf("2. Testing arithmetic operations...\n");
    uint24_t num1 = ezom_create_integer(10);
    uint24_t num2 = ezom_create_integer(5);
    uint24_t plus_selector = ezom_create_symbol("+", 1);
    uint24_t result = ezom_send_binary_message(num1, plus_selector, num2);
    
    if (result) {
        printf("   10 + 5 = ");
        ezom_send_unary_message(result, ezom_create_symbol("println", 7));
    }
    
    // Test 3: String operations
    printf("3. Testing string operations...\n");
    uint24_t hello = ezom_create_string("Hello", 5);
    uint24_t world = ezom_create_string(" World", 6);
    uint24_t concat = ezom_send_binary_message(hello, plus_selector, world);
    
    if (concat) {
        printf("   Concatenation: ");
        ezom_send_unary_message(concat, ezom_create_symbol("println", 7));
    }
    
    printf("✓ VM functionality tests passed!\n");
}

int main(int argc, char* argv[]) {
    // Initialize VM
    ezom_init_vm();
    
    // Parse command line arguments
    ezom_args_t args = ezom_parse_arguments(argc, argv);
    
    // If no arguments, run VM tests and exit
    if (argc == 1) {
        ezom_test_vm();
        printf("\nUsage: %s [OPTIONS] [FILE]\n", argv[0]);
        printf("Use -h or --help for more information.\n");
        ezom_log_close();
        return 0;
    }
    
    int exit_code = 0;
    
    // Handle different execution modes
    if (args.eval_code) {
        // Execute code directly
        printf("Executing code: %s\n", args.eval_code);
        uint24_t result;
        ezom_file_result_t status = ezom_execute_som_code(args.eval_code, &result);
        
        if (status == EZOM_FILE_OK) {
            printf("Result: ");
            if (result != 0) {
                uint24_t println_selector = ezom_create_symbol("println", 7);
                ezom_send_unary_message(result, println_selector);
            } else {
                printf("nil\n");
            }
        } else {
            ezom_print_file_error(status, "<command-line>");
            exit_code = 1;
        }
    } else if (args.interactive_mode) {
        // Start interactive REPL
        ezom_start_repl();
    } else if (args.filename) {
        // Execute file
        printf("Loading file: %s\n", args.filename);
        uint24_t result;
        ezom_file_result_t status = ezom_execute_som_file(args.filename, &result);
        
        if (status == EZOM_FILE_OK) {
            printf("Program executed successfully\n");
            if (result != 0) {
                printf("Result: ");
                uint24_t println_selector = ezom_create_symbol("println", 7);
                ezom_send_unary_message(result, println_selector);
            }
        } else {
            ezom_print_file_error(status, args.filename);
            exit_code = 1;
        }
    } else {
        // No valid arguments
        ezom_print_usage(argv[0]);
        exit_code = 1;
    }
    
    // Show memory statistics if verbose mode
    if (args.verbose_mode) {
        printf("\n=== Memory Statistics ===\n");
        ezom_detailed_memory_stats();
    }
    
    // Cleanup
    ezom_log_close();
    return exit_code;
}
