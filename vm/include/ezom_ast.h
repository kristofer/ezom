// ============================================================================
// File: include/ezom_ast.h
// Abstract Syntax Tree definitions
// ============================================================================

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ezom_platform.h"

typedef enum {
    AST_CLASS_DEF,
    AST_METHOD_DEF,
    AST_VARIABLE_DEF,
    AST_MESSAGE_SEND,
    AST_BLOCK,
    AST_RETURN,
    AST_ASSIGNMENT,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_ARRAY_LITERAL,
    AST_UNARY_MESSAGE,
    AST_BINARY_MESSAGE,
    AST_KEYWORD_MESSAGE,
    AST_SUPER_SEND,
    AST_VARIABLE_LIST,
    AST_STATEMENT_LIST,
    AST_PARAMETER_LIST
} ezom_ast_type_t;

typedef struct ezom_ast_node ezom_ast_node_t;

struct ezom_ast_node {
    ezom_ast_type_t type;
    uint16_t        line;
    uint16_t        column;
    ezom_ast_node_t* next;    // For linked lists of nodes
    
    union {
        // Class definition
        struct {
            char* name;
            ezom_ast_node_t* superclass;
            ezom_ast_node_t* instance_vars;
            ezom_ast_node_t* instance_methods;
            ezom_ast_node_t* class_methods;
        } class_def;
        
        // Method definition
        struct {
            char* selector;
            ezom_ast_node_t* parameters;
            ezom_ast_node_t* locals;
            ezom_ast_node_t* body;
            bool is_class_method;
            bool is_primitive;
            uint8_t primitive_number;
        } method_def;
        
        // Variable definition/reference
        struct {
            char* name;
            bool is_instance_var;
            bool is_local;
            uint16_t index;
        } variable;
        
        // Message send (unified for all message types)
        struct {
            ezom_ast_node_t* receiver;
            char* selector;
            ezom_ast_node_t* arguments;
            bool is_super;
            uint8_t arg_count;
        } message_send;
        
        // Block (closure)
        struct {
            ezom_ast_node_t* parameters;
            ezom_ast_node_t* locals;
            ezom_ast_node_t* body;
            uint8_t param_count;
            uint8_t local_count;
        } block;
        
        // Return statement
        struct {
            ezom_ast_node_t* expression;
            bool is_block_return;
        } return_stmt;
        
        // Assignment
        struct {
            ezom_ast_node_t* variable;
            ezom_ast_node_t* value;
        } assignment;
        
        // Literal values
        struct {
            enum {
                LITERAL_INTEGER,
                LITERAL_STRING,
                LITERAL_SYMBOL,
                LITERAL_ARRAY,
                LITERAL_NIL,
                LITERAL_TRUE,
                LITERAL_FALSE
            } type;
            union {
                int16_t integer_value;
                char* string_value;
                char* symbol_value;
                ezom_ast_node_t* array_elements;
            } value;
        } literal;
        
        // Identifier
        struct {
            char* name;
        } identifier;
        
        // Statement/expression list
        struct {
            ezom_ast_node_t* statements;
            uint16_t count;
        } statement_list;
        
        // Variable list (for locals, parameters, instance vars)
        struct {
            char** names;
            uint16_t count;
        } variable_list;
        
    } data;
};

// AST node creation functions
ezom_ast_node_t* ezom_ast_create(ezom_ast_type_t type);
ezom_ast_node_t* ezom_ast_create_class_def(char* name, ezom_ast_node_t* superclass);
ezom_ast_node_t* ezom_ast_create_method_def(char* selector, bool is_class_method);
ezom_ast_node_t* ezom_ast_create_message_send(ezom_ast_node_t* receiver, char* selector);
ezom_ast_node_t* ezom_ast_create_block(void);
ezom_ast_node_t* ezom_ast_create_return(ezom_ast_node_t* expression);
ezom_ast_node_t* ezom_ast_create_assignment(ezom_ast_node_t* variable, ezom_ast_node_t* value);
ezom_ast_node_t* ezom_ast_create_literal_integer(int16_t value);
ezom_ast_node_t* ezom_ast_create_literal_string(const char* value);
ezom_ast_node_t* ezom_ast_create_literal_symbol(const char* value);
ezom_ast_node_t* ezom_ast_create_identifier(const char* name);
ezom_ast_node_t* ezom_ast_create_variable_list(void);
ezom_ast_node_t* ezom_ast_create_statement_list(void);

// AST manipulation functions
void ezom_ast_add_statement(ezom_ast_node_t* list, ezom_ast_node_t* statement);
void ezom_ast_add_variable(ezom_ast_node_t* list, char* name);
void ezom_ast_add_argument(ezom_ast_node_t* message, ezom_ast_node_t* argument);
void ezom_ast_set_line_info(ezom_ast_node_t* node, uint16_t line, uint16_t column);

// AST utility functions
void ezom_ast_free(ezom_ast_node_t* node);
void ezom_ast_print(ezom_ast_node_t* node, int indent);
uint16_t ezom_ast_count_parameters(ezom_ast_node_t* params);
uint16_t ezom_ast_count_locals(ezom_ast_node_t* locals);

// AST validation
bool ezom_ast_validate(ezom_ast_node_t* node, char* error_buffer, size_t buffer_size);

// Simple AST creation functions for testing
ezom_ast_node_t* ezom_ast_create_integer_literal(int value);
ezom_ast_node_t* ezom_ast_create_string_literal(char* value);
ezom_ast_node_t* ezom_ast_create_binary_message(ezom_ast_node_t* receiver, char* selector, ezom_ast_node_t* argument);

// Simple AST evaluation functions for testing
uint24_t ezom_ast_simple_evaluate(ezom_ast_node_t* node);
uint24_t ezom_ast_test_simple_expression(void);

// Complex AST test functions
uint24_t ezom_ast_test_nested_arithmetic(void);      // (10 + 5) * 2
uint24_t ezom_ast_test_chain_operations(void);       // 20 - 5 + 3  
uint24_t ezom_ast_test_mixed_operations(void);       // 100 / 4 + 15 * 2
uint24_t ezom_ast_test_comparison_chain(void);       // (5 < 10) = true
uint24_t ezom_ast_test_deep_nesting(void);           // ((8 + 2) * 3) - (5 + 1)