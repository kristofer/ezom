// ============================================================================
// File: src/context.c
// Execution context and block object implementation
// ============================================================================

#include "../include/ezom_context.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_primitives.h"
#include <stdio.h>
#include <string.h>

// Global object references (declared in bootstrap.c)
extern uint24_t g_block_class;
extern uint24_t g_context_class;
extern uint24_t g_boolean_class;
extern uint24_t g_true_class;
extern uint24_t g_false_class;
extern uint24_t g_true;
extern uint24_t g_false;
uint24_t g_current_context = 0;

// Context stack for execution
#define MAX_CONTEXT_STACK 64
static uint24_t context_stack[MAX_CONTEXT_STACK];
static uint8_t context_stack_top = 0;

void ezom_init_context_system(void) {
    printf("EZOM: Initializing context system...\n");
    context_stack_top = 0;
    g_current_context = 0;
    
    // Create Block class
    g_block_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_block_class) {
        ezom_init_object(g_block_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* block_class = (ezom_class_t*)g_block_class;
        block_class->superclass = g_object_class;
        block_class->method_dict = ezom_create_method_dictionary(8);
        block_class->instance_vars = 0;
        block_class->instance_size = sizeof(ezom_block_t);
        block_class->instance_var_count = 0;
        
        printf("   Block class created successfully\n");
    }
    
    // Create Context class
    g_context_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_context_class) {
        ezom_init_object(g_context_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* context_class = (ezom_class_t*)g_context_class;
        context_class->superclass = g_object_class;
        context_class->method_dict = ezom_create_method_dictionary(4);
        context_class->instance_vars = 0;
        context_class->instance_size = sizeof(ezom_context_t);
        context_class->instance_var_count = 0;
        
        printf("   Context class created successfully\n");
    }
    printf("EZOM: Context system initialization complete.\n");
}

uint24_t ezom_create_extended_context(uint24_t outer_context, uint24_t method, uint24_t receiver, uint8_t local_count) {
    uint16_t total_size = sizeof(ezom_context_t) + (local_count * sizeof(uint24_t));
    uint24_t ptr = ezom_allocate(total_size);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_context_class, EZOM_TYPE_OBJECT);
    
    ezom_context_t* context = (ezom_context_t*)ptr;
    context->outer_context = outer_context;
    context->method = method;
    context->receiver = receiver;
    context->sender = 0;  // Use Phase 1.5 structure
    context->pc = 0;      // Use Phase 1.5 structure
    context->local_count = local_count;
    
    // Initialize locals to nil
    for (uint8_t i = 0; i < local_count; i++) {
        context->locals[i] = g_nil;
    }
    
    return ptr;
}

uint24_t ezom_create_block_context(uint24_t outer_context, uint24_t block, uint8_t local_count) {
    uint24_t context = ezom_create_context(outer_context, local_count);
    if (!context) return 0;
    
    ezom_context_t* ctx = (ezom_context_t*)context;
    ctx->method = block; // Store block reference in method field
    
    return context;
}

void ezom_context_set_local(uint24_t context_ptr, uint8_t index, uint24_t value) {
    if (!context_ptr) return;
    
    ezom_context_t* context = (ezom_context_t*)context_ptr;
    if (index < context->local_count) {
        context->locals[index] = value;
    }
}

uint24_t ezom_context_get_local(uint24_t context_ptr, uint8_t index) {
    if (!context_ptr) return g_nil;
    
    ezom_context_t* context = (ezom_context_t*)context_ptr;
    if (index < context->local_count) {
        return context->locals[index];
    }
    
    return g_nil;
}

uint24_t ezom_context_lookup_variable(uint24_t context_ptr, const char* name) {
    // This is a simplified version - a full implementation would need
    // to track variable names and their indices
    if (!context_ptr || !name) return g_nil;
    
    ezom_context_t* context = (ezom_context_t*)context_ptr;
    
    // Look up in current context's locals
    // For now, just return nil - this would need a symbol table
    // to map variable names to indices
    
    // Check outer context
    if (context->outer_context) {
        return ezom_context_lookup_variable(context->outer_context, name);
    }
    
    return g_nil;
}

void ezom_context_bind_parameters(uint24_t context_ptr, uint24_t* args, uint8_t arg_count) {
    if (!context_ptr || !args) return;
    
    ezom_context_t* context = (ezom_context_t*)context_ptr;
    uint8_t param_count = arg_count < context->local_count ? arg_count : context->local_count;
    
    for (uint8_t i = 0; i < param_count; i++) {
        context->locals[i] = args[i];
    }
}

// Block object functions

uint24_t ezom_create_ast_block(ezom_ast_node_t* ast_node, uint24_t outer_context) {
    uint24_t ptr = ezom_allocate(sizeof(ezom_block_t));
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_block_class, EZOM_TYPE_OBJECT);
    
    ezom_block_t* block = (ezom_block_t*)ptr;
    block->outer_context = outer_context;
    block->code = (uint24_t)ast_node; // Store AST pointer in code field
    
    if (ast_node && ast_node->type == AST_BLOCK) {
        block->param_count = ezom_ast_count_parameters(ast_node->data.block.parameters);
        block->local_count = ezom_ast_count_locals(ast_node->data.block.locals);
    } else {
        block->param_count = 0;
        block->local_count = 0;
    }
    
    return ptr;
}

uint24_t ezom_block_evaluate(uint24_t block_ptr, uint24_t* args, uint8_t arg_count) {
    if (!block_ptr) return g_nil;
    
    ezom_block_t* block = (ezom_block_t*)block_ptr;
    
    // Check argument count
    if (arg_count != block->param_count) {
        printf("Block argument count mismatch: expected %d, got %d\n", 
               block->param_count, arg_count);
        return g_nil;
    }
    
    // Create block execution context
    uint24_t context = ezom_create_block_context(block->outer_context, block_ptr, 
                                                 block->local_count + block->param_count);
    if (!context) return g_nil;
    
    // Bind parameters
    ezom_context_bind_parameters(context, args, arg_count);
    
    // Push context and evaluate
    ezom_push_context(context);
    
    uint24_t result = g_nil;
    if (block->code) {
        ezom_ast_node_t* ast = (ezom_ast_node_t*)block->code;
        if (ast->type == AST_BLOCK && ast->data.block.body) {
            // Evaluate block body (this would use the AST evaluator)
            // For now, just return nil
            result = g_nil;
        }
    }
    
    ezom_pop_context();
    
    return result;
}

// Boolean object functions (now handled by bootstrap.c)

void ezom_init_boolean_objects(void) {
    // Boolean objects are now created in bootstrap.c
    // This function is kept for compatibility but does nothing
    return;
}

uint24_t ezom_create_boolean(bool value) {
    return value ? g_true : g_false;
}

bool ezom_is_true_object(uint24_t object_ptr) {
    return object_ptr == g_true;
}

bool ezom_is_false_object(uint24_t object_ptr) {
    return object_ptr == g_false;
}

// Context stack management

void ezom_push_context(uint24_t context_ptr) {
    if (context_stack_top < MAX_CONTEXT_STACK) {
        context_stack[context_stack_top++] = g_current_context;
        g_current_context = context_ptr;
    } else {
        printf("Context stack overflow!\n");
    }
}

uint24_t ezom_pop_context(void) {
    if (context_stack_top > 0) {
        uint24_t previous = g_current_context;
        g_current_context = context_stack[--context_stack_top];
        return previous;
    }
    return 0;
}

uint24_t ezom_get_current_context(void) {
    return g_current_context;
}

// Utility functions

bool ezom_is_block_object(uint24_t object_ptr) {
    if (!object_ptr) return false;
    ezom_object_t* obj = (ezom_object_t*)object_ptr;
    return obj->class_ptr == g_block_class;
}

bool ezom_is_context_object(uint24_t object_ptr) {
    if (!object_ptr) return false;
    ezom_object_t* obj = (ezom_object_t*)object_ptr;
    return obj->class_ptr == g_context_class;
}