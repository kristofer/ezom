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

// Test class definition parsing
void test_class_definition_parsing() {
    printf("Testing class definition parsing...\n");
    
    // Test source: Simple Counter class
    const char* class_source = 
        "Counter = Object (\n"
        "    | value |\n"
        "    \n"
        "    initialize = (\n"
        "        value := 0.\n"
        "        ^self\n"
        "    )\n"
        "    \n"
        "    increment = (\n"
        "        value := value + 1.\n"
        "        ^value\n"
        "    )\n"
        "    \n"
        "    ----\n"
        "    \n"
        "    new = (\n"
        "        ^super new initialize\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)class_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    assert(class_ast != NULL);
    assert(class_ast->type == AST_CLASS_DEF);
    
    // Check class name
    assert(strcmp(class_ast->data.class_def.name, "Counter") == 0);
    
    // Check superclass
    assert(class_ast->data.class_def.superclass != NULL);
    assert(class_ast->data.class_def.superclass->type == AST_IDENTIFIER);
    assert(strcmp(class_ast->data.class_def.superclass->data.identifier.name, "Object") == 0);
    
    // Check instance variables
    assert(class_ast->data.class_def.instance_vars != NULL);
    
    // Check instance methods
    assert(class_ast->data.class_def.instance_methods != NULL);
    
    // Check class methods
    assert(class_ast->data.class_def.class_methods != NULL);
    
    printf("✓ Class definition parsing test passed\n");
}

void test_class_definition_evaluation() {
    printf("Testing class definition evaluation...\n");
    
    // Test source: Simple class
    const char* class_source = 
        "TestClass = Object (\n"
        "    | value |\n"
        "    \n"
        "    getValue = (\n"
        "        ^value\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)class_source);
    
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
    
    // Verify the class was created and registered as global
    uint24_t class_obj = ezom_lookup_global("TestClass");
    assert(class_obj != 0);
    assert(class_obj == result.value);
    
    printf("✓ Class definition evaluation test passed\n");
}

void test_method_definition_parsing() {
    printf("Testing method definition parsing...\n");
    
    // Test source: Method with parameters
    const char* method_source = 
        "setValue: newValue = (\n"
        "    value := newValue.\n"
        "    ^self\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)method_source);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* method_ast = ezom_parse_method_definition(&parser, false);
    assert(method_ast != NULL);
    assert(method_ast->type == AST_METHOD_DEF);
    
    // Check method selector
    assert(method_ast->data.method_def.selector != NULL);
    assert(strcmp(method_ast->data.method_def.selector, "setValue:") == 0);
    
    // Check method parameters
    assert(method_ast->data.method_def.parameters != NULL);
    
    // Check method body
    assert(method_ast->data.method_def.body != NULL);
    
    printf("✓ Method definition parsing test passed\n");
}

void test_simple_class_creation() {
    printf("Testing simple class creation...\n");
    
    // Test source: Minimal class
    const char* simple_class = 
        "SimpleClass = Object (\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, (char*)simple_class);
    
    ezom_parser_t parser;
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&parser);
    assert(class_ast != NULL);
    assert(class_ast->type == AST_CLASS_DEF);
    assert(strcmp(class_ast->data.class_def.name, "SimpleClass") == 0);
    
    // Create a context and evaluate
    uint24_t context = ezom_create_extended_context(0, 0, 0, 5);
    assert(context != 0);
    
    ezom_eval_result_t result = ezom_evaluate_class_definition(class_ast, context);
    assert(!result.is_error);
    assert(result.value != 0);
    
    printf("✓ Simple class creation test passed\n");
}

int main() {
    printf("=== Class Definition Parser Test Suite ===\n");
    
    // Initialize the VM systems
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_context_system();
    
    // Run class definition tests
    test_class_definition_parsing();
    test_method_definition_parsing();
    test_simple_class_creation();
    test_class_definition_evaluation();
    
    printf("\n=== All Class Definition Tests Passed! ===\n");
    return 0;
}
