#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/ezom_object.h"
#include "../include/ezom_context.h"
#include "../include/ezom_memory.h"

// Simple test to verify enhanced block parameter binding
void test_simple_block_parameter() {
    printf("=== Simple Block Parameter Test ===\n");
    
    // Create a simple block object manually
    uint24_t block_ptr = ezom_allocate(sizeof(ezom_block_t));
    assert(block_ptr != 0);
    
    ezom_init_object(block_ptr, g_block_class, EZOM_TYPE_BLOCK);
    
    ezom_block_t* block = (ezom_block_t*)EZOM_OBJECT_PTR(block_ptr);
    block->outer_context = 0;
    block->code = 0;  // No AST for this simple test
    block->param_count = 2;
    block->local_count = 0;
    
    // Test enhanced block context creation
    uint24_t context = ezom_create_enhanced_block_context(0, block_ptr, 2, 0);
    assert(context != 0);
    
    printf("✓ Enhanced block context created successfully\n");
    
    // Test parameter binding
    uint24_t arg1 = ezom_create_integer(42);
    uint24_t arg2 = ezom_create_integer(84);
    uint24_t args[2] = {arg1, arg2};
    
    ezom_context_bind_block_parameters(context, args, 2, 2);
    
    // Test parameter retrieval
    ezom_context_t* ctx = (ezom_context_t*)EZOM_OBJECT_PTR(context);
    assert(ctx->locals[0] == arg1);
    assert(ctx->locals[1] == arg2);
    
    printf("✓ Block parameters bound and retrieved correctly\n");
    printf("=== Simple Block Parameter Test Passed ===\n");
}

int main() {
    printf("=== Simple Enhanced Block Test ===\n");
    
    // Initialize the VM systems
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_context_system();
    
    // Run simple test
    test_simple_block_parameter();
    
    printf("\n=== All Simple Tests Passed! ===\n");
    return 0;
}
