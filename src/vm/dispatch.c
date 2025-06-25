// ============================================================================
// File: src/vm/dispatch.c
// Method dispatch implementation
// ============================================================================

#include "../include/ezom_dispatch.h"
#include "../include/ezom_primitives.h"
#include <stdio.h>

ezom_method_lookup_t ezom_lookup_method(uint24_t class_ptr, uint24_t selector) {
    ezom_method_lookup_t result = {0};
    
    if (!class_ptr) return result;
    
    ezom_class_t* current_class = (ezom_class_t*)class_ptr;
    
    while (current_class) {
        if (!current_class->method_dict) {
            // Move to superclass
            current_class = (ezom_class_t*)current_class->superclass;
            continue;
        }
        
        ezom_method_dict_t* dict = (ezom_method_dict_t*)current_class->method_dict;
        
        // Linear search in method dictionary
        for (uint16_t i = 0; i < dict->size; i++) {
            if (dict->methods[i].selector == selector) {
                result.method = &dict->methods[i];
                result.class_ptr = (uint24_t)current_class;
                result.is_primitive = (dict->methods[i].flags & EZOM_METHOD_PRIMITIVE) != 0;
                return result;
            }
        }
        
        // Move to superclass
        current_class = (ezom_class_t*)current_class->superclass;
    }
    
    return result;  // Method not found
}

uint24_t ezom_send_message(ezom_message_t* msg) {
    if (!msg->receiver) {
        printf("Error: Message sent to nil\n");
        return 0;
    }
    
    ezom_object_t* receiver = (ezom_object_t*)msg->receiver;
    
    // Look up method
    ezom_method_lookup_t lookup = ezom_lookup_method(receiver->class_ptr, msg->selector);
    
    if (!lookup.method) {
        printf("Method not found for selector\n");
        return 0;
    }
    
    if (lookup.is_primitive) {
        // Call primitive function
        uint8_t prim_num = (uint8_t)lookup.method->code;
        if (prim_num < MAX_PRIMITIVES && g_primitives[prim_num]) {
            return g_primitives[prim_num](msg->receiver, msg->args, msg->arg_count);
        } else {
            printf("Invalid primitive number: %d\n", prim_num);
            return 0;
        }
    } else {
        // Bytecode methods not implemented in Phase 1
        printf("Bytecode methods not implemented yet\n");
        return 0;
    }
}

uint24_t ezom_send_unary_message(uint24_t receiver, uint24_t selector) {
    ezom_message_t msg = {
        .selector = selector,
        .receiver = receiver,
        .args = NULL,
        .arg_count = 0
    };
    
    return ezom_send_message(&msg);
}

uint24_t ezom_send_binary_message(uint24_t receiver, uint24_t selector, uint24_t arg) {
    uint24_t args[] = {arg};
    
    ezom_message_t msg = {
        .selector = selector,
        .receiver = receiver,
        .args = args,
        .arg_count = 1
    };
    
    return ezom_send_message(&msg);
}