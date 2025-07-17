// ============================================================================
// File: include/ezom_parser.h
// Parser for EZOM language syntax
// ============================================================================

#pragma once
#include "ezom_lexer.h"
#include "ezom_ast.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct ezom_parser {
    ezom_lexer_t* lexer;
    bool has_error;
    char error_message[256];
    uint16_t error_count;
} ezom_parser_t;

// Parser initialization
void ezom_parser_init(ezom_parser_t* parser, ezom_lexer_t* lexer);

// Top-level parsing
ezom_ast_node_t* ezom_parse_program(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_class_definition(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_method_definition(ezom_parser_t* parser, bool is_class_method);

// Expression parsing
ezom_ast_node_t* ezom_parse_expression(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_primary(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_unary_message(ezom_parser_t* parser, ezom_ast_node_t* receiver);
ezom_ast_node_t* ezom_parse_binary_message(ezom_parser_t* parser, ezom_ast_node_t* receiver);
ezom_ast_node_t* ezom_parse_keyword_message(ezom_parser_t* parser, ezom_ast_node_t* receiver);

// Statement parsing
ezom_ast_node_t* ezom_parse_statement(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_statement_list(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_assignment(ezom_parser_t* parser, ezom_ast_node_t* variable);
ezom_ast_node_t* ezom_parse_return_statement(ezom_parser_t* parser);

// Block parsing
ezom_ast_node_t* ezom_parse_block(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_block_parameters(ezom_parser_t* parser);

// Literal parsing
ezom_ast_node_t* ezom_parse_literal(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_array_literal(ezom_parser_t* parser);

// Variable and parameter parsing
ezom_ast_node_t* ezom_parse_variable_list(ezom_parser_t* parser);
ezom_ast_node_t* ezom_parse_parameter_list(ezom_parser_t* parser);

// Method selector parsing
char* ezom_parse_unary_selector(ezom_parser_t* parser);
char* ezom_parse_binary_selector(ezom_parser_t* parser);
char* ezom_parse_keyword_selector(ezom_parser_t* parser);
char* ezom_parse_keyword_method_signature(ezom_parser_t* parser, ezom_ast_node_t** parameters);
char* ezom_parse_binary_method_signature(ezom_parser_t* parser, ezom_ast_node_t** parameters);

// Utility functions
bool ezom_parser_match(ezom_parser_t* parser, ezom_token_type_t type);
bool ezom_parser_check(ezom_parser_t* parser, ezom_token_type_t type);
void ezom_parser_consume(ezom_parser_t* parser, ezom_token_type_t type, const char* error_msg);
void ezom_parser_advance(ezom_parser_t* parser);
void ezom_parser_error(ezom_parser_t* parser, const char* message);
void ezom_parser_synchronize(ezom_parser_t* parser);

// Helper functions
bool ezom_is_binary_operator(ezom_token_type_t type);
bool ezom_is_keyword_start(ezom_parser_t* parser);
int ezom_get_precedence(ezom_token_type_t type);
char* ezom_copy_current_token_text(ezom_parser_t* parser);

// Instance variable resolution context
typedef struct {
    ezom_ast_node_t* class_def;
    ezom_ast_node_t* method_def;
    ezom_ast_node_t* current_locals;
    ezom_ast_node_t* current_parameters;
} ezom_variable_context_t;

// Resolve variable type based on context
typedef enum {
    VAR_INSTANCE,
    VAR_LOCAL,
    VAR_PARAMETER,
    VAR_UNKNOWN
} ezom_variable_type_t;

ezom_variable_type_t ezom_resolve_variable_type(const char* name, ezom_variable_context_t* context);
int ezom_find_instance_variable_index(const char* name, ezom_ast_node_t* class_def);
int ezom_find_local_variable_index(const char* name, ezom_ast_node_t* locals);
int ezom_find_parameter_index(const char* name, ezom_ast_node_t* parameters);
ezom_ast_node_t* ezom_create_variable_with_context(const char* name, ezom_variable_context_t* context);