// ============================================================================
// File: src/vm/bootstrap.c
// Enhanced class bootstrapping with Boolean hierarchy
// ============================================================================

#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_primitives.h"
#include <stdio.h>
#include <string.h>

// Global class pointers (basic classes)
uint24_t g_object_class = 0;
uint24_t g_class_class = 0;
uint24_t g_integer_class = 0;
uint24_t g_string_class = 0;
uint24_t g_symbol_class = 0;

// Enhanced global class pointers
uint24_t g_array_class = 0;
uint24_t g_block_class = 0;
uint24_t g_boolean_class = 0;
uint24_t g_true_class = 0;
uint24_t g_false_class = 0;
uint24_t g_nil_class = 0;
uint24_t g_context_class = 0;

// Global singleton objects
uint24_t g_nil = 0;
uint24_t g_true = 0;
uint24_t g_false = 0;

// Forward declarations
void ezom_install_object_methods(void);
void ezom_install_integer_methods(void);
void ezom_install_string_methods(void);
void ezom_install_array_methods(void);
void ezom_install_boolean_methods(void);
void ezom_install_block_methods(void);

void ezom_bootstrap_enhanced_classes(void) {
    printf("Bootstrapping enhanced SOM-compatible classes...\n");
    
    // 1. Create Nil class and singleton first
    g_nil_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_nil_class, g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* nil_class = (ezom_class_t*)g_nil_class;
    nil_class->superclass = g_object_class;
    nil_class->method_dict = ezom_create_method_dictionary(4);
    nil_class->instance_vars = 0;
    nil_class->instance_size = sizeof(ezom_object_t);
    nil_class->instance_var_count = 0;
    
    // Create nil singleton
    g_nil = ezom_allocate(sizeof(ezom_object_t));
    ezom_init_object(g_nil, g_nil_class, EZOM_TYPE_NIL);
    
    printf("   Nil class and singleton created\n");
    
    // 2. Create Boolean hierarchy
    g_boolean_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_boolean_class, g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* boolean_class = (ezom_class_t*)g_boolean_class;
    boolean_class->superclass = g_object_class;
    boolean_class->method_dict = ezom_create_method_dictionary(8);
    boolean_class->instance_vars = 0;
    boolean_class->instance_size = sizeof(ezom_object_t);
    boolean_class->instance_var_count = 0;
    
    // Create True class
    g_true_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_true_class, g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* true_class = (ezom_class_t*)g_true_class;
    true_class->superclass = g_boolean_class;
    true_class->method_dict = ezom_create_method_dictionary(8);
    true_class->instance_vars = 0;
    true_class->instance_size = sizeof(ezom_object_t);
    true_class->instance_var_count = 0;
    
    // Create False class
    g_false_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_false_class, g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* false_class = (ezom_class_t*)g_false_class;
    false_class->superclass = g_boolean_class;
    false_class->method_dict = ezom_create_method_dictionary(8);
    false_class->instance_vars = 0;
    false_class->instance_size = sizeof(ezom_object_t);
    false_class->instance_var_count = 0;
    
    // Create singleton instances
    g_true = ezom_allocate(sizeof(ezom_object_t));
    ezom_init_object(g_true, g_true_class, EZOM_TYPE_BOOLEAN);
    
    g_false = ezom_allocate(sizeof(ezom_object_t));
    ezom_init_object(g_false, g_false_class, EZOM_TYPE_BOOLEAN);
    
    printf("   Boolean hierarchy created (True, False)\n");
    
    // 3. Create Array class
    g_array_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_array_class, g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* array_class = (ezom_class_t*)g_array_class;
    array_class->superclass = g_object_class;
    array_class->method_dict = ezom_create_method_dictionary(16);
    array_class->instance_vars = 0;
    array_class->instance_size = sizeof(ezom_array_t);
    array_class->instance_var_count = 0;
    
    printf("   Array class created at 0x%06X\n", g_array_class);
    
    // 4. Create Block class
    g_block_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_block_class, g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* block_class = (ezom_class_t*)g_block_class;
    block_class->superclass = g_object_class;
    block_class->method_dict = ezom_create_method_dictionary(8);
    block_class->instance_vars = 0;
    block_class->instance_size = sizeof(ezom_block_t);
    block_class->instance_var_count = 0;
    
    printf("   Block class created at 0x%06X\n", g_block_class);
    
    // 5. Create Context class
    g_context_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_context_class, g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* context_class = (ezom_class_t*)g_context_class;
    context_class->superclass = g_object_class;
    context_class->method_dict = ezom_create_method_dictionary(4);
    context_class->instance_vars = 0;
    context_class->instance_size = sizeof(ezom_context_t);
    context_class->instance_var_count = 0;
    
    printf("   Context class created at 0x%06X\n", g_context_class);
    
    // Install methods in all classes
    ezom_install_object_methods();
    ezom_install_integer_methods();
    ezom_install_string_methods();
    ezom_install_array_methods();
    ezom_install_boolean_methods();
    ezom_install_block_methods();
    
    printf("Enhanced bootstrap complete! SOM-compatible class hierarchy ready.\n");
}

// Helper function for adding methods to method dictionary
static void add_method_to_dict(ezom_method_dict_t* dict, const char* selector, uint8_t prim_num, uint8_t arg_count) {
    if (dict->size < dict->capacity) {
        ezom_method_t* method = &dict->methods[dict->size];
        method->selector = ezom_create_symbol(selector, strlen(selector));
        method->code = prim_num;
        method->arg_count = arg_count;
        method->flags = EZOM_METHOD_PRIMITIVE;
        dict->size++;
    }
}

// Enhanced method installation functions
void ezom_install_object_methods(void) {
    ezom_class_t* object_class = (ezom_class_t*)g_object_class;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)object_class->method_dict;
    
    // Install Object methods (enhanced)
    add_method_to_dict(dict, "class", PRIM_OBJECT_CLASS, 0);
    add_method_to_dict(dict, "=", PRIM_OBJECT_EQUALS, 1);
    add_method_to_dict(dict, "hash", PRIM_OBJECT_HASH, 0);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    add_method_to_dict(dict, "isNil", PRIM_OBJECT_IS_NIL, 0);
    add_method_to_dict(dict, "notNil", PRIM_OBJECT_NOT_NIL, 0);
    
    printf("      Installed %d methods in Object\n", dict->size);
}

void ezom_install_integer_methods(void) {
    ezom_class_t* integer_class = (ezom_class_t*)g_integer_class;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)integer_class->method_dict;
    
    // Arithmetic operations
    add_method_to_dict(dict, "+", PRIM_INTEGER_ADD, 1);
    add_method_to_dict(dict, "-", PRIM_INTEGER_SUB, 1);
    add_method_to_dict(dict, "*", PRIM_INTEGER_MUL, 1);
    add_method_to_dict(dict, "/", PRIM_INTEGER_DIV, 1);
    add_method_to_dict(dict, "\\", PRIM_INTEGER_MOD, 1);
    add_method_to_dict(dict, "abs", PRIM_INTEGER_ABS, 0);
    
    // Comparison operations
    add_method_to_dict(dict, "<", PRIM_INTEGER_LT, 1);
    add_method_to_dict(dict, ">", PRIM_INTEGER_GT, 1);
    add_method_to_dict(dict, "<=", PRIM_INTEGER_LTE, 1);
    add_method_to_dict(dict, ">=", PRIM_INTEGER_GTE, 1);
    add_method_to_dict(dict, "=", PRIM_INTEGER_EQ, 1);
    add_method_to_dict(dict, "~=", PRIM_INTEGER_NEQ, 1);
    
    // Conversion operations
    add_method_to_dict(dict, "asString", PRIM_INTEGER_AS_STRING, 0);
    
    // Iteration operations
    add_method_to_dict(dict, "to:do:", PRIM_INTEGER_TO_DO, 2);
    add_method_to_dict(dict, "timesRepeat:", PRIM_INTEGER_TIMES_REPEAT, 1);
    
    // Standard operations
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in Integer\n", dict->size);
}

void ezom_install_string_methods(void) {
    ezom_class_t* string_class = (ezom_class_t*)g_string_class;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)string_class->method_dict;
    
    add_method_to_dict(dict, "length", PRIM_STRING_LENGTH, 0);
    add_method_to_dict(dict, "+", PRIM_STRING_CONCAT, 1);
    add_method_to_dict(dict, "=", PRIM_STRING_EQUALS, 1);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in String\n", dict->size);
}

void ezom_install_array_methods(void) {
    ezom_class_t* array_class = (ezom_class_t*)g_array_class;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)array_class->method_dict;
    
    add_method_to_dict(dict, "at:", PRIM_ARRAY_AT, 1);
    add_method_to_dict(dict, "at:put:", PRIM_ARRAY_AT_PUT, 2);
    add_method_to_dict(dict, "length", PRIM_ARRAY_LENGTH, 0);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in Array\n", dict->size);
}

void ezom_install_boolean_methods(void) {
    // Install methods for True class
    ezom_class_t* true_class = (ezom_class_t*)g_true_class;
    ezom_method_dict_t* true_dict = (ezom_method_dict_t*)true_class->method_dict;
    
    add_method_to_dict(true_dict, "ifTrue:", PRIM_TRUE_IF_TRUE, 1);
    add_method_to_dict(true_dict, "ifFalse:", PRIM_TRUE_IF_FALSE, 1);
    add_method_to_dict(true_dict, "ifTrue:ifFalse:", PRIM_TRUE_IF_TRUE_IF_FALSE, 2);
    add_method_to_dict(true_dict, "not", PRIM_BOOLEAN_NOT, 0);
    add_method_to_dict(true_dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    // Install methods for False class
    ezom_class_t* false_class = (ezom_class_t*)g_false_class;
    ezom_method_dict_t* false_dict = (ezom_method_dict_t*)false_class->method_dict;
    
    add_method_to_dict(false_dict, "ifTrue:", PRIM_FALSE_IF_TRUE, 1);
    add_method_to_dict(false_dict, "ifFalse:", PRIM_FALSE_IF_FALSE, 1);
    add_method_to_dict(false_dict, "ifTrue:ifFalse:", PRIM_FALSE_IF_TRUE_IF_FALSE, 2);
    add_method_to_dict(false_dict, "not", PRIM_BOOLEAN_NOT, 0);
    add_method_to_dict(false_dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed methods in True (%d) and False (%d)\n", 
           true_dict->size, false_dict->size);
}

void ezom_install_block_methods(void) {
    ezom_class_t* block_class = (ezom_class_t*)g_block_class;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)block_class->method_dict;
    
    add_method_to_dict(dict, "value", PRIM_BLOCK_VALUE, 0);
    add_method_to_dict(dict, "value:", PRIM_BLOCK_VALUE_WITH, 1);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in Block\n", dict->size);
}
