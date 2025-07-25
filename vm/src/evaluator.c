// ============================================================================
// File: src/evaluator.c
// AST evaluation engine implementation
// ============================================================================

#include "../include/ezom_evaluator.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_dispatch.h"
#include "../include/ezom_primitives.h"
#include "../include/ezom_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global symbol table for variables (simplified) - reduced size for ez80
#define MAX_GLOBALS 16
static struct {
    char* name;
    uint24_t value;
} g_globals[MAX_GLOBALS];
static uint16_t g_global_count = 0;

// Forward declarations
static ezom_eval_result_t ezom_evaluate_arguments_internal(ezom_ast_node_t* arg_list, 
                                                         uint24_t* arg_values, 
                                                         uint8_t max_args, 
                                                         uint24_t context);

// Forward declarations for context accessors
uint24_t ezom_get_context_receiver(uint24_t context_ptr);
uint24_t ezom_get_local_variable(uint24_t context_ptr, uint16_t index);
uint24_t ezom_get_parameter(uint24_t context_ptr, uint16_t index);
void ezom_set_local_variable(uint24_t context_ptr, uint16_t index, uint24_t value);

void ezom_evaluator_init(void) {
    printf("EZOM: Initializing evaluator...\n");
    
    // Initialize with smaller array size
    g_global_count = 0;
    
    printf("   Evaluator initialization complete.\n");
}

void ezom_evaluator_cleanup(void) {
    for (uint16_t i = 0; i < g_global_count; i++) {
        free(g_globals[i].name);
    }
    g_global_count = 0;
}

// Main evaluation entry point
ezom_eval_result_t ezom_evaluate_ast(ezom_ast_node_t* node, uint24_t context) {
    printf("   Debug: ezom_evaluate_ast called with node=0x%06lX, context=0x%06lX\n", 
           (unsigned long)node, context);
    
    if (!node) {
        printf("   Debug: node is NULL, returning g_nil\n");
        return ezom_make_result(g_nil);
    }
    
    printf("   Debug: accessing node->type...\n");
    printf("   Debug: node->type = %d\n", node->type);
    
    switch (node->type) {
        case AST_CLASS_DEF:
            return ezom_evaluate_class_definition(node, context);
            
        case AST_METHOD_DEF:
            // Method definitions are handled during class creation
            return ezom_make_result(g_nil);
            
        case AST_MESSAGE_SEND:
            return ezom_evaluate_message_send(node, context);
            
        case AST_UNARY_MESSAGE:
            return ezom_evaluate_unary_message(node, context);
            
        case AST_BINARY_MESSAGE:
            return ezom_evaluate_binary_message(node, context);
            
        case AST_KEYWORD_MESSAGE:
            return ezom_evaluate_keyword_message(node, context);
            
        case AST_LITERAL:
            return ezom_evaluate_literal(node, context);
            
        case AST_IDENTIFIER:
            return ezom_evaluate_identifier(node, context);
            
        case AST_VARIABLE_DEF:
            return ezom_evaluate_variable(node, context);
            
        case AST_ASSIGNMENT:
            return ezom_evaluate_assignment(node, context);
            
        case AST_RETURN:
            return ezom_evaluate_return(node, context);
            
        case AST_BLOCK:
            return ezom_evaluate_block_literal(node, context);
            
        case AST_STATEMENT_LIST:
            return ezom_evaluate_statement_list(node, context);
            
        default:
            return ezom_make_error_result("Unknown AST node type");
    }
}

ezom_eval_result_t ezom_evaluate_expression(ezom_ast_node_t* expr, uint24_t context) {
    return ezom_evaluate_ast(expr, context);
}

ezom_eval_result_t ezom_evaluate_statement(ezom_ast_node_t* stmt, uint24_t context) {
    return ezom_evaluate_ast(stmt, context);
}

ezom_eval_result_t ezom_evaluate_statement_list(ezom_ast_node_t* statements, uint24_t context) {
    if (!statements || statements->type != AST_STATEMENT_LIST) {
        return ezom_make_result(g_nil);
    }
    
    ezom_eval_result_t result = ezom_make_result(g_nil);
    ezom_ast_node_t* current = statements->data.statement_list.statements;
    
    while (current) {
        result = ezom_evaluate_statement(current, context);
        
        // Handle early returns
        if (result.is_return || result.is_error) {
            break;
        }
        
        current = current->next;
    }
    
    return result;
}

// Message send evaluation
ezom_eval_result_t ezom_evaluate_message_send(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_MESSAGE_SEND) {
        return ezom_make_error_result("Invalid message send node");
    }
    
    // Evaluate receiver
    ezom_eval_result_t receiver_result = ezom_evaluate_expression(node->data.message_send.receiver, context);
    if (receiver_result.is_error) {
        return receiver_result;
    }
    
    const char* selector = node->data.message_send.selector;
    uint24_t receiver = receiver_result.value;
    
    // Handle different message types
    if (node->data.message_send.arg_count == 0) {
        // Unary message
        return ezom_eval_send_unary_message(receiver, selector, context);
    } else if (strstr(selector, ":") != NULL) {
        // Keyword message (selector contains colon)
        uint24_t arg_values[16]; // Max 16 arguments
        ezom_eval_result_t arg_result = ezom_evaluate_arguments(node->data.message_send.arguments, 
                                                              arg_values, 16, context);
        if (arg_result.is_error) {
            return arg_result;
        }
        uint8_t arg_count = (uint8_t)arg_result.value;
        return ezom_eval_send_keyword_message(receiver, selector, arg_values, arg_count, context);
    } else {
        // Binary message (typically)
        ezom_eval_result_t arg_result = ezom_evaluate_expression(node->data.message_send.arguments, context);
        if (arg_result.is_error) {
            return arg_result;
        }
        return ezom_eval_send_binary_message(receiver, selector, arg_result.value, context);
    }
}

// Literal evaluation
ezom_eval_result_t ezom_evaluate_literal(ezom_ast_node_t* node, uint24_t context) {
    printf("   Debug: evaluate_literal called with node=0x%06lX\n", (unsigned long)node);
    
    if (!node) {
        printf("   Debug: node is NULL!\n");
        return ezom_make_error_result("Invalid literal node");
    }
    
    printf("   Debug: node->type = %d (should be %d for AST_LITERAL)\n", node->type, AST_LITERAL);
    
    if (node->type != AST_LITERAL) {
        printf("   Debug: node type mismatch!\n");
        return ezom_make_error_result("Invalid literal node");
    }
    
    printf("   Debug: accessing node->data.literal.type...\n");
    switch (node->data.literal.type) {
        case LITERAL_INTEGER:
            {
                uint24_t int_obj = ezom_create_integer(node->data.literal.value.integer_value);
                return ezom_make_result(int_obj);
            }
            
        case LITERAL_STRING:
            {
                const char* str_val = node->data.literal.value.string_value;
                uint24_t str_obj = ezom_create_string(str_val, strlen(str_val));
                return ezom_make_result(str_obj);
            }
            
        case LITERAL_SYMBOL:
            {
                const char* sym_val = node->data.literal.value.symbol_value;
                uint24_t sym_obj = ezom_create_symbol(sym_val, strlen(sym_val));
                return ezom_make_result(sym_obj);
            }
            
        case LITERAL_ARRAY:
            // Arrays would need special handling - simplified for now
            return ezom_make_result(g_nil);
            
        default:
            return ezom_make_error_result("Unknown literal type");
    }
}

// Identifier evaluation (variable lookup)
ezom_eval_result_t ezom_evaluate_identifier(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_IDENTIFIER) {
        return ezom_make_error_result("Invalid identifier node");
    }
    
    const char* name = node->data.identifier.name;
    printf("   Debug: Looking up identifier '%s'\n", name);
    
    // Look up in current context first
    uint24_t value = ezom_lookup_variable(name, context);
    if (value != g_nil) {
        printf("   Debug: Found in context: 0x%06X\n", value);
        return ezom_make_result(value);
    }
    
    // Look up in globals
    value = ezom_lookup_global(name);
    if (value != g_nil) {
        printf("   Debug: Found in globals: 0x%06X\n", value);
        return ezom_make_result(value);
    }
    
    // Special cases for reserved words
    if (strcmp(name, "nil") == 0) {
        printf("   Debug: Returning g_nil: 0x%06X\n", g_nil);
        return ezom_make_result(g_nil);
    }
    if (strcmp(name, "true") == 0) {
        printf("   Debug: Returning g_true: 0x%06X\n", g_true);
        return ezom_make_result(g_true);
    }
    if (strcmp(name, "false") == 0) {
        printf("   Debug: Returning g_false: 0x%06X\n", g_false);
        return ezom_make_result(g_false);
    }
    if (strcmp(name, "self") == 0) {
        // Return current receiver
        if (context) {
            ezom_context_t* ctx = (ezom_context_t*)EZOM_OBJECT_PTR(context);
            printf("   Debug: Returning self: 0x%06X\n", ctx->receiver);
            return ezom_make_result(ctx->receiver);
        }
    }
    
    printf("   Debug: Undefined variable: '%s'\n", name);
    return ezom_make_error_result("Undefined variable");
}

// Enhanced assignment evaluation with full variable type support
ezom_eval_result_t ezom_evaluate_assignment(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_ASSIGNMENT) {
        return ezom_make_error_result("Invalid assignment node");
    }
    
    printf("Evaluating assignment\n");
    
    // Evaluate the value first
    ezom_eval_result_t value_result = ezom_evaluate_ast(node->data.assignment.value, context);
    if (value_result.is_error) {
        return value_result;
    }
    
    printf("Assignment value evaluated: 0x%06X\n", value_result.value);
    
    // Handle different variable types
    if (node->data.assignment.variable->type == AST_VARIABLE_DEF) {
        // Variable with type information
        ezom_ast_node_t* var_node = node->data.assignment.variable;
        
        if (var_node->data.variable.is_instance_var) {
            // Instance variable assignment
            printf("Assigning to instance variable '%s' at index %d\n", 
                   var_node->data.variable.name, var_node->data.variable.index);
            
            return ezom_evaluate_instance_variable_assignment(var_node, value_result.value, context);
        } else if (var_node->data.variable.is_local) {
            // Local variable assignment
            printf("Assigning to local variable '%s' at index %d\n", 
                   var_node->data.variable.name, var_node->data.variable.index);
            
            ezom_context_set_local(context, var_node->data.variable.index, value_result.value);
        } else {
            // Parameter assignment (usually not allowed, but handle gracefully)
            printf("Assigning to parameter '%s' at index %d\n", 
                   var_node->data.variable.name, var_node->data.variable.index);
            
            ezom_context_set_local(context, var_node->data.variable.index, value_result.value);
        }
    } else if (node->data.assignment.variable->type == AST_IDENTIFIER) {
        // Legacy identifier-based assignment - resolve variable type
        const char* var_name = node->data.assignment.variable->data.identifier.name;
        
        printf("Assigning to identifier '%s' (resolving type)\n", var_name);
        
        // Try to resolve variable type and assign appropriately
        if (!ezom_assign_resolved_variable(var_name, value_result.value, context)) {
            // Fall back to globals
            printf("Creating global variable '%s'\n", var_name);
            ezom_set_global(var_name, value_result.value);
        }
    } else {
        return ezom_make_error_result("Can only assign to variables");
    }
    
    return value_result;
}

// Return statement evaluation
ezom_eval_result_t ezom_evaluate_return(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_RETURN) {
        return ezom_make_error_result("Invalid return node");
    }
    
    if (node->data.return_stmt.expression) {
        ezom_eval_result_t result = ezom_evaluate_expression(node->data.return_stmt.expression, context);
        if (result.is_error) {
            return result;
        }
        return ezom_make_return_result(result.value);
    } else {
        return ezom_make_return_result(g_nil);
    }
}

// Block literal evaluation
ezom_eval_result_t ezom_evaluate_block_literal(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_BLOCK) {
        return ezom_make_error_result("Invalid block node");
    }
    
    // Create block object with current context as outer context
    uint24_t block_obj = ezom_create_ast_block(node, context);
    return ezom_make_result(block_obj);
}

// Enhanced class definition evaluation
ezom_eval_result_t ezom_evaluate_class_definition(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_CLASS_DEF) {
        return ezom_make_error_result("Invalid class definition node");
    }
    
    printf("Defining class: %s\n", node->data.class_def.name);
    
    // Determine superclass
    uint24_t superclass = g_object_class;
    if (node->data.class_def.superclass && node->data.class_def.superclass->type == AST_IDENTIFIER) {
        uint24_t super_obj = ezom_lookup_global(node->data.class_def.superclass->data.identifier.name);
        if (super_obj != g_nil) {
            superclass = super_obj;
        }
    }
    
    // Count instance variables
    uint16_t instance_var_count = 0;
    if (node->data.class_def.instance_vars) {
        instance_var_count = ezom_ast_count_locals(node->data.class_def.instance_vars);
    }
    
    // Create class object with proper inheritance
    uint24_t class_obj = ezom_create_class_with_inheritance(
        node->data.class_def.name, 
        superclass, 
        instance_var_count
    );
    
    if (!class_obj) {
        return ezom_make_error_result("Failed to create class object");
    }
    
    // Install instance methods
    if (node->data.class_def.instance_methods) {
        ezom_install_methods_from_ast(class_obj, node->data.class_def.instance_methods, false);
    }
    
    // Install class methods
    if (node->data.class_def.class_methods) {
        ezom_class_t* class_struct = (ezom_class_t*)EZOM_OBJECT_PTR(class_obj);
        uint24_t metaclass = class_struct->header.class_ptr; // Class's class
        ezom_install_methods_from_ast(metaclass, node->data.class_def.class_methods, true);
    }
    
    // Register class as global
    ezom_set_global(node->data.class_def.name, class_obj);
    
    return ezom_make_result(class_obj);
}

// Variable and context management
uint24_t ezom_lookup_variable(const char* name, uint24_t context) {
    return ezom_context_lookup_variable(context, name);
}

bool ezom_set_variable(const char* name, uint24_t value, uint24_t context) {
    // Simplified - would need proper variable name tracking
    return false;
}

uint24_t ezom_lookup_global(const char* name) {
    for (uint16_t i = 0; i < g_global_count; i++) {
        if (strcmp(g_globals[i].name, name) == 0) {
            return g_globals[i].value;
        }
    }
    return g_nil;
}

bool ezom_set_global(const char* name, uint24_t value) {
    printf("     Setting global: %s\n", name);
    printf("     Current global count: %d\n", g_global_count);
    
    // Check if already exists
    for (uint16_t i = 0; i < g_global_count; i++) {
        printf("     Checking existing global %d\n", i);
        if (!g_globals[i].name) {
            printf("     ERROR: global %d has NULL name!\n", i);
            continue;
        }
        if (strcmp(g_globals[i].name, name) == 0) {
            printf("     Updated existing\n");
            g_globals[i].value = value;
            return true;
        }
    }
    
    printf("     After loop check\n");
    
    // Add new global
    if (g_global_count < MAX_GLOBALS) {
        printf("     Adding new global\n");
        g_globals[g_global_count].name = strdup(name);
        if (!g_globals[g_global_count].name) {
            printf("     ERROR: strdup failed!\n");
            return false;
        }
        g_globals[g_global_count].value = value;
        g_global_count++;
        printf("     Success\n");
        return true;
    }
    
    printf("     ERROR: Too many globals!\n");
    return false;
}

// Message dispatch support
ezom_eval_result_t ezom_eval_send_message(uint24_t receiver, const char* selector, 
                                         ezom_ast_node_t* arguments, uint24_t context) {
    // Evaluate arguments first
    uint24_t arg_values[16];
    uint8_t arg_count = 0;
    
    if (arguments) {
        ezom_eval_result_t arg_result = ezom_evaluate_arguments(arguments, arg_values, 16, context);
        if (arg_result.is_error) {
            return arg_result;
        }
        arg_count = (uint8_t)arg_result.value;
    }
    
    // Create selector symbol and dispatch message using existing system
    uint24_t selector_sym = ezom_create_symbol(selector, strlen(selector));
    
    if (arg_count == 0) {
        uint24_t result = ezom_send_unary_message(receiver, selector_sym);
        return ezom_make_result(result);
    } else if (arg_count == 1) {
        uint24_t result = ezom_send_binary_message(receiver, selector_sym, arg_values[0]);
        return ezom_make_result(result);
    } else {
        // Multi-argument messages would need enhanced dispatch system
        return ezom_make_result(g_nil);
    }
}

ezom_eval_result_t ezom_eval_send_unary_message(uint24_t receiver, const char* selector, uint24_t context) {
    uint24_t selector_sym = ezom_create_symbol(selector, strlen(selector));
    uint24_t result = ezom_send_unary_message(receiver, selector_sym);
    return ezom_make_result(result);
}

ezom_eval_result_t ezom_eval_send_binary_message(uint24_t receiver, const char* selector, 
                                                uint24_t argument, uint24_t context) {
    uint24_t selector_sym = ezom_create_symbol(selector, strlen(selector));
    uint24_t result = ezom_send_binary_message(receiver, selector_sym, argument);
    return ezom_make_result(result);
}

ezom_eval_result_t ezom_eval_send_keyword_message(uint24_t receiver, const char* selector, 
                                                 uint24_t* arguments, uint8_t arg_count, uint24_t context) {
    uint24_t selector_sym = ezom_create_symbol(selector, strlen(selector));
    
    // Create message structure for keyword message
    ezom_message_t msg = {
        .selector = selector_sym,
        .receiver = receiver,
        .args = arguments,
        .arg_count = arg_count
    };
    
    uint24_t result = ezom_send_message(&msg);
    return ezom_make_result(result);
}

// Argument evaluation
ezom_eval_result_t ezom_evaluate_arguments(ezom_ast_node_t* arg_list, uint24_t* arg_values, 
                                         uint8_t max_args, uint24_t context) {
    return ezom_evaluate_arguments_internal(arg_list, arg_values, max_args, context);
}

static ezom_eval_result_t ezom_evaluate_arguments_internal(ezom_ast_node_t* arg_list, 
                                                         uint24_t* arg_values, 
                                                         uint8_t max_args, 
                                                         uint24_t context) {
    uint8_t count = 0;
    ezom_ast_node_t* current = arg_list;
    
    while (current && count < max_args) {
        ezom_eval_result_t arg_result = ezom_evaluate_expression(current, context);
        if (arg_result.is_error) {
            return arg_result;
        }
        
        arg_values[count] = arg_result.value;
        count++;
        current = current->next;
    }
    
    return ezom_make_result(count);
}

// Control flow evaluation
ezom_eval_result_t ezom_evaluate_if_true(uint24_t condition, uint24_t true_block, uint24_t context) {
    if (ezom_is_truthy(condition)) {
        if (ezom_is_block_object(true_block)) {
            return ezom_make_result(ezom_block_evaluate(true_block, NULL, 0));
        }
    }
    return ezom_make_result(g_nil);
}

ezom_eval_result_t ezom_evaluate_if_false(uint24_t condition, uint24_t false_block, uint24_t context) {
    if (!ezom_is_truthy(condition)) {
        if (ezom_is_block_object(false_block)) {
            return ezom_make_result(ezom_block_evaluate(false_block, NULL, 0));
        }
    }
    return ezom_make_result(g_nil);
}

ezom_eval_result_t ezom_evaluate_if_true_if_false(uint24_t condition, uint24_t true_block, 
                                                uint24_t false_block, uint24_t context) {
    if (ezom_is_truthy(condition)) {
        if (ezom_is_block_object(true_block)) {
            return ezom_make_result(ezom_block_evaluate(true_block, NULL, 0));
        }
    } else {
        if (ezom_is_block_object(false_block)) {
            return ezom_make_result(ezom_block_evaluate(false_block, NULL, 0));
        }
    }
    return ezom_make_result(g_nil);
}

ezom_eval_result_t ezom_evaluate_while_true(uint24_t condition_block, uint24_t body_block, uint24_t context) {
    uint24_t result = g_nil;
    
    while (true) {
        uint24_t condition = ezom_block_evaluate(condition_block, NULL, 0);
        if (!ezom_is_truthy(condition)) {
            break;
        }
        
        result = ezom_block_evaluate(body_block, NULL, 0);
    }
    
    return ezom_make_result(result);
}

// Runtime instance variable access functions
uint24_t ezom_get_instance_variable(uint24_t object_ptr, uint16_t index) {
    if (!object_ptr) {
        return g_nil;
    }
    
    // Instance variables are stored after the object header
    ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(object_ptr);
    uint24_t* instance_vars = (uint24_t*)((char*)obj + sizeof(ezom_object_t));
    
    // TODO: Add bounds checking based on class definition
    return instance_vars[index];
}

void ezom_set_instance_variable(uint24_t object_ptr, uint16_t index, uint24_t value) {
    if (!object_ptr) {
        return;
    }
    
    // Instance variables are stored after the object header
    ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(object_ptr);
    uint24_t* instance_vars = (uint24_t*)((char*)obj + sizeof(ezom_object_t));
    
    // TODO: Add bounds checking based on class definition
    instance_vars[index] = value;
}

// Instance variable access and assignment
ezom_eval_result_t ezom_evaluate_instance_variable_access(ezom_ast_node_t* var_node, uint24_t context) {
    if (!var_node || var_node->type != AST_VARIABLE_DEF) {
        return ezom_make_error_result("Invalid variable node");
    }
    
    if (!var_node->data.variable.is_instance_var) {
        return ezom_make_error_result("Not an instance variable");
    }
    
    // Get the receiver (self) from the context
    uint24_t receiver = ezom_get_context_receiver(context);
    if (!receiver) {
        return ezom_make_error_result("No receiver in context");
    }
    
    // Access instance variable by index
    uint24_t value = ezom_get_instance_variable(receiver, var_node->data.variable.index);
    return ezom_make_result(value);
}

ezom_eval_result_t ezom_evaluate_instance_variable_assignment(ezom_ast_node_t* var_node, 
                                                            uint24_t value, uint24_t context) {
    if (!var_node || var_node->type != AST_VARIABLE_DEF) {
        return ezom_make_error_result("Invalid variable node");
    }
    
    if (!var_node->data.variable.is_instance_var) {
        return ezom_make_error_result("Not an instance variable");
    }
    
    // Get the receiver (self) from the context
    uint24_t receiver = ezom_get_context_receiver(context);
    if (!receiver) {
        return ezom_make_error_result("No receiver in context");
    }
    
    // Set instance variable by index
    ezom_set_instance_variable(receiver, var_node->data.variable.index, value);
    return ezom_make_result(value);
}

// Utility functions
ezom_eval_result_t ezom_make_result(uint24_t value) {
    ezom_eval_result_t result;
    result.value = value;
    result.is_return = false;
    result.is_error = false;
    result.error_msg[0] = '\0';
    return result;
}

ezom_eval_result_t ezom_make_return_result(uint24_t value) {
    ezom_eval_result_t result;
    result.value = value;
    result.is_return = true;
    result.is_error = false;
    result.error_msg[0] = '\0';
    return result;
}

ezom_eval_result_t ezom_make_error_result(const char* message) {
    ezom_eval_result_t result;
    result.value = g_nil;
    result.is_return = false;
    result.is_error = true;
    strncpy(result.error_msg, message, sizeof(result.error_msg) - 1);
    result.error_msg[sizeof(result.error_msg) - 1] = '\0';
    return result;
}

bool ezom_is_truthy(uint24_t object) {
    // In EZOM, only false and nil are falsy
    return object != g_false && object != g_nil;
}

// Debug support
void ezom_evaluator_debug_context(uint24_t context) {
    if (!context) {
        printf("Debug: No context\n");
        return;
    }
    
    ezom_context_t* ctx = (ezom_context_t*)EZOM_OBJECT_PTR(context);
    printf("Debug context: receiver=0x%06X, method=0x%06X, locals=%d\n",
           ctx->receiver, ctx->method, ctx->local_count);
}

void ezom_evaluator_trace_message(const char* selector, uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    printf("Trace: %s -> 0x%06X", selector, receiver);
    if (arg_count > 0) {
        printf(" (");
        for (uint8_t i = 0; i < arg_count; i++) {
            printf("0x%06X", args[i]);
            if (i < arg_count - 1) printf(", ");
        }
        printf(")");
    }
    printf("\n");
}

// Phase 2: Enhanced evaluation functions for class creation and method installation

// Class creation and method installation functions

uint24_t ezom_create_class_with_inheritance(const char* name, uint24_t superclass, uint16_t instance_var_count) {
    // Allocate class object
    uint24_t class_ptr = ezom_allocate(sizeof(ezom_class_t));
    if (!class_ptr) return 0;
    
    // Initialize as class object
    ezom_init_object(class_ptr, g_class_class ? g_class_class : g_object_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* class_obj = (ezom_class_t*)EZOM_OBJECT_PTR(class_ptr);
    class_obj->superclass = superclass;
    class_obj->method_dict = ezom_create_method_dictionary(16);
    class_obj->instance_vars = 0; // TODO: Support instance variable names
    class_obj->instance_var_count = instance_var_count;
    
    // Calculate instance size including superclass
    uint16_t super_size = sizeof(ezom_object_t);
    if (superclass) {
        ezom_class_t* super = (ezom_class_t*)EZOM_OBJECT_PTR(superclass);
        super_size = super->instance_size;
    }
    
    class_obj->instance_size = super_size + (instance_var_count * sizeof(uint24_t));
    
    printf("Created class '%s' at 0x%06X (superclass: 0x%06X, size: %d)\n", 
           name, class_ptr, superclass, class_obj->instance_size);
    
    return class_ptr;
}

uint24_t ezom_create_instance(uint24_t class_ptr) {
    if (!class_ptr) return 0;
    
    ezom_class_t* class_obj = (ezom_class_t*)EZOM_OBJECT_PTR(class_ptr);
    uint24_t instance_ptr = ezom_allocate(class_obj->instance_size);
    if (!instance_ptr) return 0;
    
    ezom_init_object(instance_ptr, class_ptr, EZOM_TYPE_OBJECT);
    
    return instance_ptr;
}

void ezom_install_method_in_class(uint24_t class_ptr, const char* selector, uint24_t code, uint8_t arg_count, bool is_primitive) {
    if (!class_ptr) return;
    
    ezom_class_t* class_obj = (ezom_class_t*)EZOM_OBJECT_PTR(class_ptr);
    ezom_method_dict_t* dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(class_obj->method_dict);
    if (!dict) return;
    
    // Check if method already exists (override)
    uint24_t selector_symbol = ezom_create_symbol(selector, strlen(selector));
    
    for (uint16_t i = 0; i < dict->size; i++) {
        if (dict->methods[i].selector == selector_symbol) {
            // Override existing method
            dict->methods[i].code = code;
            dict->methods[i].arg_count = arg_count;
            dict->methods[i].flags = is_primitive ? EZOM_METHOD_PRIMITIVE : 0;
            return;
        }
    }
    
    // Add new method
    if (dict->size >= dict->capacity) {
        printf("Warning: Method dictionary full for class 0x%06X\n", class_ptr);
        return;
    }
    
    ezom_method_t* method = &dict->methods[dict->size];
    method->selector = selector_symbol;
    method->code = code;
    method->arg_count = arg_count;
    method->flags = is_primitive ? EZOM_METHOD_PRIMITIVE : 0;
    dict->size++;
    
    printf("Installed method '%s' in class 0x%06X\n", selector, class_ptr);
}

void ezom_install_methods_from_ast(uint24_t class_ptr, ezom_ast_node_t* method_list, bool is_class_method) {
    if (!class_ptr || !method_list) return;
    
    // Iterate through method list
    ezom_ast_node_t* current = method_list->data.statement_list.statements;
    while (current) {
        if (current->type == AST_METHOD_DEF) {
            // Compile method from AST
            uint24_t method_code = ezom_compile_method_from_ast(current);
            if (method_code) {
                uint8_t arg_count = ezom_ast_count_parameters(current->data.method_def.parameters);
                ezom_install_method_in_class(class_ptr, current->data.method_def.selector, 
                                           method_code, arg_count, false);
            }
        }
        current = current->next;
    }
}

uint24_t ezom_compile_method_from_ast(ezom_ast_node_t* method_ast) {
    if (!method_ast || method_ast->type != AST_METHOD_DEF) {
        printf("Error: Invalid method AST node\n");
        return 0;
    }
    
    printf("Compiling method: %s\n", method_ast->data.method_def.selector);
    
    // For Phase 4.1.1, we'll create a compiled method object that contains:
    // 1. The AST for execution
    // 2. Parameter and local variable metadata  
    // 3. Method compilation info
    
    // Allocate space for a method execution context
    uint24_t method_code_ptr = ezom_allocate(sizeof(ezom_method_code_t));
    if (!method_code_ptr) {
        printf("Error: Failed to allocate method code object\n");
        return 0;
    }
    
    ezom_init_object(method_code_ptr, g_object_class, EZOM_TYPE_OBJECT);
    
    ezom_method_code_t* method_code = (ezom_method_code_t*)EZOM_OBJECT_PTR(method_code_ptr);
    
    // Store method compilation data
    method_code->ast_node = (uint24_t)method_ast; // Store AST for now
    method_code->param_count = ezom_ast_count_parameters(method_ast->data.method_def.parameters);
    method_code->local_count = ezom_ast_count_locals(method_ast->data.method_def.locals);
    method_code->is_primitive = method_ast->data.method_def.is_primitive;
    method_code->primitive_number = method_ast->data.method_def.primitive_number;
    
    printf("  Parameters: %d, Locals: %d\n", method_code->param_count, method_code->local_count);
    
    // In a full implementation, we would compile the method body to bytecode here
    // For now, we'll evaluate the AST at runtime
    
    printf("Method compiled successfully at 0x%06X\n", method_code_ptr);
    return method_code_ptr;
}

// Message send evaluation functions
ezom_eval_result_t ezom_evaluate_unary_message(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_UNARY_MESSAGE) {
        return ezom_make_error_result("Invalid unary message node");
    }
    
    // Evaluate the receiver
    ezom_eval_result_t receiver_result = ezom_evaluate_ast(node->data.message_send.receiver, context);
    if (receiver_result.is_error) {
        return receiver_result;
    }
    
    // Send the unary message
    uint24_t selector = ezom_create_symbol(node->data.message_send.selector, 
                                         strlen(node->data.message_send.selector));
    uint24_t result = ezom_send_unary_message(receiver_result.value, selector);
    
    return ezom_make_result(result);
}

ezom_eval_result_t ezom_evaluate_binary_message(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_BINARY_MESSAGE) {
        return ezom_make_error_result("Invalid binary message node");
    }
    
    // Evaluate the receiver
    ezom_eval_result_t receiver_result = ezom_evaluate_ast(node->data.message_send.receiver, context);
    if (receiver_result.is_error) {
        return receiver_result;
    }
    
    // Evaluate the argument (binary messages have one argument)
    ezom_eval_result_t arg_result = ezom_evaluate_ast(node->data.message_send.arguments, context);
    if (arg_result.is_error) {
        return arg_result;
    }
    
    // Send the binary message
    uint24_t selector = ezom_create_symbol(node->data.message_send.selector, 
                                         strlen(node->data.message_send.selector));
    uint24_t result = ezom_send_binary_message(receiver_result.value, selector, arg_result.value);
    
    return ezom_make_result(result);
}

ezom_eval_result_t ezom_evaluate_keyword_message(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_KEYWORD_MESSAGE) {
        return ezom_make_error_result("Invalid keyword message node");
    }
    
    // Evaluate the receiver
    ezom_eval_result_t receiver_result = ezom_evaluate_ast(node->data.message_send.receiver, context);
    if (receiver_result.is_error) {
        return receiver_result;
    }
    
    // For keyword messages, the selector is already built in the parser
    uint24_t selector = ezom_create_symbol(node->data.message_send.selector, 
                                         strlen(node->data.message_send.selector));
    
    // Evaluate arguments
    uint24_t args[16]; // Maximum 16 arguments
    uint8_t arg_count = 0;
    
    ezom_ast_node_t* current_arg = node->data.message_send.arguments;
    while (current_arg && arg_count < 16) {
        ezom_eval_result_t arg_result = ezom_evaluate_ast(current_arg, context);
        if (arg_result.is_error) {
            return arg_result;
        }
        args[arg_count++] = arg_result.value;
        current_arg = current_arg->next;
    }
    
    // Create message and send it
    ezom_message_t message = {
        .receiver = receiver_result.value,
        .selector = selector,
        .args = args,
        .arg_count = arg_count
    };
    
    uint24_t result = ezom_send_message(&message);
    
    return ezom_make_result(result);
}

// Variable evaluation (instance variables, locals, parameters)
ezom_eval_result_t ezom_evaluate_variable(ezom_ast_node_t* node, uint24_t context) {
    if (!node || node->type != AST_VARIABLE_DEF) {
        return ezom_make_result(g_nil);
    }
    
    if (node->data.variable.is_instance_var) {
        // Access instance variable
        uint24_t self_ptr = ezom_get_context_receiver(context);
        if (self_ptr == 0) {
            printf("Error: Cannot access instance variable outside of object context\n");
            return ezom_make_result(g_nil);
        }
        
        uint24_t value = ezom_get_instance_variable(self_ptr, node->data.variable.index);
        return ezom_make_result(value);
    } else if (node->data.variable.is_local) {
        // Access local variable
        uint24_t value = ezom_get_local_variable(context, node->data.variable.index);
        return ezom_make_result(value);
    } else {
        // Access parameter
        uint24_t value = ezom_get_parameter(context, node->data.variable.index);
        return ezom_make_result(value);
    }
}

// Execute a compiled method with given receiver and arguments
ezom_eval_result_t ezom_execute_compiled_method(uint24_t method_code_ptr, uint24_t receiver, 
                                               uint24_t* args, uint8_t arg_count) {
    if (!method_code_ptr) {
        return ezom_make_error_result("Invalid method code pointer");
    }
    
    ezom_method_code_t* method_code = (ezom_method_code_t*)EZOM_OBJECT_PTR(method_code_ptr);
    
    // Validate argument count
    if (arg_count != method_code->param_count) {
        return ezom_make_error_result("Wrong number of arguments");
    }
    
    // Handle primitive methods
    if (method_code->is_primitive) {
        // Execute primitive method
        return ezom_execute_primitive_method(method_code->primitive_number, receiver, args, arg_count);
    }
    
    // Create execution context for the method
    uint24_t method_context = ezom_create_enhanced_method_context(receiver, method_code, args, arg_count);
    if (!method_context) {
        return ezom_make_error_result("Failed to create method context");
    }
    
    // Execute the method AST
    ezom_ast_node_t* method_ast = (ezom_ast_node_t*)method_code->ast_node;
    if (!method_ast || method_ast->type != AST_METHOD_DEF) {
        return ezom_make_error_result("Invalid method AST");
    }
    
    // Evaluate the method body
    ezom_eval_result_t result = ezom_evaluate_method_body(method_ast->data.method_def.body, method_context);
    
    // Clean up context (in full implementation)
    // ezom_destroy_context(method_context);
    
    return result;
}

// Create a method execution context with proper parameter and local variable binding
uint24_t ezom_create_enhanced_method_context(uint24_t receiver, ezom_method_code_t* method_code, 
                                           uint24_t* args, uint8_t arg_count) {
    // Create context with space for parameters and locals
    uint8_t total_vars = method_code->param_count + method_code->local_count;
    uint24_t context = ezom_create_extended_context(g_current_context, receiver, 0, total_vars);
    
    if (!context) {
        return 0;
    }
    
    // Bind parameters to the context
    for (uint8_t i = 0; i < method_code->param_count && i < arg_count; i++) {
        ezom_context_set_local(context, i, args[i]);
    }
    
    // Initialize local variables to nil
    for (uint8_t i = method_code->param_count; i < total_vars; i++) {
        ezom_context_set_local(context, i, g_nil);
    }
    
    return context;
}

// Evaluate a method body with proper context
ezom_eval_result_t ezom_evaluate_method_body(ezom_ast_node_t* body, uint24_t context) {
    if (!body) {
        return ezom_make_result(g_nil);
    }
    
    // Set current context for evaluation
    uint24_t old_context = g_current_context;
    g_current_context = context;
    
    // Evaluate the body
    ezom_eval_result_t result = ezom_evaluate_ast(body, context);
    
    // Restore previous context
    g_current_context = old_context;
    
    return result;
}

// Execute a primitive method
ezom_eval_result_t ezom_execute_primitive_method(uint8_t primitive_number, uint24_t receiver, 
                                                uint24_t* args, uint8_t arg_count) {
    // Check if primitive number is valid
    if (primitive_number >= MAX_PRIMITIVES || !g_primitives[primitive_number]) {
        return ezom_make_error_result("Invalid primitive number");
    }
    
    // Call the primitive function
    uint24_t result = g_primitives[primitive_number](receiver, args, arg_count);
    return ezom_make_result(result);
}

// Resolve variable type and assign appropriately
bool ezom_assign_resolved_variable(const char* var_name, uint24_t value, uint24_t context) {
    // This function resolves a variable name to its type and assigns the value
    // It handles instance variables, local variables, and parameters
    
    // First, check if it's a local variable or parameter in the current context
    if (ezom_context_has_local(context, var_name)) {
        uint16_t local_index = ezom_context_get_local_index(context, var_name);
        ezom_context_set_local(context, local_index, value);
        return true;
    }
    
    // Check if it's an instance variable
    uint24_t receiver = ezom_get_context_receiver(context);
    if (receiver) {
        uint16_t instance_var_index = ezom_get_instance_variable_index(receiver, var_name);
        if (instance_var_index != UINT16_MAX) {
            ezom_set_instance_variable(receiver, instance_var_index, value);
            return true;
        }
    }
    
    // Variable not found in current context
    return false;
}

// Check if a context has a local variable with given name
bool ezom_context_has_local(uint24_t context_ptr, const char* name) {
    // This is a simplified version - in a full implementation,
    // we would store variable names in the context
    // For now, we'll use the context lookup function
    uint24_t result = ezom_context_lookup_variable(context_ptr, name);
    return result != g_nil;
}

// Get the index of a local variable by name
uint16_t ezom_context_get_local_index(uint24_t context_ptr, const char* name) {
    // This is a placeholder - in a full implementation,
    // we would maintain a mapping of variable names to indices
    // For now, we'll return a default index
    return 0; // TODO: Implement proper variable name mapping
}

// Get the index of an instance variable by name
uint16_t ezom_get_instance_variable_index(uint24_t object_ptr, const char* name) {
    if (!object_ptr) {
        return UINT16_MAX;
    }
    
    // Get the object's class
    ezom_object_t* obj = (ezom_object_t*)EZOM_OBJECT_PTR(object_ptr);
    uint24_t class_ptr = obj->class_ptr;
    
    if (!class_ptr) {
        return UINT16_MAX;
    }
    
    // Look up instance variable in class definition
    return ezom_find_instance_variable_index_in_class(class_ptr, name);
}

// Find instance variable index in class definition
uint16_t ezom_find_instance_variable_index_in_class(uint24_t class_ptr, const char* name) {
    if (!class_ptr || !name) {
        return UINT16_MAX;
    }
    
    // This is a placeholder - in a full implementation,
    // we would store instance variable names in the class
    // For now, we'll return UINT16_MAX (not found)
    return UINT16_MAX; // TODO: Implement proper instance variable lookup
}