// ============================================================================
// File: src/include/ezom_object.h
// Core object system definitions
// ============================================================================

#pragma once
#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct ezom_ast_node ezom_ast_node_t;

// Object header - 6 bytes total in ADL mode
typedef struct ezom_object {
    uint24_t class_ptr;     // Pointer to class object (3 bytes)
    uint16_t hash;          // Object hash code (2 bytes) 
    uint8_t  flags;         // GC and type flags (1 byte)
} ezom_object_t;

// Flag definitions
#define EZOM_FLAG_MARKED    0x01    // GC mark bit
#define EZOM_FLAG_FIXED     0x02    // Fixed size object
#define EZOM_FLAG_WEAK      0x04    // Weak reference
#define EZOM_FLAG_FINALIZE  0x08    // Has finalizer

// Object types for built-in classes (ENHANCED)
#define EZOM_TYPE_OBJECT    0x10
#define EZOM_TYPE_INTEGER   0x20
#define EZOM_TYPE_STRING    0x30
#define EZOM_TYPE_ARRAY     0x40
#define EZOM_TYPE_CLASS     0x50
#define EZOM_TYPE_BLOCK     0x60    // NEW: Block objects
#define EZOM_TYPE_BOOLEAN   0x70    // NEW: Boolean objects
#define EZOM_TYPE_NIL       0x80    // NEW: Nil object

// Class object layout
typedef struct ezom_class {
    ezom_object_t header;           // Standard object header
    uint24_t      superclass;       // Pointer to superclass
    uint24_t      method_dict;      // Pointer to method dictionary
    uint24_t      instance_vars;    // Pointer to instance variable names
    uint16_t      instance_size;    // Size of instances in bytes
    uint16_t      instance_var_count; // Number of instance variables
} ezom_class_t;

// Method dictionary entry
typedef struct ezom_method {
    uint24_t selector;      // Pointer to selector symbol
    uint24_t code;          // Pointer to method code/primitive
    uint16_t arg_count;     // Number of arguments
    uint8_t  flags;         // Method flags (primitive, etc.)
} ezom_method_t;

// Method dictionary
typedef struct ezom_method_dict {
    ezom_object_t header;
    uint16_t      size;         // Number of methods
    uint16_t      capacity;     // Allocated capacity
    ezom_method_t methods[];    // Variable length array
} ezom_method_dict_t;

#define EZOM_METHOD_PRIMITIVE   0x01
#define EZOM_METHOD_SUPER       0x02

// Built-in object types
typedef struct ezom_integer {
    ezom_object_t header;
    int16_t       value;        // 16-bit signed integer
} ezom_integer_t;

typedef struct ezom_string {
    ezom_object_t header;
    uint16_t      length;       // String length
    char          data[];       // Variable length string data
} ezom_string_t;

typedef struct ezom_symbol {
    ezom_object_t header;
    uint16_t      length;
    uint16_t      hash_cache;   // Cached hash for fast lookup
    char          data[];
} ezom_symbol_t;

// NEW: Array object
typedef struct ezom_array {
    ezom_object_t header;
    uint16_t      size;         // Number of elements
    uint24_t      elements[];   // Variable length array of object pointers
} ezom_array_t;

// NEW: Block object (closure)
typedef struct ezom_block {
    ezom_object_t header;
    uint24_t      outer_context;    // Lexical environment
    uint24_t      code;             // AST or bytecode pointer
    uint8_t       param_count;      // Number of parameters
    uint8_t       local_count;      // Number of local variables
    uint24_t      captured_vars[];  // Captured variables from outer scope
} ezom_block_t;

// NEW: Execution context for blocks and methods
typedef struct ezom_context {
    ezom_object_t header;
    uint24_t      outer_context;    // Parent context
    uint24_t      method;           // Method being executed
    uint24_t      receiver;         // Message receiver
    uint24_t      sender;           // Context that sent the message
    uint8_t       pc;               // Program counter
    uint8_t       local_count;      // Number of locals
    uint24_t      locals[];         // Local variables
} ezom_context_t;

// Global class pointers (ENHANCED)
extern uint24_t g_object_class;
extern uint24_t g_class_class;
extern uint24_t g_integer_class;
extern uint24_t g_string_class;
extern uint24_t g_symbol_class;
extern uint24_t g_array_class;      // NEW
extern uint24_t g_block_class;      // NEW
extern uint24_t g_boolean_class;    // NEW
extern uint24_t g_true_class;       // NEW
extern uint24_t g_false_class;      // NEW
extern uint24_t g_nil_class;        // NEW
extern uint24_t g_context_class;    // NEW

// Global singleton objects
extern uint24_t g_nil;              // ENHANCED
extern uint24_t g_true;             // NEW
extern uint24_t g_false;            // NEW

// Object system functions
void ezom_init_object_system(void);
void ezom_init_object(uint24_t obj_ptr, uint24_t class_ptr, uint8_t type);
uint16_t ezom_get_object_size(uint24_t obj_ptr);
bool ezom_is_valid_object(uint24_t obj_ptr);
uint16_t ezom_compute_hash(uint24_t obj_ptr);

// Object creation functions
uint24_t ezom_create_integer(int16_t value);
uint24_t ezom_create_string(const char* data, uint16_t length);
uint24_t ezom_create_symbol(const char* data, uint16_t length);
uint24_t ezom_create_method_dictionary(uint16_t initial_capacity);
uint24_t ezom_object_to_string(uint24_t obj_ptr);

// NEW: Enhanced object creation functions
uint24_t ezom_create_array(uint16_t size);
uint24_t ezom_create_block(uint8_t param_count, uint8_t local_count, uint24_t outer_context);
uint24_t ezom_create_context(uint24_t outer_context, uint8_t local_count);

// Phase 2: Class creation and method installation functions
uint24_t ezom_create_class_with_inheritance(const char* name, uint24_t superclass, uint16_t instance_var_count);
uint24_t ezom_create_instance(uint24_t class_ptr);
void ezom_install_method_in_class(uint24_t class_ptr, const char* selector, uint24_t code, uint8_t arg_count, bool is_primitive);
void ezom_install_methods_from_ast(uint24_t class_ptr, ezom_ast_node_t* method_list, bool is_class_method);
uint24_t ezom_compile_method_from_ast(ezom_ast_node_t* method_ast);

// Bootstrap functions
void ezom_bootstrap_classes(void);
void ezom_bootstrap_enhanced_classes(void);  // NEW
void ezom_install_integer_methods(void);
void ezom_install_string_methods(void);