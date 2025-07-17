// ============================================================================
// File: vm/src/file_loader.c
// File loading and SOM program execution implementation
// ============================================================================

#include "../include/ezom_file_loader.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_object.h"
#include "../include/ezom_lexer.h"
#include "../include/ezom_parser.h"
#include "../include/ezom_evaluator.h"
#include "../include/ezom_context.h"
#include "../include/ezom_ast_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// File I/O Utilities
// ============================================================================

char* ezom_read_file_contents(const char* filename, size_t* length) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    // Read file contents
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    
    if (length) {
        *length = bytes_read;
    }
    
    return buffer;
}

int ezom_file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

const char* ezom_get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    return dot ? dot + 1 : "";
}

// ============================================================================
// File Loading Implementation
// ============================================================================

ezom_file_result_t ezom_load_file(const char* filename, ezom_file_context_t* context) {
    if (!filename || !context) {
        return EZOM_FILE_READ_ERROR;
    }
    
    // Initialize context
    memset(context, 0, sizeof(ezom_file_context_t));
    
    // Store filename
    context->filename = malloc(strlen(filename) + 1);
    if (!context->filename) {
        return EZOM_FILE_MEMORY_ERROR;
    }
    strcpy(context->filename, filename);
    
    // Check if file exists
    if (!ezom_file_exists(filename)) {
        return EZOM_FILE_NOT_FOUND;
    }
    
    // Read file contents
    context->source_code = ezom_read_file_contents(filename, &context->source_length);
    if (!context->source_code) {
        return EZOM_FILE_READ_ERROR;
    }
    
    printf("Loaded file: %s (%zu bytes)\n", filename, context->source_length);
    return EZOM_FILE_OK;
}

ezom_file_result_t ezom_parse_file(ezom_file_context_t* context) {
    if (!context || !context->source_code) {
        return EZOM_FILE_PARSE_ERROR;
    }
    
    // Initialize lexer
    ezom_lexer_init(&context->lexer, context->source_code);
    
    // Initialize parser
    ezom_parser_init(&context->parser, &context->lexer);
    
    // Parse the program
    // For now, we'll parse it as a single expression or class definition
    ezom_ast_node_t* ast = ezom_parse_program(&context->parser);
    if (!ast) {
        // Try parsing as a class definition
        ast = ezom_parse_class_definition(&context->parser);
    }
    
    if (!ast) {
        return EZOM_FILE_PARSE_ERROR;
    }
    
    context->program_ast = ast;
    printf("Parsed program successfully\n");
    return EZOM_FILE_OK;
}

ezom_file_result_t ezom_evaluate_file(ezom_file_context_t* context) {
    if (!context || !context->program_ast) {
        return EZOM_FILE_EVAL_ERROR;
    }
    
    // Create evaluation context
    uint24_t eval_context = ezom_create_extended_context(0, 0, 0, 10);
    if (!eval_context) {
        return EZOM_FILE_MEMORY_ERROR;
    }
    
    // Evaluate the program AST
    ezom_eval_result_t result = ezom_evaluate_ast(context->program_ast, eval_context);
    if (result.is_error) {
        printf("Evaluation error: %s\n", result.error_msg ? result.error_msg : "Unknown error");
        return EZOM_FILE_EVAL_ERROR;
    }
    
    context->result_value = result.value;
    printf("Program executed successfully\n");
    return EZOM_FILE_OK;
}

void ezom_free_file_context(ezom_file_context_t* context) {
    if (!context) return;
    
    if (context->filename) {
        free(context->filename);
    }
    if (context->source_code) {
        free(context->source_code);
    }
    
    memset(context, 0, sizeof(ezom_file_context_t));
}

// ============================================================================
// High-Level Program Execution
// ============================================================================

ezom_file_result_t ezom_execute_som_file(const char* filename, uint24_t* result) {
    ezom_file_context_t context;
    ezom_file_result_t status;
    
    // Load file
    status = ezom_load_file(filename, &context);
    if (status != EZOM_FILE_OK) {
        return status;
    }
    
    // Parse file
    status = ezom_parse_file(&context);
    if (status != EZOM_FILE_OK) {
        ezom_free_file_context(&context);
        return status;
    }
    
    // Evaluate file
    status = ezom_evaluate_file(&context);
    if (status != EZOM_FILE_OK) {
        ezom_free_file_context(&context);
        return status;
    }
    
    // Return result
    if (result) {
        *result = context.result_value;
    }
    
    ezom_free_file_context(&context);
    return EZOM_FILE_OK;
}

ezom_file_result_t ezom_execute_som_code(const char* code, uint24_t* result) {
    if (!code) {
        return EZOM_FILE_READ_ERROR;
    }
    
    // Create a temporary file context
    ezom_file_context_t context;
    memset(&context, 0, sizeof(context));
    
    // Set up source code
    context.source_code = malloc(strlen(code) + 1);
    if (!context.source_code) {
        return EZOM_FILE_MEMORY_ERROR;
    }
    strcpy(context.source_code, code);
    context.source_length = strlen(code);
    
    // Parse the code
    ezom_file_result_t status = ezom_parse_file(&context);
    if (status != EZOM_FILE_OK) {
        ezom_free_file_context(&context);
        return status;
    }
    
    // Evaluate the code
    status = ezom_evaluate_file(&context);
    if (status != EZOM_FILE_OK) {
        ezom_free_file_context(&context);
        return status;
    }
    
    // Return result
    if (result) {
        *result = context.result_value;
    }
    
    ezom_free_file_context(&context);
    return EZOM_FILE_OK;
}

// ============================================================================
// Phase 4.1.2: Enhanced .som File Loading and Parsing
// ============================================================================

// Load a .som class file and register it globally
ezom_file_result_t ezom_load_som_class_file(const char* filename, uint24_t* class_result) {
    printf("Loading .som class file: %s\n", filename);
    
    ezom_file_context_t context;
    ezom_file_result_t status;
    
    // Load file
    status = ezom_load_file(filename, &context);
    if (status != EZOM_FILE_OK) {
        printf("Failed to load file: %s\n", filename);
        return status;
    }
    
    // Parse file as class definition
    status = ezom_parse_som_class_file(&context);
    if (status != EZOM_FILE_OK) {
        printf("Failed to parse .som file: %s\n", filename);
        ezom_free_file_context(&context);
        return status;
    }
    
    // Evaluate and create class
    status = ezom_evaluate_som_class(&context);
    if (status != EZOM_FILE_OK) {
        printf("Failed to evaluate .som class: %s\n", filename);
        ezom_free_file_context(&context);
        return status;
    }
    
    // Return class result
    if (class_result) {
        *class_result = context.result_value;
    }
    
    printf("Successfully loaded .som class from: %s\n", filename);
    ezom_free_file_context(&context);
    return EZOM_FILE_OK;
}

// Parse a .som file specifically as a class definition
ezom_file_result_t ezom_parse_som_class_file(ezom_file_context_t* context) {
    if (!context || !context->source_code) {
        return EZOM_FILE_PARSE_ERROR;
    }
    
    printf("Parsing .som class file...\n");
    
    // Initialize lexer
    ezom_lexer_init(&context->lexer, context->source_code);
    
    // Initialize parser
    ezom_parser_init(&context->parser, &context->lexer);
    
    // Parse as class definition - this is what .som files should contain
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&context->parser);
    if (!class_ast) {
        printf("Failed to parse as class definition\n");
        printf("  Line: %d, Column: %d\n", context->parser.lexer->line, context->parser.lexer->column);
        return EZOM_FILE_PARSE_ERROR;
    }
    
    context->program_ast = class_ast;
    printf("Successfully parsed .som class definition\n");
    return EZOM_FILE_OK;
}

// Evaluate a .som class and register it globally
ezom_file_result_t ezom_evaluate_som_class(ezom_file_context_t* context) {
    if (!context || !context->program_ast) {
        return EZOM_FILE_EVAL_ERROR;
    }
    
    printf("Evaluating .som class...\n");
    
    // Create evaluation context
    uint24_t eval_context = ezom_create_context(0, 0);
    
    // Evaluate the class definition
    ezom_eval_result_t result = ezom_evaluate_class_definition(context->program_ast, eval_context);
    
    if (result.is_error) {
        printf("Evaluation error: %s\n", result.error_msg);
        return EZOM_FILE_EVAL_ERROR;
    }
    
    // Store result
    context->result_value = result.value;
    
    printf("Successfully evaluated .som class: 0x%06X\n", result.value);
    return EZOM_FILE_OK;
}

// Load multiple .som files from a directory
ezom_file_result_t ezom_load_som_directory(const char* directory) {
    printf("Loading .som files from directory: %s\n", directory);
    
    // This is a placeholder for directory loading
    // In a full implementation, we would:
    // 1. Scan directory for .som files
    // 2. Load each file in dependency order
    // 3. Handle class inheritance properly
    
    printf("Directory loading not yet implemented\n");
    return EZOM_FILE_OK;
}

// ============================================================================
// Command Line Interface
// ============================================================================

ezom_args_t ezom_parse_arguments(int argc, char* argv[]) {
    ezom_args_t args;
    memset(&args, 0, sizeof(args));
    
    args.argc = argc;
    args.argv = argv;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--code") == 0) {
            if (i + 1 < argc) {
                args.eval_code = argv[++i];
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            args.interactive_mode = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            args.verbose_mode = 1;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            args.debug_mode = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            ezom_print_usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "--version") == 0) {
            ezom_print_version();
            exit(0);
        } else if (argv[i][0] != '-') {
            // This is a filename
            args.filename = argv[i];
        }
    }
    
    return args;
}

void ezom_print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] [FILE]\n", program_name);
    printf("\nOptions:\n");
    printf("  -c, --code CODE    Execute CODE directly\n");
    printf("  -i, --interactive  Start interactive REPL\n");
    printf("  -v, --verbose      Enable verbose output\n");
    printf("  -d, --debug        Enable debug output\n");
    printf("  -h, --help         Show this help message\n");
    printf("  --version          Show version information\n");
    printf("\nExamples:\n");
    printf("  %s hello.som                # Execute hello.som file\n", program_name);
    printf("  %s -c \"5 + 3\"              # Execute code directly\n", program_name);
    printf("  %s -i                       # Start interactive REPL\n", program_name);
}

void ezom_print_version(void) {
    printf("EZOM (Easy Simple Object Machine) v1.0\n");
    printf("Phase 4.4.1 - File Loading and SOM Program Execution\n");
    printf("Built for ez80 Agon Light 2 and native development\n");
}

// ============================================================================
// Interactive REPL
// ============================================================================

void ezom_start_repl(void) {
    printf("EZOM Interactive REPL\n");
    printf("Type 'exit' or 'quit' to exit, 'help' for help\n");
    printf("=============================================\n");
    
    ezom_repl_loop();
}

void ezom_repl_loop(void) {
    char input[1024];
    
    while (1) {
        printf("EZOM> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // Remove trailing newline
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        // Handle REPL commands
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        } else if (strcmp(input, "help") == 0) {
            printf("Available commands:\n");
            printf("  help      - Show this help\n");
            printf("  exit/quit - Exit the REPL\n");
            printf("  gc        - Run garbage collection\n");
            printf("  stats     - Show memory statistics\n");
            printf("  classes   - List available classes\n");
            printf("Or enter any SOM expression to evaluate it.\n");
            continue;
        } else if (strcmp(input, "gc") == 0) {
            ezom_garbage_collect();
            printf("Garbage collection completed\n");
            continue;
        } else if (strcmp(input, "stats") == 0) {
            ezom_detailed_memory_stats();
            continue;
        } else if (strcmp(input, "classes") == 0) {
            printf("Available classes:\n");
            printf("  Object, Integer, String, Symbol, Array, Boolean, Block, Context, Class\n");
            continue;
        }
        
        // Evaluate the input
        if (strlen(input) > 0) {
            ezom_repl_evaluate(input);
        }
    }
    
    printf("Goodbye!\n");
}

void ezom_repl_evaluate(const char* input) {
    uint24_t result;
    ezom_file_result_t status = ezom_execute_som_code(input, &result);
    
    if (status == EZOM_FILE_OK) {
        if (result != 0) {
            // Print the result
            uint24_t println_selector = ezom_create_symbol("println", 7);
            ezom_send_unary_message(result, println_selector);
        } else {
            printf("nil\n");
        }
    } else {
        ezom_print_file_error(status, "<input>");
    }
}

// ============================================================================
// Error Reporting
// ============================================================================

void ezom_print_file_error(ezom_file_result_t error, const char* filename) {
    switch (error) {
        case EZOM_FILE_OK:
            return;
        case EZOM_FILE_NOT_FOUND:
            printf("Error: File not found: %s\n", filename);
            break;
        case EZOM_FILE_READ_ERROR:
            printf("Error: Cannot read file: %s\n", filename);
            break;
        case EZOM_FILE_PARSE_ERROR:
            printf("Error: Parse error in file: %s\n", filename);
            break;
        case EZOM_FILE_EVAL_ERROR:
            printf("Error: Evaluation error in file: %s\n", filename);
            break;
        case EZOM_FILE_MEMORY_ERROR:
            printf("Error: Memory allocation error while processing: %s\n", filename);
            break;
    }
}

void ezom_print_parse_error(ezom_parser_t* parser, const char* filename) {
    if (!parser) return;
    
    printf("Parse error in %s:\n", filename);
    printf("  Line: %d, Column: %d\n", parser->lexer->line, parser->lexer->column);
    printf("  Expected: valid SOM expression or class definition\n");
}
