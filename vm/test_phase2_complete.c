// ============================================================================
// File: test_phase2_complete.c
// Comprehensive test suite for EZOM Phase 2 implementation
// ============================================================================

#include "include/ezom_memory.h"
#include "include/ezom_object.h"
#include "include/ezom_primitives.h"
#include "include/ezom_dispatch.h"
#include "include/ezom_context.h"
#include "include/ezom_evaluator.h"
#include "include/ezom_lexer.h"
#include "include/ezom_parser.h"
#include "include/ezom_ast.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Test result tracking
static int tests_passed = 0;
static int tests_total = 0;

#define TEST(name) \
    do { \
        printf("Testing %s... ", #name); \
        tests_total++; \
        if (test_##name()) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else { \
            printf("FAIL\n"); \
        } \
    } while(0)

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s at line %d\n", #condition, __LINE__); \
            return false; \
        } \
    } while(0)

// Forward declarations
extern void ezom_bootstrap_classes(void);
extern void ezom_bootstrap_enhanced_classes(void);

// Test 1: Lexer tokenization
bool test_lexer_basic_tokenization() {
    ezom_lexer_t lexer;
    const char* source = "42 'hello' #symbol ClassName";
    
    ezom_lexer_init(&lexer, (char*)source);
    
    // Test integer token
    ASSERT(lexer.current_token.type == TOKEN_INTEGER);
    ASSERT(lexer.current_token.value.int_value == 42);
    
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_STRING);
    ASSERT(strcmp(lexer.current_token.value.string_value, "hello") == 0);
    
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_SYMBOL);
    ASSERT(strcmp(lexer.current_token.value.string_value, "symbol") == 0);
    
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_IDENTIFIER);
    ASSERT(strcmp(lexer.current_token.value.string_value, "ClassName") == 0);
    
    return true;
}

// Test 2: Lexer operators and special characters
bool test_lexer_operators() {
    ezom_lexer_t lexer;
    const char* source = "+ - * / = < > ( ) [ ] | . ^ :=";
    
    ezom_lexer_init(&lexer, (char*)source);
    
    ASSERT(lexer.current_token.type == TOKEN_PLUS);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_MINUS);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_MULTIPLY);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_DIVIDE);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_EQUALS);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_LT);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_GT);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_LPAREN);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_RPAREN);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_LBRACKET);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_RBRACKET);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_PIPE);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_DOT);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_CARET);
    ezom_lexer_next_token(&lexer);
    ASSERT(lexer.current_token.type == TOKEN_ASSIGN);
    
    return true;
}

// Test 3: AST creation
bool test_ast_creation() {
    // Test literal creation
    ezom_ast_node_t* int_node = ezom_ast_create_literal_integer(42);
    ASSERT(int_node != NULL);
    ASSERT(int_node->type == AST_LITERAL);
    ASSERT(int_node->data.literal.type == LITERAL_INTEGER);
    ASSERT(int_node->data.literal.value.integer_value == 42);
    
    ezom_ast_node_t* str_node = ezom_ast_create_literal_string("hello");
    ASSERT(str_node != NULL);
    ASSERT(str_node->type == AST_LITERAL);
    ASSERT(str_node->data.literal.type == LITERAL_STRING);
    ASSERT(strcmp(str_node->data.literal.value.string_value, "hello") == 0);
    
    // Test message send creation
    ezom_ast_node_t* receiver = ezom_ast_create_identifier("object");
    ezom_ast_node_t* message = ezom_ast_create_message_send(receiver, "method");
    ASSERT(message != NULL);
    ASSERT(message->type == AST_MESSAGE_SEND);
    ASSERT(strcmp(message->data.message_send.selector, "method") == 0);
    ASSERT(message->data.message_send.receiver == receiver);
    
    // Cleanup
    ezom_ast_free(int_node);
    ezom_ast_free(str_node);
    ezom_ast_free(message);
    
    return true;
}

// Test 4: Simple expression parsing
bool test_parser_simple_expression() {
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    const char* source = "42";
    
    ezom_lexer_init(&lexer, (char*)source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* expr = ezom_parse_expression(&parser);
    ASSERT(expr != NULL);
    ASSERT(expr->type == AST_LITERAL);
    ASSERT(expr->data.literal.type == LITERAL_INTEGER);
    ASSERT(expr->data.literal.value.integer_value == 42);
    
    ezom_ast_free(expr);
    return true;
}

// Test 5: Message send parsing
bool test_parser_message_send() {
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    const char* source = "object method";
    
    ezom_lexer_init(&lexer, (char*)source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* expr = ezom_parse_expression(&parser);
    ASSERT(expr != NULL);
    ASSERT(expr->type == AST_MESSAGE_SEND);
    ASSERT(strcmp(expr->data.message_send.selector, "method") == 0);
    ASSERT(expr->data.message_send.receiver != NULL);
    ASSERT(expr->data.message_send.receiver->type == AST_IDENTIFIER);
    
    ezom_ast_free(expr);
    return true;
}

// Test 6: Binary message parsing
bool test_parser_binary_message() {
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    const char* source = "5 + 3";
    
    ezom_lexer_init(&lexer, (char*)source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* expr = ezom_parse_expression(&parser);
    ASSERT(expr != NULL);
    ASSERT(expr->type == AST_MESSAGE_SEND);
    ASSERT(strcmp(expr->data.message_send.selector, "+") == 0);
    ASSERT(expr->data.message_send.receiver != NULL);
    ASSERT(expr->data.message_send.receiver->type == AST_LITERAL);
    ASSERT(expr->data.message_send.arguments != NULL);
    ASSERT(expr->data.message_send.arguments->type == AST_LITERAL);
    
    ezom_ast_free(expr);
    return true;
}

// Test 7: Block parsing
bool test_parser_block() {
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    const char* source = "[ 42 ]";
    
    ezom_lexer_init(&lexer, (char*)source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* expr = ezom_parse_expression(&parser);
    ASSERT(expr != NULL);
    ASSERT(expr->type == AST_BLOCK);
    ASSERT(expr->data.block.body != NULL);
    
    ezom_ast_free(expr);
    return true;
}

// Test 8: Literal evaluation
bool test_evaluator_literals() {
    uint24_t context = 0; // No context needed for literals
    
    // Test integer literal
    ezom_ast_node_t* int_node = ezom_ast_create_literal_integer(42);
    ezom_eval_result_t result = ezom_evaluate_literal(int_node, context);
    ASSERT(!result.is_error);
    ASSERT(result.value != 0);
    
    ezom_integer_t* int_obj = (ezom_integer_t*)result.value;
    ASSERT(int_obj->value == 42);
    
    // Test string literal
    ezom_ast_node_t* str_node = ezom_ast_create_literal_string("hello");
    result = ezom_evaluate_literal(str_node, context);
    ASSERT(!result.is_error);
    ASSERT(result.value != 0);
    
    ezom_string_t* str_obj = (ezom_string_t*)result.value;
    ASSERT(str_obj->length == 5);
    ASSERT(strncmp(str_obj->data, "hello", 5) == 0);
    
    ezom_ast_free(int_node);
    ezom_ast_free(str_node);
    return true;
}

// Test 9: Simple class definition
bool test_class_definition_creation() {
    // Create a simple class AST
    ezom_ast_node_t* superclass = ezom_ast_create_identifier("Object");
    ezom_ast_node_t* class_def = ezom_ast_create_class_def("TestClass", superclass);
    
    ASSERT(class_def != NULL);
    ASSERT(class_def->type == AST_CLASS_DEF);
    ASSERT(strcmp(class_def->data.class_def.name, "TestClass") == 0);
    ASSERT(class_def->data.class_def.superclass == superclass);
    
    ezom_ast_free(class_def);
    return true;
}

// Test 10: Method definition creation
bool test_method_definition_creation() {
    ezom_ast_node_t* method_def = ezom_ast_create_method_def("testMethod", false);
    
    ASSERT(method_def != NULL);
    ASSERT(method_def->type == AST_METHOD_DEF);
    ASSERT(strcmp(method_def->data.method_def.selector, "testMethod") == 0);
    ASSERT(method_def->data.method_def.is_class_method == false);
    
    ezom_ast_free(method_def);
    return true;
}

// Test 11: Integration test - parse and evaluate simple expression
bool test_integration_simple() {
    ezom_lexer_t lexer;
    ezom_parser_t parser;
    const char* source = "42";
    
    ezom_lexer_init(&lexer, (char*)source);
    ezom_parser_init(&parser, &lexer);
    
    ezom_ast_node_t* expr = ezom_parse_expression(&parser);
    ASSERT(expr != NULL);
    
    ezom_eval_result_t result = ezom_evaluate_ast(expr, 0);
    ASSERT(!result.is_error);
    ASSERT(result.value != 0);
    
    ezom_integer_t* int_obj = (ezom_integer_t*)result.value;
    ASSERT(int_obj->value == 42);
    
    ezom_ast_free(expr);
    return true;
}

// Test 12: Context system
bool test_context_system() {
    uint24_t context = ezom_create_context(0, 5);
    ASSERT(context != 0);
    
    // Test variable setting and getting
    ezom_context_set_local(context, 0, ezom_create_integer(42));
    uint24_t value = ezom_context_get_local(context, 0);
    ASSERT(value != 0);
    
    ezom_integer_t* int_obj = (ezom_integer_t*)value;
    ASSERT(int_obj->value == 42);
    
    return true;
}

// Test 13: Block object creation and evaluation
bool test_block_objects() {
    // Create a simple block AST
    ezom_ast_node_t* block_ast = ezom_ast_create_block();
    ezom_ast_node_t* literal = ezom_ast_create_literal_integer(42);
    ezom_ast_node_t* statements = ezom_ast_create_statement_list();
    ezom_ast_add_statement(statements, literal);
    block_ast->data.block.body = statements;
    
    // Create block object
    uint24_t block_obj = ezom_create_ast_block(block_ast, 0);
    ASSERT(block_obj != 0);
    
    // Test block evaluation
    uint24_t result = ezom_block_evaluate(block_obj, NULL, 0);
    ASSERT(result != 0);
    
    ezom_integer_t* int_obj = (ezom_integer_t*)result;
    ASSERT(int_obj->value == 42);
    
    ezom_ast_free(block_ast);
    return true;
}

// Test 14: Enhanced class creation
bool test_enhanced_class_creation() {
    uint24_t class_obj = ezom_create_class_with_inheritance("TestClass", g_object_class, 2);
    ASSERT(class_obj != 0);
    
    ezom_class_t* class_struct = (ezom_class_t*)class_obj;
    ASSERT(class_struct->superclass == g_object_class);
    ASSERT(class_struct->instance_var_count == 2);
    ASSERT(class_struct->method_dict != 0);
    
    return true;
}

// Test 15: Method installation
bool test_method_installation() {
    uint24_t class_obj = ezom_create_class_with_inheritance("TestClass", g_object_class, 0);
    ASSERT(class_obj != 0);
    
    // Install a test method
    ezom_install_method_in_class(class_obj, "testMethod", PRIM_OBJECT_PRINTLN, 0, true);
    
    // Verify method was installed
    uint24_t selector = ezom_create_symbol("testMethod", 10);
    ezom_method_lookup_t lookup = ezom_lookup_method(class_obj, selector);
    ASSERT(lookup.method != NULL);
    ASSERT(lookup.is_primitive == true);
    ASSERT(lookup.method->code == PRIM_OBJECT_PRINTLN);
    
    return true;
}

// Test 16: Full class definition evaluation
bool test_full_class_evaluation() {
    // Create a complete class definition AST
    ezom_ast_node_t* superclass = ezom_ast_create_identifier("Object");
    ezom_ast_node_t* class_def = ezom_ast_create_class_def("TestClass", superclass);
    
    // Add a simple method
    ezom_ast_node_t* method_def = ezom_ast_create_method_def("getValue", false);
    ezom_ast_node_t* method_body = ezom_ast_create_statement_list();
    ezom_ast_node_t* return_stmt = ezom_ast_create_return(ezom_ast_create_literal_integer(42));
    ezom_ast_add_statement(method_body, return_stmt);
    method_def->data.method_def.body = method_body;
    
    ezom_ast_node_t* instance_methods = ezom_ast_create_statement_list();
    ezom_ast_add_statement(instance_methods, method_def);
    class_def->data.class_def.instance_methods = instance_methods;
    
    // Evaluate class definition
    ezom_eval_result_t result = ezom_evaluate_class_definition(class_def, 0);
    ASSERT(!result.is_error);
    ASSERT(result.value != 0);
    
    // Verify class was created
    uint24_t class_obj = ezom_lookup_global("TestClass");
    ASSERT(class_obj != 0);
    ASSERT(class_obj == result.value);
    
    ezom_ast_free(class_def);
    return true;
}

// Test initialization function
void init_test_environment() {
    // Initialize VM components
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_primitives();
    
    // Bootstrap basic classes
    ezom_bootstrap_classes();
    ezom_bootstrap_enhanced_classes();
    
    // Initialize Phase 2 systems
    ezom_init_context_system();
    ezom_init_boolean_objects();
    ezom_evaluator_init();
}

// Main test runner
int main() {
    printf("EZOM Phase 2 Comprehensive Test Suite\n");
    printf("=====================================\n\n");
    
    // Initialize test environment
    printf("Initializing test environment...\n");
    init_test_environment();
    printf("Initialization complete.\n\n");
    
    // Run lexer tests
    printf("=== Lexer Tests ===\n");
    TEST(lexer_basic_tokenization);
    TEST(lexer_operators);
    
    // Run AST tests
    printf("\n=== AST Tests ===\n");
    TEST(ast_creation);
    
    // Run parser tests
    printf("\n=== Parser Tests ===\n");
    TEST(parser_simple_expression);
    TEST(parser_message_send);
    TEST(parser_binary_message);
    TEST(parser_block);
    
    // Run evaluator tests
    printf("\n=== Evaluator Tests ===\n");
    TEST(evaluator_literals);
    
    // Run class and method tests
    printf("\n=== Class System Tests ===\n");
    TEST(class_definition_creation);
    TEST(method_definition_creation);
    TEST(enhanced_class_creation);
    TEST(method_installation);
    
    // Run integration tests
    printf("\n=== Integration Tests ===\n");
    TEST(integration_simple);
    TEST(context_system);
    TEST(block_objects);
    TEST(full_class_evaluation);
    
    // Print final results
    printf("\n=====================================\n");
    printf("Test Results: %d/%d tests passed\n", tests_passed, tests_total);
    printf("Success rate: %.1f%%\n", (tests_passed * 100.0) / tests_total);
    
    if (tests_passed == tests_total) {
        printf("\n✅ ALL TESTS PASSED! Phase 2 implementation is complete.\n");
    } else {
        printf("\n❌ Some tests failed. Phase 2 needs more work.\n");
    }
    
    return tests_passed == tests_total ? 0 : 1;
}