// ============================================================================
// File: test_phase1.c
// Test suite for Phase 1 functionality
// ============================================================================

#include "src/include/ezom_memory.h"
#include "src/include/ezom_object.h"
#include "src/include/ezom_primitives.h"
#include "src/include/ezom_dispatch.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static uint16_t tests_passed = 0;
static uint16_t tests_total = 0;

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

// Test basic memory allocation
bool test_memory_allocation() {
    uint24_t ptr1 = ezom_allocate(100);
    uint24_t ptr2 = ezom_allocate(50);
    
    return ptr1 != 0 && ptr2 != 0 && ptr1 != ptr2;
}

// Test object creation
bool test_object_creation() {
    uint24_t int_obj = ezom_create_integer(42);
    if (!int_obj) return false;
    
    ezom_integer_t* obj = (ezom_integer_t*)int_obj;
    return obj->value == 42 && 
           (obj->header.flags & 0xF0) == EZOM_TYPE_INTEGER;
}

// Test string creation
bool test_string_creation() {
    uint24_t str_obj = ezom_create_string("test", 4);
    if (!str_obj) return false;
    
    ezom_string_t* obj = (ezom_string_t*)str_obj;
    return obj->length == 4 && 
           strncmp(obj->data, "test", 4) == 0;
}

// Test symbol creation
bool test_symbol_creation() {
    uint24_t sym1 = ezom_create_symbol("test", 4);
    uint24_t sym2 = ezom_create_symbol("test", 4);
    
    return sym1 != 0 && sym2 != 0;
}

// Test method dispatch
bool test_method_dispatch() {
    if (!g_integer_class) return false;
    
    uint24_t obj1 = ezom_create_integer(5);
    uint24_t obj2 = ezom_create_integer(3);
    uint24_t plus_selector = ezom_create_symbol("+", 1);
    
    if (!obj1 || !obj2 || !plus_selector) return false;
    
    uint24_t result = ezom_send_binary_message(obj1, plus_selector, obj2);
    if (!result) return false;
    
    ezom_integer_t* result_obj = (ezom_integer_t*)result;
    return result_obj->value == 8;
}

// Test string concatenation
bool test_string_concat() {
    if (!g_string_class) return false;
    
    uint24_t str1 = ezom_create_string("Hello", 5);
    uint24_t str2 = ezom_create_string(" World", 6);
    uint24_t plus_selector = ezom_create_symbol("+", 1);
    
    if (!str1 || !str2 || !plus_selector) return false;
    
    uint24_t result = ezom_send_binary_message(str1, plus_selector, str2);
    if (!result) return false;
    
    ezom_string_t* result_str = (ezom_string_t*)result;
    return result_str->length == 11 && 
           strncmp(result_str->data, "Hello World", 11) == 0;
}

// Test object validation
bool test_object_validation() {
    uint24_t valid_obj = ezom_create_integer(42);
    uint24_t invalid_obj = 0x123456; // Invalid address
    
    return ezom_is_valid_object(valid_obj) && 
           !ezom_is_valid_object(invalid_obj);
}

// Test method lookup
bool test_method_lookup() {
    if (!g_integer_class) return false;
    
    uint24_t plus_selector = ezom_create_symbol("+", 1);
    if (!plus_selector) return false;
    
    ezom_method_lookup_t lookup = ezom_lookup_method(g_integer_class, plus_selector);
    
    return lookup.method != NULL && 
           lookup.is_primitive && 
           lookup.method->code == PRIM_INTEGER_ADD;
}

// Test primitive calls
bool test_primitive_calls() {
    uint24_t obj = ezom_create_integer(42);
    if (!obj) return false;
    
    // Test println primitive
    uint24_t println_selector = ezom_create_symbol("println", 7);
    if (!println_selector) return false;
    
    uint24_t result = ezom_send_unary_message(obj, println_selector);
    return result == obj; // println returns self
}

int main() {
    printf("EZOM Phase 1 Test Suite\n");
    printf("=======================\n");
    
    // Initialize VM
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_primitives();
    ezom_bootstrap_classes();
    
    printf("\nRunning tests...\n");
    
    // Run tests
    TEST(memory_allocation);
    TEST(object_creation);
    TEST(string_creation);
    TEST(symbol_creation);
    TEST(object_validation);
    TEST(method_lookup);
    TEST(method_dispatch);
    TEST(string_concat);
    TEST(primitive_calls);
    
    printf("\n======================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_total);
    
    if (tests_passed == tests_total) {
        printf("✓ All tests passed! Phase 1 is working correctly.\n");
        return 0;
    } else {
        printf("✗ Some tests failed. Check implementation.\n");
        return 1;
    }
}