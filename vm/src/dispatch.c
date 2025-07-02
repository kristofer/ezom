// ============================================================================
// File: src/vm/dispatch.c
// Method dispatch implementation
// ============================================================================

#include "../include/ezom_dispatch.h"
#include "../include/ezom_primitives.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// File-based logging for debugging
static FILE* g_log_file = NULL;

void ezom_log_init() {
    if (!g_log_file) {
        g_log_file = fopen("ezom.log", "w");
        if (g_log_file) {
            fprintf(g_log_file, "EZOM VM Debug Log Started\n");
            fprintf(g_log_file, "========================\n\n");
            fflush(g_log_file);
        }
    }
}

void ezom_log(const char* format, ...) {
    if (!g_log_file) ezom_log_init();
    if (g_log_file) {
        va_list args;
        va_start(args, format);
        vfprintf(g_log_file, format, args);
        va_end(args);
        fflush(g_log_file);
    }
}

void ezom_log_close() {
    if (g_log_file) {
        fprintf(g_log_file, "\nEZOM VM Debug Log Ended\n");
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

// External global variables
extern uint24_t g_nil;

// Helper function to compare two symbols by content
bool ezom_symbols_equal(uint24_t sym1, uint24_t sym2) {
    if (sym1 == sym2) return true;  // Same object
    if (!sym1 || !sym2) return false;  // One is null
    
    // Check if pointers are in valid memory range (ez80 heap: 0x042000-0x04FFFF)
    if (sym1 < 0x042000 || sym1 > 0x04FFFF) {
        return false;
    }
    if (sym2 < 0x042000 || sym2 > 0x04FFFF) {
        return false;
    }
    
    ezom_symbol_t* s1 = (ezom_symbol_t*)sym1;
    ezom_symbol_t* s2 = (ezom_symbol_t*)sym2;
    
    if (s1->length != s2->length) return false;
    
    return strncmp(s1->data, s2->data, s1->length) == 0;
}

ezom_method_lookup_t ezom_lookup_method(uint24_t class_ptr, uint24_t selector) {
    ezom_method_lookup_t result = {0};
    
    printf("DEBUG: ezom_lookup_method entry, class_ptr=0x%06lX, selector=0x%06lX\n", 
           (unsigned long)class_ptr, (unsigned long)selector);
    ezom_log("DEBUG: ezom_lookup_method entry, class_ptr=0x%06lX, selector=0x%06lX\n", 
             (unsigned long)class_ptr, (unsigned long)selector);
    
    if (!class_ptr || !selector) {
        printf("DEBUG: Null class_ptr or selector, returning empty result\n");
        ezom_log("DEBUG: Null class_ptr or selector, returning empty result\n");
        return result;
    }
    
    // Check for corrupted input pointers
    if (class_ptr == 0xffffff || selector == 0xffffff) {
        printf("DEBUG: Input corruption detected - class_ptr=0x%06lX, selector=0x%06lX\n",
               (unsigned long)class_ptr, (unsigned long)selector);
        ezom_log("DEBUG: Input corruption detected - class_ptr=0x%06lX, selector=0x%06lX\n",
                 (unsigned long)class_ptr, (unsigned long)selector);
        return result;
    }
    
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
            // Compare symbol content, not addresses
            if (ezom_symbols_equal(dict->methods[i].selector, selector)) {
                printf("DEBUG: Found method at index %d\n", i);
                printf("DEBUG: Method selector=0x%06lX, code=0x%06lX, flags=0x%02X\n",
                       (unsigned long)dict->methods[i].selector,
                       (unsigned long)dict->methods[i].code,
                       dict->methods[i].flags);
                ezom_log("DEBUG: Found method at index %d\n", i);
                ezom_log("DEBUG: Method selector=0x%06lX, code=0x%06lX, flags=0x%02X\n",
                         (unsigned long)dict->methods[i].selector,
                         (unsigned long)dict->methods[i].code,
                         dict->methods[i].flags);
                
                result.method = &dict->methods[i];
                result.class_ptr = (uint24_t)current_class;
                result.is_primitive = (dict->methods[i].flags & EZOM_METHOD_PRIMITIVE) != 0;
                
                printf("DEBUG: Returning method=0x%06lX, class_ptr=0x%06lX, is_primitive=%d\n",
                       (unsigned long)result.method, (unsigned long)result.class_ptr, result.is_primitive);
                ezom_log("DEBUG: Returning method=0x%06lX, class_ptr=0x%06lX, is_primitive=%d\n",
                         (unsigned long)result.method, (unsigned long)result.class_ptr, result.is_primitive);
                
                return result;
            }
        }
        
        // Move to superclass
        current_class = (ezom_class_t*)current_class->superclass;
    }
    
    return result;  // Method not found
}

uint24_t ezom_send_message(ezom_message_t* msg) {
    printf("DEBUG: ezom_send_message entry, receiver=0x%06X\n", msg->receiver);
    ezom_log("DEBUG: ezom_send_message entry, receiver=0x%06X\n", msg->receiver);
    
    // CRITICAL: Check for corruption pattern immediately and abort safely
    if (msg->receiver == 0xffffff) {
        printf("DEBUG: Corrupted receiver detected, returning early\n");
        ezom_log("DEBUG: Corrupted receiver detected, returning early\n");
        return g_nil ? g_nil : 0;
    }
    
    if (msg->args && msg->arg_count > 0) {
        for (int i = 0; i < msg->arg_count; i++) {
            if (msg->args[i] == 0xffffff) {
                printf("DEBUG: Corrupted arg[%d] detected, returning early\n", i);
                ezom_log("DEBUG: Corrupted arg[%d] detected, returning early\n", i);
                return g_nil ? g_nil : 0;
            }
        }
    }
    
    if (!msg->receiver) {
        return 0;
    }
    
    if (!ezom_is_valid_object(msg->receiver)) {
        return 0;
    }
    
    ezom_object_t* receiver = (ezom_object_t*)msg->receiver;
    
    // Look up method
    ezom_method_lookup_t lookup = ezom_lookup_method(receiver->class_ptr, msg->selector);
    
    if (!lookup.method) {
        return 0;
    }
    
    if (lookup.is_primitive) {
        // Call primitive function
        uint8_t prim_num = (uint8_t)lookup.method->code;
        printf("DEBUG: Found primitive %d\n", prim_num);
        ezom_log("DEBUG: Found primitive %d\n", prim_num);
        
        if (prim_num < MAX_PRIMITIVES && g_primitives[prim_num]) {
            // Emergency bypass for integer addition to prevent crash
            if (prim_num == PRIM_INTEGER_ADD) { // PRIM_INTEGER_ADD = 10
                printf("DEBUG: Using emergency bypass for integer addition\n");
                ezom_log("DEBUG: Using emergency bypass for integer addition\n");
                // Manual implementation of integer addition to avoid function call crash
                if (msg->receiver == 0xffffff || (msg->args && msg->args[0] == 0xffffff)) {
                    printf("DEBUG: Corrupted objects in addition, returning 0\n");
                    ezom_log("DEBUG: Corrupted objects in addition, returning 0\n");
                    return ezom_create_integer(0);
                }
                if (msg->arg_count == 1 && ezom_is_integer(msg->receiver) && ezom_is_integer(msg->args[0])) {
                    ezom_integer_t* recv = (ezom_integer_t*)msg->receiver;
                    ezom_integer_t* arg = (ezom_integer_t*)msg->args[0];
                    printf("DEBUG: Adding %d + %d\n", recv->value, arg->value);
                    ezom_log("DEBUG: Adding %d + %d\n", recv->value, arg->value);
                    return ezom_create_integer(recv->value + arg->value);
                }
                printf("DEBUG: Invalid types for addition, returning nil\n");
                ezom_log("DEBUG: Invalid types for addition, returning nil\n");
                return g_nil;
            }
            printf("DEBUG: About to call primitive function\n");
            ezom_log("DEBUG: About to call primitive function\n");
            return g_primitives[prim_num](msg->receiver, msg->args, msg->arg_count);
        } else {
            printf("DEBUG: Invalid primitive number or null function\n");
            ezom_log("DEBUG: Invalid primitive number or null function\n");
            return 0;
        }
    } else {
        printf("DEBUG: Bytecode method not implemented\n");
        ezom_log("DEBUG: Bytecode method not implemented\n");
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
    // Immediate corruption check
    if (receiver == 0xffffff || selector == 0xffffff || arg == 0xffffff) {
        return 0; // Return early to prevent crash
    }
    
    uint24_t args[] = {arg};
    
    ezom_message_t msg = {
        .selector = selector,
        .receiver = receiver,
        .args = args,
        .arg_count = 1
    };
    
    return ezom_send_message(&msg);
}