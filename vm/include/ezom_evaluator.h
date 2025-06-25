// ============================================================================
// File: include/ezom_evaluator.h
// AST evaluation engine for EZOM virtual machine
// ============================================================================

#pragma once
#include "ezom_ast.h"
#include "ezom_context.h"
#include "ezom_object.h"
#include <stdint.h>
#include <stdbool.h>

// Evaluation result structure
typedef struct ezom_eval_result {
    uint24_t value;           // Result object
    bool     is_return;       // True if this is a return value
    bool     is_error;        // True if evaluation failed
    char     error_msg[256];  // Error message if any
} ezom_eval_result_t;

// Evaluator initialization and cleanup
void ezom_evaluator_init(void);
void ezom_evaluator_cleanup(void);

// Main evaluation functions
ezom_eval_result_t ezom_evaluate_ast(ezom_ast_node_t* node, uint24_t context);
ezom_eval_result_t ezom_evaluate_expression(ezom_ast_node_t* expr, uint24_t context);
ezom_eval_result_t ezom_evaluate_statement(ezom_ast_node_t* stmt, uint24_t context);
ezom_eval_result_t ezom_evaluate_statement_list(ezom_ast_node_t* statements, uint24_t context);

// Specific node evaluation
ezom_eval_result_t ezom_evaluate_message_send(ezom_ast_node_t* node, uint24_t context);
ezom_eval_result_t ezom_evaluate_literal(ezom_ast_node_t* node, uint24_t context);
ezom_eval_result_t ezom_evaluate_identifier(ezom_ast_node_t* node, uint24_t context);
ezom_eval_result_t ezom_evaluate_assignment(ezom_ast_node_t* node, uint24_t context);
ezom_eval_result_t ezom_evaluate_return(ezom_ast_node_t* node, uint24_t context);
ezom_eval_result_t ezom_evaluate_block_literal(ezom_ast_node_t* node, uint24_t context);

// Class and method evaluation
ezom_eval_result_t ezom_evaluate_class_definition(ezom_ast_node_t* node, uint24_t context);
ezom_eval_result_t ezom_evaluate_method_definition(ezom_ast_node_t* node, uint24_t class_ptr, uint24_t context);

// Variable and context management
uint24_t ezom_lookup_variable(const char* name, uint24_t context);
bool ezom_set_variable(const char* name, uint24_t value, uint24_t context);
uint24_t ezom_lookup_global(const char* name);
bool ezom_set_global(const char* name, uint24_t value);

// Message dispatch support
ezom_eval_result_t ezom_eval_send_message(uint24_t receiver, const char* selector, 
                                         ezom_ast_node_t* arguments, uint24_t context);
ezom_eval_result_t ezom_eval_send_unary_message(uint24_t receiver, const char* selector, uint24_t context);
ezom_eval_result_t ezom_eval_send_binary_message(uint24_t receiver, const char* selector, 
                                                uint24_t argument, uint24_t context);
ezom_eval_result_t ezom_eval_send_keyword_message(uint24_t receiver, const char* selector, 
                                                 uint24_t* arguments, uint8_t arg_count, uint24_t context);

// Argument evaluation
uint8_t ezom_evaluate_arguments(ezom_ast_node_t* arg_list, uint24_t* arg_values, 
                               uint8_t max_args, uint24_t context);

// Control flow evaluation
ezom_eval_result_t ezom_evaluate_if_true(uint24_t condition, uint24_t true_block, uint24_t context);
ezom_eval_result_t ezom_evaluate_if_false(uint24_t condition, uint24_t false_block, uint24_t context);
ezom_eval_result_t ezom_evaluate_if_true_if_false(uint24_t condition, uint24_t true_block, 
                                                uint24_t false_block, uint24_t context);
ezom_eval_result_t ezom_evaluate_while_true(uint24_t condition_block, uint24_t body_block, uint24_t context);

// Utility functions
ezom_eval_result_t ezom_make_result(uint24_t value);
ezom_eval_result_t ezom_make_return_result(uint24_t value);
ezom_eval_result_t ezom_make_error_result(const char* message);
bool ezom_is_truthy(uint24_t object);

// Debug support
void ezom_evaluator_debug_context(uint24_t context);
void ezom_evaluator_trace_message(const char* selector, uint24_t receiver, uint24_t* args, uint8_t arg_count);