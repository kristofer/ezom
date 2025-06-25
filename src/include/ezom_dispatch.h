// ============================================================================
// File: src/include/ezom_dispatch.h
// Method dispatch system
// ============================================================================

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ezom_object.h"

typedef struct ezom_message {
    uint24_t selector;      // Symbol for method name
    uint24_t receiver;      // Object receiving message
    uint24_t* args;         // Array of arguments
    uint8_t  arg_count;     // Number of arguments
} ezom_message_t;

// Method lookup result
typedef struct ezom_method_lookup {
    ezom_method_t* method;  // Found method or NULL
    uint24_t       class_ptr; // Class where method was found
    bool           is_primitive; // True if primitive method
} ezom_method_lookup_t;

// Message dispatch functions
ezom_method_lookup_t ezom_lookup_method(uint24_t class_ptr, uint24_t selector);
uint24_t ezom_send_message(ezom_message_t* msg);
uint24_t ezom_send_unary_message(uint24_t receiver, uint24_t selector);
uint24_t ezom_send_binary_message(uint24_t receiver, uint24_t selector, uint24_t arg);