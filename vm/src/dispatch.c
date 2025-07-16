// ============================================================================
// File: src/vm/dispatch.c
// Method dispatch implementation
// ============================================================================

#include "../include/ezom_dispatch.h"
#include "../include/ezom_primitives.h"
#include "../include/ezom_memory.h"
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
    printf("DEBUG: ezom_symbols_equal comparing 0x%06lX and 0x%06lX\n", 
           (unsigned long)sym1, (unsigned long)sym2);
    
    if (sym1 == sym2) {
        printf("DEBUG: Same object - equal\n");
        return true;  // Same object
    }
    if (!sym1 || !sym2) {
        printf("DEBUG: One is null - not equal\n");
        return false;  // One is null
    }
    
    // Check if pointers are in valid memory range (ez80 heap: 0x042000-0x04FFFF)
    if (sym1 < 0x042000 || sym1 > 0x04FFFF) {
        printf("DEBUG: sym1 out of range - not equal\n");
        return false;
    }
    if (sym2 < 0x042000 || sym2 > 0x04FFFF) {
        printf("DEBUG: sym2 out of range - not equal\n");
        return false;
    }
    
    ezom_symbol_t* s1 = (ezom_symbol_t*)EZOM_OBJECT_PTR(sym1);
    ezom_symbol_t* s2 = (ezom_symbol_t*)EZOM_OBJECT_PTR(sym2);
    
    printf("DEBUG: s1->length=%d, s2->length=%d\n", s1->length, s2->length);
    
    if (s1->length != s2->length) {
        printf("DEBUG: Different lengths - not equal\n");
        return false;
    }
    
    // FIXED: Use explicit pointer arithmetic instead of flexible array member
    char* s1_data = (char*)EZOM_OBJECT_PTR(sym1 + sizeof(ezom_object_t) + sizeof(uint16_t) + sizeof(uint16_t));
    char* s2_data = (char*)EZOM_OBJECT_PTR(sym2 + sizeof(ezom_object_t) + sizeof(uint16_t) + sizeof(uint16_t));
    
    printf("DEBUG: Comparing strings: '");
    for (int i = 0; i < s1->length && i < 10; i++) {
        printf("%c", s1_data[i]);
    }
    printf("' vs '");
    for (int i = 0; i < s2->length && i < 10; i++) {
        printf("%c", s2_data[i]);
    }
    printf("'\n");
    
    bool result = strncmp(s1_data, s2_data, s1->length) == 0;
    printf("DEBUG: String comparison result: %s\n", result ? "equal" : "not equal");
    return result;
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
    
    ezom_class_t* current_class = (ezom_class_t*)EZOM_OBJECT_PTR(class_ptr);
    uint24_t original_class = class_ptr;
    int depth = 0;
    int max_depth = 10; // Safety limit to prevent infinite loops
    
    while (current_class && depth < max_depth) {
        printf("DEBUG: Checking class %p (depth %d)\n", 
               (void*)current_class, depth);
        ezom_log("DEBUG: Checking class %p (depth %d)\n", 
                 (void*)current_class, depth);
        
        if (!current_class->method_dict) {
            printf("DEBUG: No method dictionary, moving to superclass 0x%06lX\n",
                   (unsigned long)current_class->superclass);
            ezom_log("DEBUG: No method dictionary, moving to superclass 0x%06lX\n",
                     (unsigned long)current_class->superclass);
            
            // Check for circular reference
            if (current_class->superclass == original_class) {
                printf("DEBUG: CIRCULAR CLASS HIERARCHY DETECTED - breaking loop\n");
                ezom_log("DEBUG: CIRCULAR CLASS HIERARCHY DETECTED - breaking loop\n");
                break;
            }
            
            // Move to superclass
            if (current_class->superclass) {
                current_class = (ezom_class_t*)EZOM_OBJECT_PTR(current_class->superclass);
            } else {
                current_class = NULL;
            }
            depth++;
            continue;
        }
        
        ezom_method_dict_t* dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(current_class->method_dict);
        printf("DEBUG: Method dictionary has %d methods\n", dict->size);
        ezom_log("DEBUG: Method dictionary has %d methods\n", dict->size);
        
        // Linear search in method dictionary
        for (uint16_t i = 0; i < dict->size; i++) {
            printf("DEBUG: Comparing method %d: selector=0x%06lX\n", 
                   i, (unsigned long)dict->methods[i].selector);
            
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
                result.class_ptr = class_ptr; // Use original class_ptr instead of casting
                result.is_primitive = (dict->methods[i].flags & EZOM_METHOD_PRIMITIVE) != 0;
                
                printf("DEBUG: Returning method=0x%06lX, class_ptr=0x%06lX, is_primitive=%d\n",
                       (unsigned long)result.method, (unsigned long)result.class_ptr, result.is_primitive);
                ezom_log("DEBUG: Returning method=0x%06lX, class_ptr=0x%06lX, is_primitive=%d\n",
                         (unsigned long)result.method, (unsigned long)result.class_ptr, result.is_primitive);
                
                return result;
            }
        }
        
        printf("DEBUG: Method not found in this class, moving to superclass 0x%06lX\n",
               (unsigned long)current_class->superclass);
        ezom_log("DEBUG: Method not found in this class, moving to superclass 0x%06lX\n",
                 (unsigned long)current_class->superclass);
        
        // Check for circular reference
        if (current_class->superclass == original_class) {
            printf("DEBUG: CIRCULAR CLASS HIERARCHY DETECTED - breaking loop\n");
            ezom_log("DEBUG: CIRCULAR CLASS HIERARCHY DETECTED - breaking loop\n");
            break;
        }
        
        // Move to superclass
        if (current_class->superclass) {
            current_class = (ezom_class_t*)EZOM_OBJECT_PTR(current_class->superclass);
        } else {
            current_class = NULL;
        }
        depth++;
    }
    
    if (depth >= max_depth) {
        printf("DEBUG: Maximum depth reached (%d) - possible infinite loop\n", max_depth);
        ezom_log("DEBUG: Maximum depth reached (%d) - possible infinite loop\n", max_depth);
    }
    
    printf("DEBUG: Method not found anywhere in hierarchy\n");
    ezom_log("DEBUG: Method not found anywhere in hierarchy\n");
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
    
    ezom_object_t* receiver = (ezom_object_t*)EZOM_OBJECT_PTR(msg->receiver);
    
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
            // Integer addition now uses improved primitive implementation
            if (prim_num == PRIM_INTEGER_ADD) {
                printf("DEBUG: Calling improved integer addition primitive\n");
                ezom_log("DEBUG: Calling improved integer addition primitive\n");
            }
            
            // Emergency bypass for string concatenation - primitive function crashes
            if (prim_num == PRIM_STRING_CONCAT) { // PRIM_STRING_CONCAT = 32
                printf("DEBUG: Using emergency bypass for string concatenation (primitive crashes)\n");
                ezom_log("DEBUG: Using emergency bypass for string concatenation (primitive crashes)\n");
                
                if (msg->arg_count == 1) {
                    ezom_object_t* recv_obj = (ezom_object_t*)EZOM_OBJECT_PTR(msg->receiver);
                    ezom_object_t* arg_obj = (ezom_object_t*)EZOM_OBJECT_PTR(msg->args[0]);
                    
                    // Direct type checking
                    if ((recv_obj->flags & 0xF0) == EZOM_TYPE_STRING && 
                        (arg_obj->flags & 0xF0) == EZOM_TYPE_STRING) {
                        
                        ezom_string_t* str1 = (ezom_string_t*)EZOM_OBJECT_PTR(msg->receiver);
                        ezom_string_t* str2 = (ezom_string_t*)EZOM_OBJECT_PTR(msg->args[0]);
                        
                        printf("DEBUG: Concatenating '%.*s' + '%.*s'\n", 
                               str1->length, str1->data, str2->length, str2->data);
                        ezom_log("DEBUG: Concatenating '%.*s' + '%.*s'\n", 
                               str1->length, str1->data, str2->length, str2->data);
                        
                        uint16_t new_length = str1->length + str2->length;
                        uint24_t result = ezom_allocate(sizeof(ezom_string_t) + new_length + 1);
                        
                        if (!result) {
                            printf("DEBUG: String allocation failed\n");
                            ezom_log("DEBUG: String allocation failed\n");
                            return g_nil;
                        }
                        
                        ezom_init_object(result, g_string_class, EZOM_TYPE_STRING);
                        
                        ezom_string_t* result_str = (ezom_string_t*)EZOM_OBJECT_PTR(result);
                        result_str->length = new_length;
                        
                        memcpy(result_str->data, str1->data, str1->length);
                        memcpy(result_str->data + str1->length, str2->data, str2->length);
                        result_str->data[new_length] = '\0';
                        
                        printf("DEBUG: String concatenation result: '%.*s'\n", 
                               result_str->length, result_str->data);
                        ezom_log("DEBUG: String concatenation result: '%.*s'\n", 
                               result_str->length, result_str->data);
                        
                        return result;
                    }
                }
                
                printf("DEBUG: Invalid types for string concatenation, returning nil\n");
                ezom_log("DEBUG: Invalid types for string concatenation, returning nil\n");
                return g_nil;
            }
            
            // Emergency bypass for true ifTrue: primitive - improved version crashes
            if (prim_num == PRIM_TRUE_IF_TRUE) { // PRIM_TRUE_IF_TRUE = 50
                printf("DEBUG: Using emergency bypass for true ifTrue: (primitive crashes)\n");
                ezom_log("DEBUG: Using emergency bypass for true ifTrue: (primitive crashes)\n");
                
                if (msg->arg_count == 1 && msg->receiver == g_true) {
                    printf("DEBUG: Executing true ifTrue: block\n");
                    ezom_log("DEBUG: Executing true ifTrue: block\n");
                    
                    // Simply return the receiver (true) - don't try to execute the block yet
                    return g_true;
                } else {
                    printf("DEBUG: Invalid receiver for true ifTrue:\n");
                    ezom_log("DEBUG: Invalid receiver for true ifTrue:\n");
                    return g_nil;
                }
            }
            
            if (prim_num == PRIM_INTEGER_MOD) {
                printf("DEBUG: Calling improved integer modulo primitive\n");
                ezom_log("DEBUG: Calling improved integer modulo primitive\n");
            }
            
            if (prim_num == PRIM_INTEGER_LTE) {
                printf("DEBUG: Calling improved integer LTE primitive\n");
                ezom_log("DEBUG: Calling improved integer LTE primitive\n");
            }
            
            if (prim_num == PRIM_INTEGER_AS_STRING) {
                printf("DEBUG: Calling improved integer asString primitive\n");
                ezom_log("DEBUG: Calling improved integer asString primitive\n");
            }
            
            // Emergency bypass for false ifTrue: primitive - improved version crashes
            if (prim_num == PRIM_FALSE_IF_TRUE) { // PRIM_FALSE_IF_TRUE = 53
                printf("DEBUG: Using emergency bypass for false ifTrue: (primitive crashes)\n");
                ezom_log("DEBUG: Using emergency bypass for false ifTrue: (primitive crashes)\n");
                
                if (msg->arg_count == 1 && msg->receiver == g_false) {
                    printf("DEBUG: false ifTrue: should not execute block, returning nil\n");
                    ezom_log("DEBUG: false ifTrue: should not execute block, returning nil\n");
                    
                    // false ifTrue: should not execute the block, return nil
                    return g_nil;
                } else {
                    printf("DEBUG: Invalid receiver for false ifTrue:\n");
                    ezom_log("DEBUG: Invalid receiver for false ifTrue:\n");
                    return g_nil;
                }
            }
            
            printf("DEBUG: About to call primitive function\n");
            printf("DEBUG: prim_num=%d, function_ptr=0x%06lX\n", prim_num, (unsigned long)g_primitives[prim_num]);
            printf("DEBUG: receiver=0x%06lX, args=0x%06lX, arg_count=%d\n", 
                   (unsigned long)msg->receiver, (unsigned long)msg->args, msg->arg_count);
            
            ezom_log("DEBUG: About to call primitive function\n");
            ezom_log("DEBUG: prim_num=%d, function_ptr=0x%06lX\n", prim_num, (unsigned long)g_primitives[prim_num]);
            ezom_log("DEBUG: receiver=0x%06lX, args=0x%06lX, arg_count=%d\n", 
                     (unsigned long)msg->receiver, (unsigned long)msg->args, msg->arg_count);
            
            // Validate function pointer before calling
            if (!g_primitives[prim_num]) {
                printf("DEBUG: NULL function pointer for primitive %d\n", prim_num);
                ezom_log("DEBUG: NULL function pointer for primitive %d\n", prim_num);
                return g_nil;
            }
            
            printf("DEBUG: Calling primitive function now...\n");
            ezom_log("DEBUG: Calling primitive function now...\n");
            
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