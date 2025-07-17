// ============================================================================
// File: vm/include/ezom_file_loader.h
// File loading and SOM program execution interface
// ============================================================================

#pragma once
#include "ezom_object.h"
#include "ezom_parser.h"
#include "ezom_evaluator.h"
#include <stdio.h>

// Error codes for file operations
typedef enum {
    EZOM_FILE_OK = 0,
    EZOM_FILE_NOT_FOUND,
    EZOM_FILE_READ_ERROR,
    EZOM_FILE_PARSE_ERROR,
    EZOM_FILE_EVAL_ERROR,
    EZOM_FILE_MEMORY_ERROR
} ezom_file_result_t;

// File loading context
typedef struct {
    char* filename;
    char* source_code;
    size_t source_length;
    ezom_parser_t parser;
    ezom_lexer_t lexer;
    ezom_ast_node_t* program_ast;
    uint24_t result_value;
    ezom_file_result_t status;
} ezom_file_context_t;

// Command line argument structure
typedef struct {
    int argc;
    char** argv;
    char* filename;
    char* eval_code;
    int interactive_mode;
    int verbose_mode;
    int debug_mode;
} ezom_args_t;

// Core file loading functions
ezom_file_result_t ezom_load_file(const char* filename, ezom_file_context_t* context);
ezom_file_result_t ezom_parse_file(ezom_file_context_t* context);
ezom_file_result_t ezom_evaluate_file(ezom_file_context_t* context);
void ezom_free_file_context(ezom_file_context_t* context);

// Program execution functions
ezom_file_result_t ezom_execute_som_file(const char* filename, uint24_t* result);
ezom_file_result_t ezom_execute_som_code(const char* code, uint24_t* result);

// Command line interface
ezom_args_t ezom_parse_arguments(int argc, char* argv[]);
void ezom_print_usage(const char* program_name);
void ezom_print_version(void);

// Interactive REPL
void ezom_start_repl(void);
void ezom_repl_loop(void);
void ezom_repl_evaluate(const char* input);

// Error reporting
void ezom_print_file_error(ezom_file_result_t error, const char* filename);
void ezom_print_parse_error(ezom_parser_t* parser, const char* filename);

// File I/O utilities
char* ezom_read_file_contents(const char* filename, size_t* length);
int ezom_file_exists(const char* filename);
const char* ezom_get_file_extension(const char* filename);
