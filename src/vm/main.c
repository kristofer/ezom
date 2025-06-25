// ============================================================================
// File: src/vm/main.c
// Main entry point and VM initialization
// ============================================================================

#include "../include/ezom_memory.h"
#include "../include/ezom_object.h"
#include "../include/ezom_primitives.h"
#include "../include/ezom_dispatch.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("EZOM Phase 1 - Basic Object System\n");
    printf("===================================\n");
    
    // Initialize VM components
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_primitives();
    
    // Bootstrap basic classes
    ezom_bootstrap_classes();
    
    // Print memory stats
    ezom_memory_stats();
    
    // Phase 1 Test: Create and manipulate objects
    printf("\nPhase 1 Tests:\n");
    
    // Test 1: Create integers
    printf("1. Creating integers...\n");
    uint24_t int1 = ezom_create_integer(42);
    uint24_t int2 = ezom_create_integer(8);
    
    if (int1 && int2) {
        printf("   Created integers: %d and %d\n", 
               ((ezom_integer_t*)int1)->value,
               ((ezom_integer_t*)int2)->value);
    }
    
    // Test 2: Create strings
    printf("2. Creating strings...\n");
    uint24_t str1 = ezom_create_string("Hello", 5);
    uint24_t str2 = ezom_create_string(" World", 6);
    
    if (str1 && str2) {
        ezom_string_t* s1 = (ezom_string_t*)str1;
        ezom_string_t* s2 = (ezom_string_t*)str2;
        printf("   Created strings: '%.*s' and '%.*s'\n", 
               s1->length, s1->data, s2->length, s2->data);
    }
    
    // Test 3: Integer arithmetic (if classes are set up)
    if (g_integer_class) {
        printf("3. Testing integer arithmetic...\n");
        
        // Create symbols for method selectors
        uint24_t plus_selector = ezom_create_symbol("+", 1);
        uint24_t println_selector = ezom_create_symbol("println", 7);
        
        if (plus_selector) {
            // Test: 42 + 8
            uint24_t result = ezom_send_binary_message(int1, plus_selector, int2);
            if (result) {
                ezom_integer_t* res = (ezom_integer_t*)result;
                printf("   42 + 8 = %d\n", res->value);
                
                // Test println
                if (println_selector) {
                    printf("   Calling println on result: ");
                    ezom_send_unary_message(result, println_selector);
                }
            }
        }
    }
    
    // Test 4: String concatenation
    if (g_string_class) {
        printf("4. Testing string concatenation...\n");
        uint24_t concat_selector = ezom_create_symbol("+", 1);
        
        if (concat_selector) {
            uint24_t result = ezom_send_binary_message(str1, concat_selector, str2);
            if (result) {
                ezom_string_t* res = (ezom_string_t*)result;
                printf("   'Hello' + ' World' = '%.*s'\n", res->length, res->data);
            }
        }
    }
    
    // Print final memory stats
    printf("\nFinal memory statistics:\n");
    ezom_memory_stats();
    
    printf("\nPhase 1 complete!\n");
    return 0;
}

// Bootstrap basic classes (minimal implementation for Phase 1)
void ezom_bootstrap_classes(void) {
    printf("Bootstrapping basic classes...\n");
    
    // Create Object class (bootstrap - self-referential)
    g_object_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_object_class) {
        ezom_init_object(g_object_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* object_class = (ezom_class_t*)g_object_class;
        object_class->superclass = 0; // No superclass
        object_class->method_dict = ezom_create_method_dictionary(8);
        object_class->instance_vars = 0;
        object_class->instance_size = sizeof(ezom_object_t);
        object_class->instance_var_count = 0;
        
        printf("   Object class created at 0x%06X\n", g_object_class);
    }
    
    // Create Integer class
    g_integer_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_integer_class) {
        ezom_init_object(g_integer_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* integer_class = (ezom_class_t*)g_integer_class;
        integer_class->superclass = g_object_class;
        integer_class->method_dict = ezom_create_method_dictionary(16);
        integer_class->instance_vars = 0;
        integer_class->instance_size = sizeof(ezom_integer_t);
        integer_class->instance_var_count = 0;
        
        // Install integer methods
        if (integer_class->method_dict) {
            ezom_install_integer_methods();
        }
        
        printf("   Integer class created at 0x%06X\n", g_integer_class);
    }
    
    // Create String class
    g_string_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_string_class) {
        ezom_init_object(g_string_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* string_class = (ezom_class_t*)g_string_class;
        string_class->superclass = g_object_class;
        string_class->method_dict = ezom_create_method_dictionary(8);
        string_class->instance_vars = 0;
        string_class->instance_size = sizeof(ezom_string_t); // Variable size
        string_class->instance_var_count = 0;
        
        // Install string methods
        if (string_class->method_dict) {
            ezom_install_string_methods();
        }
        
        printf("   String class created at 0x%06X\n", g_string_class);
    }
    
    // Create Symbol class
    g_symbol_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_symbol_class) {
        ezom_init_object(g_symbol_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* symbol_class = (ezom_class_t*)g_symbol_class;
        symbol_class->superclass = g_object_class;
        symbol_class->method_dict = ezom_create_method_dictionary(4);
        symbol_class->instance_vars = 0;
        symbol_class->instance_size = sizeof(ezom_symbol_t);
        symbol_class->instance_var_count = 0;
        
        printf("   Symbol class created at 0x%06X\n", g_symbol_class);
    }
    
    printf("Bootstrap complete!\n");
}

// Install methods in Integer class
void ezom_install_integer_methods(void) {
    ezom_class_t* integer_class = (ezom_class_t*)g_integer_class;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)integer_class->method_dict;
    
    // Helper function to add a method
    void add_method(const char* selector, uint8_t prim_num, uint8_t arg_count) {
        if (dict->size < dict->capacity) {
            ezom_method_t* method = &dict->methods[dict->size];
            method->selector = ezom_create_symbol(selector, strlen(selector));
            method->code = prim_num;
            method->arg_count = arg_count;
            method->flags = EZOM_METHOD_PRIMITIVE;
            dict->size++;
        }
    }
    
    // Install arithmetic methods
    add_method("+", PRIM_INTEGER_ADD, 1);
    add_method("-", PRIM_INTEGER_SUB, 1);
    add_method("*", PRIM_INTEGER_MUL, 1);
    add_method("=", PRIM_INTEGER_EQ, 1);
    add_method("println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in Integer\n", dict->size);
}

// Install methods in String class
void ezom_install_string_methods(void) {
    ezom_class_t* string_class = (ezom_class_t*)g_string_class;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)string_class->method_dict;
    
    void add_method(const char* selector, uint8_t prim_num, uint8_t arg_count) {
        if (dict->size < dict->capacity) {
            ezom_method_t* method = &dict->methods[dict->size];
            method->selector = ezom_create_symbol(selector, strlen(selector));
            method->code = prim_num;
            method->arg_count = arg_count;
            method->flags = EZOM_METHOD_PRIMITIVE;
            dict->size++;
        }
    }
    
    // Install string methods
    add_method("length", PRIM_STRING_LENGTH, 0);
    add_method("+", PRIM_STRING_CONCAT, 1);
    add_method("println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in String\n", dict->size);
}