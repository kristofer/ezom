#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/ezom_object.h"
#include "../include/ezom_context.h"
#include "../include/ezom_parser.h"
#include "../include/ezom_evaluator.h"
#include "../include/ezom_lexer.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_dispatch.h"

void test_complete_class_system() {
    printf("Testing complete class system with instance creation...\n");
    
    // Test a complete class with instance variables and methods
    const char* class_source = 
        "Point = Object (\n"
        "    | x y |\n"
        "    \n"
        "    initializeX: newX y: newY = (\n"
        "        x := newX.\n"
        "        y := newY.\n"
        "        ^self\n"
        "    )\n"
        "    \n"
        "    getX = (\n"
        "        ^x\n"
        "    )\n"
        "    \n"
        "    getY = (\n"
        "        ^y\n"
        "    )\n"
        "    \n"
        "    ----\n"
        "    \n"
        "    newX: newX y: newY = (\n"
        "        ^self new initializeX: newX y: newY\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)class_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    assert(class_ast != NULL);
    assert(class_ast->type == AST_CLASS_DEF);
    
    // Check class components
    assert(strcmp(class_ast->data.class_def.name, "Point") == 0);
    assert(class_ast->data.class_def.instance_vars != NULL);
    assert(class_ast->data.class_def.instance_methods != NULL);
    assert(class_ast->data.class_def.class_methods != NULL);
    
    // Create a context and evaluate the class definition
    uint24_t context = ezom_create_extended_context(0, 0, 0, 5);
    assert(context != 0);
    
    ezom_eval_result_t result = ezom_evaluate_class_definition(class_ast, context);
    assert(!result.is_error);
    assert(result.value != 0);
    
    // Verify the Point class was created and is available globally
    uint24_t point_class = ezom_lookup_global("Point");
    assert(point_class != 0);
    assert(point_class == result.value);
    
    printf("✓ Complete class system test passed\n");
}

void test_inheritance_parsing() {
    printf("Testing class inheritance parsing...\n");
    
    // Test class with explicit superclass
    const char* inheritance_source = 
        "ColoredPoint = Point (\n"
        "    | color |\n"
        "    \n"
        "    setColor: newColor = (\n"
        "        color := newColor.\n"
        "        ^self\n"
        "    )\n"
        "    \n"
        "    getColor = (\n"
        "        ^color\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)inheritance_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    assert(class_ast != NULL);
    assert(class_ast->type == AST_CLASS_DEF);
    
    // Check inheritance
    assert(strcmp(class_ast->data.class_def.name, "ColoredPoint") == 0);
    assert(class_ast->data.class_def.superclass != NULL);
    assert(strcmp(class_ast->data.class_def.superclass->data.identifier.name, "Point") == 0);
    
    printf("✓ Inheritance parsing test passed\n");
}

void test_method_compilation() {
    printf("Testing method compilation and installation...\n");
    
    // Create a simple class to test method compilation
    const char* method_test_source = 
        "Calculator = Object (\n"
        "    add: x to: y = (\n"
        "        ^x + y\n"
        "    )\n"
        "    \n"
        "    multiply: x by: y = (\n"
        "        ^x * y\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)method_test_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    assert(class_ast != NULL);
    
    // Create a context and evaluate the class definition
    uint24_t context = ezom_create_extended_context(0, 0, 0, 5);
    assert(context != 0);
    
    ezom_eval_result_t result = ezom_evaluate_class_definition(class_ast, context);
    assert(!result.is_error);
    assert(result.value != 0);
    
    // Verify the Calculator class was created
    uint24_t calculator_class = ezom_lookup_global("Calculator");
    assert(calculator_class != 0);
    
    printf("✓ Method compilation test passed\n");
}

void test_empty_class_definition() {
    printf("Testing empty class definition...\n");
    
    const char* empty_class_source = "EmptyClass = Object ()\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)empty_class_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    assert(class_ast != NULL);
    assert(class_ast->type == AST_CLASS_DEF);
    assert(strcmp(class_ast->data.class_def.name, "EmptyClass") == 0);
    
    // Empty class should have no instance vars, instance methods, or class methods
    assert(class_ast->data.class_def.instance_vars == NULL);
    
    // Create a context and evaluate
    uint24_t context = ezom_create_extended_context(0, 0, 0, 5);
    assert(context != 0);
    
    ezom_eval_result_t result = ezom_evaluate_class_definition(class_ast, context);
    assert(!result.is_error);
    assert(result.value != 0);
    
    printf("✓ Empty class definition test passed\n");
}

void test_class_only_methods() {
    printf("Testing class with only class methods...\n");
    
    const char* class_methods_source = 
        "Utility = Object (\n"
        "    ----\n"
        "    \n"
        "    max: x and: y = (\n"
        "        x > y ifTrue: [^x] ifFalse: [^y]\n"
        "    )\n"
        "    \n"
        "    min: x and: y = (\n"
        "        x < y ifTrue: [^x] ifFalse: [^y]\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)class_methods_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    assert(class_ast != NULL);
    assert(class_ast->type == AST_CLASS_DEF);
    assert(strcmp(class_ast->data.class_def.name, "Utility") == 0);
    
    // Should have class methods but no instance methods
    assert(class_ast->data.class_def.class_methods != NULL);
    
    // Create a context and evaluate
    uint24_t context = ezom_create_extended_context(0, 0, 0, 5);
    assert(context != 0);
    
    ezom_eval_result_t result = ezom_evaluate_class_definition(class_ast, context);
    assert(!result.is_error);
    assert(result.value != 0);
    
    printf("✓ Class-only methods test passed\n");
}

int main() {
    printf("=== Comprehensive Class Definition Test Suite ===\n");
    
    // Initialize the VM systems
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_context_system();
    
    // Run comprehensive tests
    test_complete_class_system();
    test_inheritance_parsing();
    test_method_compilation();
    test_empty_class_definition();
    test_class_only_methods();
    
    printf("\n=== All Comprehensive Class Definition Tests Passed! ===\n");
    return 0;
}
