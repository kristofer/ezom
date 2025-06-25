# EZOM Phase 1 & 2 Implementation Specification
## Minimal VM and Core Language Foundation

**Target Platform:** ez80 Agon Light 2  
**Implementation Language:** C with AgDev toolchain  
**Timeline:** Weeks 1-4  

---

## Phase 1: Minimal VM (Weeks 1-2)

### 1.1 Overview

Phase 1 establishes the foundational object system, basic method dispatch, and core infrastructure needed for a working object-oriented virtual machine.

### 1.2 Core Data Structures

#### 1.2.1 Object Header Layout

```c
// ezom_object.h
#pragma once
#include <stdint.h>

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

// Object types for built-in classes
#define EZOM_TYPE_OBJECT    0x10
#define EZOM_TYPE_INTEGER   0x20
#define EZOM_TYPE_STRING    0x30
#define EZOM_TYPE_ARRAY     0x40
#define EZOM_TYPE_CLASS     0x50
```

#### 1.2.2 Class Structure

```c
// Class object layout
typedef struct ezom_class {
    ezom_object_t header;           // Standard object header
    uint24_t      superclass;       // Pointer to superclass
    uint24_t      method_dict;      // Pointer to method dictionary
    uint24_t      instance_vars;    // Pointer to instance variable names
    uint16_t      instance_size;    // Size of instances in bytes
    uint16_t      instance_var_count; // Number of instance variables
} ezom_class_t;
```

#### 1.2.3 Method Dictionary

```c
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
```

#### 1.2.4 Built-in Object Types

```c
// Integer object
typedef struct ezom_integer {
    ezom_object_t header;
    int16_t       value;        // 16-bit signed integer
} ezom_integer_t;

// String object
typedef struct ezom_string {
    ezom_object_t header;
    uint16_t      length;       // String length
    char          data[];       // Variable length string data
} ezom_string_t;

// Symbol object (interned strings)
typedef struct ezom_symbol {
    ezom_object_t header;
    uint16_t      length;
    uint16_t      hash_cache;   // Cached hash for fast lookup
    char          data[];
} ezom_symbol_t;
```

### 1.3 Memory Management (Basic)

#### 1.3.1 Memory Layout

```c
// ezom_memory.h
#define EZOM_HEAP_START     0x042000    // Start of object heap
#define EZOM_HEAP_SIZE      0x0E000     // 56KB heap space
#define EZOM_HEAP_END       (EZOM_HEAP_START + EZOM_HEAP_SIZE)

// Memory allocator state
typedef struct ezom_heap {
    uint24_t next_free;         // Next free address
    uint24_t heap_end;          // End of allocated space
    uint16_t objects_allocated; // Statistics
    uint16_t bytes_allocated;
} ezom_heap_t;

extern ezom_heap_t g_heap;
```

#### 1.3.2 Basic Allocation

```c
// ezom_memory.c
#include "ezom_memory.h"
#include "ezom_object.h"

// Simple bump allocator for Phase 1
uint24_t ezom_allocate(uint16_t size) {
    // Align to 2-byte boundary
    size = (size + 1) & ~1;
    
    // Check if we have space
    if (g_heap.next_free + size > g_heap.heap_end) {
        // Out of memory - will add GC in Phase 3
        return 0;
    }
    
    uint24_t ptr = g_heap.next_free;
    g_heap.next_free += size;
    g_heap.objects_allocated++;
    g_heap.bytes_allocated += size;
    
    return ptr;
}

// Initialize object header
void ezom_init_object(uint24_t obj_ptr, uint24_t class_ptr, uint8_t type) {
    ezom_object_t* obj = (ezom_object_t*)obj_ptr;
    obj->class_ptr = class_ptr;
    obj->hash = ezom_compute_hash(obj_ptr);
    obj->flags = type;
}
```

### 1.4 Core Primitives

#### 1.4.1 Primitive Function Table

```c
// ezom_primitives.h
typedef uint24_t (*ezom_primitive_fn)(uint24_t receiver, uint24_t* args, uint8_t arg_count);

// Primitive numbers
#define PRIM_OBJECT_CLASS       1
#define PRIM_OBJECT_EQUALS      2
#define PRIM_OBJECT_HASH        3
#define PRIM_OBJECT_PRINTLN     4

#define PRIM_INTEGER_ADD        10
#define PRIM_INTEGER_SUB        11
#define PRIM_INTEGER_MUL        12
#define PRIM_INTEGER_DIV        13
#define PRIM_INTEGER_LT         14
#define PRIM_INTEGER_GT         15
#define PRIM_INTEGER_EQ         16

#define PRIM_STRING_LENGTH      20
#define PRIM_STRING_AT          21
#define PRIM_STRING_CONCAT      22
#define PRIM_STRING_EQUALS      23

// Primitive function table
extern ezom_primitive_fn g_primitives[];
```

#### 1.4.2 Sample Primitive Implementations

```c
// ezom_primitives.c

// Object>>class
uint24_t prim_object_class(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_object_t* obj = (ezom_object_t*)receiver;
    return obj->class_ptr;
}

// Object>>println
uint24_t prim_object_println(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    // Convert object to string and print
    uint24_t str = ezom_object_to_string(receiver);
    ezom_string_t* string_obj = (ezom_string_t*)str;
    
    // Output to Agon VDP
    printf("%.*s\n", string_obj->length, string_obj->data);
    
    return receiver;  // Return self
}

// Integer>>+
uint24_t prim_integer_add(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return 0;  // Error
    
    ezom_integer_t* recv = (ezom_integer_t*)receiver;
    ezom_integer_t* arg = (ezom_integer_t*)args[0];
    
    // Type check
    if ((recv->header.flags & 0xF0) != EZOM_TYPE_INTEGER ||
        (arg->header.flags & 0xF0) != EZOM_TYPE_INTEGER) {
        return 0;  // Type error
    }
    
    return ezom_create_integer(recv->value + arg->value);
}
```

### 1.5 Method Dispatch

#### 1.5.1 Basic Message Send

```c
// ezom_dispatch.h
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
```

```c
// ezom_dispatch.c

// Look up method in class hierarchy
ezom_method_lookup_t ezom_lookup_method(uint24_t class_ptr, uint24_t selector) {
    ezom_method_lookup_t result = {0};
    
    ezom_class_t* current_class = (ezom_class_t*)class_ptr;
    
    while (current_class) {
        ezom_method_dict_t* dict = (ezom_method_dict_t*)current_class->method_dict;
        
        // Linear search in method dictionary (optimize later)
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

// Send message to object
uint24_t ezom_send_message(ezom_message_t* msg) {
    ezom_object_t* receiver = (ezom_object_t*)msg->receiver;
    
    // Look up method
    ezom_method_lookup_t lookup = ezom_lookup_method(receiver->class_ptr, msg->selector);
    
    if (!lookup.method) {
        // Method not found - will add proper error handling later
        printf("Method not found\n");
        return 0;
    }
    
    if (lookup.is_primitive) {
        // Call primitive function
        uint8_t prim_num = (uint8_t)lookup.method->code;
        if (prim_num < MAX_PRIMITIVES && g_primitives[prim_num]) {
            return g_primitives[prim_num](msg->receiver, msg->args, msg->arg_count);
        }
    } else {
        // Will implement bytecode execution in Phase 2
        printf("Bytecode methods not implemented yet\n");
        return 0;
    }
    
    return 0;
}
```

### 1.6 Core Object Creation

#### 1.6.1 Object Factory Functions

```c
// ezom_objects.c

// Create integer object
uint24_t ezom_create_integer(int16_t value) {
    uint24_t ptr = ezom_allocate(sizeof(ezom_integer_t));
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_integer_class, EZOM_TYPE_INTEGER);
    
    ezom_integer_t* obj = (ezom_integer_t*)ptr;
    obj->value = value;
    
    return ptr;
}

// Create string object
uint24_t ezom_create_string(const char* data, uint16_t length) {
    uint24_t ptr = ezom_allocate(sizeof(ezom_string_t) + length + 1);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_string_class, EZOM_TYPE_STRING);
    
    ezom_string_t* obj = (ezom_string_t*)ptr;
    obj->length = length;
    memcpy(obj->data, data, length);
    obj->data[length] = '\0';  // Null terminate for C compatibility
    
    return ptr;
}

// Create symbol (interned string)
uint24_t ezom_create_symbol(const char* data, uint16_t length) {
    // Check symbol table first (will implement interning)
    uint24_t existing = ezom_find_symbol(data, length);
    if (existing) return existing;
    
    // Create new symbol
    uint24_t ptr = ezom_allocate(sizeof(ezom_symbol_t) + length + 1);
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_symbol_class, EZOM_TYPE_OBJECT);
    
    ezom_symbol_t* obj = (ezom_symbol_t*)ptr;
    obj->length = length;
    obj->hash_cache = ezom_hash_string(data, length);
    memcpy(obj->data, data, length);
    obj->data[length] = '\0';
    
    // Add to symbol table
    ezom_intern_symbol(ptr);
    
    return ptr;
}
```

### 1.7 Phase 1 Testing

#### 1.7.1 Test Framework

```c
// test_phase1.c
#include "ezom.h"
#include <stdio.h>
#include <assert.h>

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

// Test object creation
bool test_object_creation() {
    uint24_t int_obj = ezom_create_integer(42);
    if (!int_obj) return false;
    
    ezom_integer_t* obj = (ezom_integer_t*)int_obj;
    return obj->value == 42 && 
           obj->header.class_ptr == g_integer_class &&
           (obj->header.flags & 0xF0) == EZOM_TYPE_INTEGER;
}

// Test method dispatch
bool test_method_dispatch() {
    uint24_t obj1 = ezom_create_integer(5);
    uint24_t obj2 = ezom_create_integer(3);
    uint24_t args[] = {obj2};
    
    ezom_message_t msg = {
        .selector = ezom_create_symbol("+", 1),
        .receiver = obj1,
        .args = args,
        .arg_count = 1
    };
    
    uint24_t result = ezom_send_message(&msg);
    if (!result) return false;
    
    ezom_integer_t* result_obj = (ezom_integer_t*)result;
    return result_obj->value == 8;
}

int main() {
    printf("EZOM Phase 1 Tests\n");
    printf("==================\n");
    
    // Initialize VM
    ezom_init_vm();
    
    // Run tests
    TEST(object_creation);
    TEST(method_dispatch);
    TEST(string_creation);
    TEST(symbol_interning);
    TEST(primitive_calls);
    
    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_total);
    return tests_passed == tests_total ? 0 : 1;
}
```

### 1.8 Phase 1 Deliverables

**Core Files:**
- `ezom_object.h` - Object system definitions
- `ezom_memory.c/h` - Basic memory allocation
- `ezom_primitives.c/h` - Core primitive operations
- `ezom_dispatch.c/h` - Method lookup and dispatch
- `ezom_objects.c/h` - Object creation functions
- `test_phase1.c` - Comprehensive test suite

**Test Criteria:**
- [ ] Create objects of different types
- [ ] Send unary messages (e.g., `42 println`)
- [ ] Send binary messages (e.g., `5 + 3`)
- [ ] Look up methods in class hierarchy
- [ ] Execute primitive operations
- [ ] Handle basic type checking

---

## Phase 2: Core Language (Weeks 3-4)

### 2.1 Overview

Phase 2 adds the complete parser for class definitions, inheritance support, block objects, and basic control flow constructs.

### 2.2 Parser Implementation

#### 2.2.1 Lexer (Tokenizer)

```c
// ezom_lexer.h
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_SYMBOL,
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    TOKEN_PIPE,         // |
    TOKEN_DOT,          // .
    TOKEN_CARET,        // ^
    TOKEN_ASSIGN,       // :=
    TOKEN_COLON,        // :
    TOKEN_SEPARATOR,    // ----
    TOKEN_EQUALS,       // =
    TOKEN_COMMENT       // " ... "
} ezom_token_type_t;

typedef struct ezom_token {
    ezom_token_type_t type;
    uint16_t         line;
    uint16_t         column;
    uint16_t         length;
    char*            text;
} ezom_token_t;

typedef struct ezom_lexer {
    char*    source;
    uint16_t position;
    uint16_t line;
    uint16_t column;
    ezom_token_t current_token;
} ezom_lexer_t;
```

```c
// ezom_lexer.c
void ezom_lexer_init(ezom_lexer_t* lexer, char* source) {
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    
    // Read first token
    ezom_lexer_next_token(lexer);
}

void ezom_lexer_next_token(ezom_lexer_t* lexer) {
    // Skip whitespace
    while (isspace(lexer->source[lexer->position])) {
        if (lexer->source[lexer->position] == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->position++;
    }
    
    char ch = lexer->source[lexer->position];
    lexer->current_token.line = lexer->line;
    lexer->current_token.column = lexer->column;
    
    switch (ch) {
        case '\0':
            lexer->current_token.type = TOKEN_EOF;
            break;
            
        case '(':
            lexer->current_token.type = TOKEN_LPAREN;
            lexer->position++;
            lexer->column++;
            break;
            
        case ')':
            lexer->current_token.type = TOKEN_RPAREN;
            lexer->position++;
            lexer->column++;
            break;
            
        case '\'':
            ezom_lexer_read_string(lexer);
            break;
            
        case '#':
            ezom_lexer_read_symbol(lexer);
            break;
            
        case '"':
            ezom_lexer_read_comment(lexer);
            break;
            
        default:
            if (isalpha(ch) || ch == '_') {
                ezom_lexer_read_identifier(lexer);
            } else if (isdigit(ch) || ch == '-') {
                ezom_lexer_read_integer(lexer);
            } else {
                // Unknown character
                lexer->current_token.type = TOKEN_EOF;
            }
            break;
    }
}
```

#### 2.2.2 Parser (Class Definitions)

```c
// ezom_parser.h
typedef enum {
    AST_CLASS_DEF,
    AST_METHOD_DEF,
    AST_VARIABLE_DEF,
    AST_MESSAGE_SEND,
    AST_BLOCK,
    AST_RETURN,
    AST_ASSIGNMENT,
    AST_LITERAL
} ezom_ast_type_t;

typedef struct ezom_ast_node {
    ezom_ast_type_t type;
    uint16_t        line;
    union {
        struct {
            char* name;
            struct ezom_ast_node* superclass;
            struct ezom_ast_node* instance_vars;
            struct ezom_ast_node* methods;
        } class_def;
        
        struct {
            char* name;
            struct ezom_ast_node* parameters;
            struct ezom_ast_node* locals;
            struct ezom_ast_node* body;
            bool is_class_method;
        } method_def;
        
        struct {
            struct ezom_ast_node* receiver;
            char* selector;
            struct ezom_ast_node* arguments;
        } message_send;
    } data;
} ezom_ast_node_t;
```

```c
// ezom_parser.c

// Parse class definition
// ClassName = SuperClass (
//     | instanceVar1 instanceVar2 |
//     method1 = ( ... )
//     method2: arg = ( ... )
//     ----
//     classMethod = ( ... )
// )
ezom_ast_node_t* ezom_parse_class_definition(ezom_lexer_t* lexer) {
    ezom_ast_node_t* node = ezom_ast_create(AST_CLASS_DEF);
    
    // Parse class name
    if (lexer->current_token.type != TOKEN_IDENTIFIER) {
        ezom_parser_error(lexer, "Expected class name");
        return NULL;
    }
    
    node->data.class_def.name = ezom_copy_token_text(lexer);
    ezom_lexer_next_token(lexer);
    
    // Parse = SuperClass
    if (lexer->current_token.type == TOKEN_EQUALS) {
        ezom_lexer_next_token(lexer);
        
        if (lexer->current_token.type == TOKEN_IDENTIFIER) {
            node->data.class_def.superclass = ezom_parse_identifier(lexer);
        } else {
            ezom_parser_error(lexer, "Expected superclass name");
            return NULL;
        }
    }
    
    // Parse (
    if (lexer->current_token.type != TOKEN_LPAREN) {
        ezom_parser_error(lexer, "Expected '('");
        return NULL;
    }
    ezom_lexer_next_token(lexer);
    
    // Parse instance variables | var1 var2 |
    if (lexer->current_token.type == TOKEN_PIPE) {
        node->data.class_def.instance_vars = ezom_parse_variable_list(lexer);
    }
    
    // Parse methods
    node->data.class_def.methods = ezom_parse_method_list(lexer);
    
    // Parse )
    if (lexer->current_token.type != TOKEN_RPAREN) {
        ezom_parser_error(lexer, "Expected ')'");
        return NULL;
    }
    ezom_lexer_next_token(lexer);
    
    return node;
}

// Parse method definition
ezom_ast_node_t* ezom_parse_method_definition(ezom_lexer_t* lexer) {
    ezom_ast_node_t* node = ezom_ast_create(AST_METHOD_DEF);
    
    // Parse method name/selector
    node->data.method_def.name = ezom_parse_method_selector(lexer);
    
    // Parse = (
    if (lexer->current_token.type != TOKEN_EQUALS) {
        ezom_parser_error(lexer, "Expected '='");
        return NULL;
    }
    ezom_lexer_next_token(lexer);
    
    if (lexer->current_token.type != TOKEN_LPAREN) {
        ezom_parser_error(lexer, "Expected '('");
        return NULL;
    }
    ezom_lexer_next_token(lexer);
    
    // Parse local variables | local1 local2 |
    if (lexer->current_token.type == TOKEN_PIPE) {
        node->data.method_def.locals = ezom_parse_variable_list(lexer);
    }
    
    // Parse method body
    node->data.method_def.body = ezom_parse_statement_list(lexer);
    
    // Parse )
    if (lexer->current_token.type != TOKEN_RPAREN) {
        ezom_parser_error(lexer, "Expected ')'");
        return NULL;
    }
    ezom_lexer_next_token(lexer);
    
    return node;
}
```

### 2.3 Block Objects

#### 2.3.1 Block Structure

```c
// Block object representation
typedef struct ezom_block {
    ezom_object_t header;
    uint24_t      outer_context;    // Lexical environment
    uint24_t      code;             // Bytecode or AST
    uint8_t       param_count;      // Number of parameters
    uint8_t       local_count;      // Number of local variables
} ezom_block_t;

// Block context (activation record)
typedef struct ezom_context {
    ezom_object_t header;
    uint24_t      outer_context;    // Parent context
    uint24_t      method;           // Method being executed
    uint24_t      receiver;         // Message receiver
    uint24_t      locals[];         // Local variables
} ezom_context_t;
```

#### 2.3.2 Block Evaluation

```c
// Create block object
uint24_t ezom_create_block(ezom_ast_node_t* block_ast, uint24_t context) {
    uint24_t ptr = ezom_allocate(sizeof(ezom_block_t));
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_block_class, EZOM_TYPE_OBJECT);
    
    ezom_block_t* block = (ezom_block_t*)ptr;
    block->outer_context = context;
    block->code = (uint24_t)block_ast;  // Store AST for now
    block->param_count = ezom_count_block_parameters(block_ast);
    block->local_count = ezom_count_block_locals(block_ast);
    
    return ptr;
}

// Block>>value: (evaluate block with one argument)
uint24_t prim_block_value(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    ezom_block_t* block = (ezom_block_t*)receiver;
    
    if (arg_count != block->param_count) {
        ezom_error("Wrong number of arguments to block");
        return 0;
    }
    
    // Create new context for block execution
    uint24_t context = ezom_create_context(block->outer_context, 
                                          block->local_count);
    
    // Bind arguments to parameters
    for (uint8_t i = 0; i < arg_count; i++) {
        ezom_context_set_local(context, i, args[i]);
    }
    
    // Evaluate block body
    return ezom_evaluate_ast_in_context((ezom_ast_node_t*)block->code, context);
}
```

### 2.4 Control Flow

#### 2.4.1 Conditional Primitives

```c
// Boolean>>ifTrue:
uint24_t prim_boolean_if_true(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 1) return g_nil;
    
    // Check if receiver is true
    if (receiver == g_true) {
        // Evaluate the block
        uint24_t block = args[0];
        return ezom_send_unary_message(block, g_value_selector);
    }
    
    return g_nil;
}

// Boolean>>ifTrue:ifFalse:
uint24_t prim_boolean_if_true_if_false(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2) return g_nil;
    
    uint24_t true_block = args[0];
    uint24_t false_block = args[1];
    
    if (receiver == g_true) {
        return ezom_send_unary_message(true_block, g_value_selector);
    } else if (receiver == g_false) {
        return ezom_send_unary_message(false_block, g_value_selector);
    }
    
    return g_nil;
}

// Integer>>to:do:
uint24_t prim_integer_to_do(uint24_t receiver, uint24_t* args, uint8_t arg_count) {
    if (arg_count != 2) return g_nil;
    
    ezom_integer_t* start = (ezom_integer_t*)receiver;
    ezom_integer_t* end = (ezom_integer_t*)args[0];
    uint24_t block = args[1];
    
    // Type checks
    if ((start->header.flags & 0xF0) != EZOM_TYPE_INTEGER ||
        (end->header.flags & 0xF0) != EZOM_TYPE_INTEGER) {
        return g_nil;
    }
    
    // Execute loop
    for (int16_t i = start->value; i <= end->value; i++) {
        uint24_t index_obj = ezom_create_integer(i);
        uint24_t block_args[] = {index_obj};
        
        ezom_message_t msg = {
            .selector = g_value_with_selector,
            .receiver = block,
            .args = block_args,
            .arg_count = 1
        };
        
        ezom_send_message(&msg);
    }
    
    return receiver;
}
```

### 2.5 Inheritance Support

#### 2.5.1 Class Creation

```c
// ezom_class.c

// Create a new class
uint24_t ezom_create_class(char* name, uint24_t superclass, 
                          char** instance_vars, uint16_t var_count) {
    uint24_t ptr = ezom_allocate(sizeof(ezom_class_t));
    if (!ptr) return 0;
    
    ezom_init_object(ptr, g_class_class, EZOM_TYPE_CLASS);
    
    ezom_class_t* class_obj = (ezom_class_t*)ptr;
    class_obj->superclass = superclass;
    class_obj->method_dict = ezom_create_method_dictionary();
    class_obj->instance_vars = ezom_create_string_array(instance_vars, var_count);
    class_obj->instance_var_count = var_count;
    
    // Calculate instance size including superclass
    uint16_t super_size = 0;
    if (superclass) {
        ezom_class_t* super = (ezom_class_t*)superclass;
        super_size = super->instance_size;
    }
    
    class_obj->instance_size = super_size + (var_count * sizeof(uint24_t));
    
    return ptr;
}

// Install method in class
void ezom_install_method(uint24_t class_ptr, char* selector, 
                        uint24_t code, uint8_t arg_count, bool is_primitive) {
    ezom_class_t* class_obj = (ezom_class_t*)class_ptr;
    ezom_method_dict_t* dict = (ezom_method_dict_t*)class_obj->method_dict;
    
    // Check if method already exists (override)
    uint24_t selector_symbol = ezom_create_symbol(selector, strlen(selector));
    
    for (uint16_t i = 0; i < dict->size; i++) {
        if (dict->methods[i].selector == selector_symbol) {
            // Override existing method
            dict->methods[i].code = code;
            dict->methods[i].arg_count = arg_count;
            dict->methods[i].flags = is_primitive ? EZOM_METHOD_PRIMITIVE : 0;
            return;
        }
    }
    
    // Add new method
    if (dict->size >= dict->capacity) {
        // Expand method dictionary
        ezom_expand_method_dictionary(class_ptr);
        dict = (ezom_method_dict_t*)class_obj->method_dict; // Refresh pointer
    }
    
    ezom_method_t* method = &dict->methods[dict->size];
    method->selector = selector_symbol;
    method->code = code;
    method->arg_count = arg_count;
    method->flags = is_primitive ? EZOM_METHOD_PRIMITIVE : 0;
    
    dict->size++;
}
```

#### 2.5.2 Super Message Send

```c
// Handle super calls
uint24_t ezom_send_super_message(ezom_message_t* msg, uint24_t calling_class) {
    ezom_class_t* class_obj = (ezom_class_t*)calling_class;
    uint24_t superclass = class_obj->superclass;
    
    if (!superclass) {
        ezom_error("Super call with no superclass");
        return 0;
    }
    
    // Look up method starting from superclass
    ezom_method_lookup_t lookup = ezom_lookup_method(superclass, msg->selector);
    
    if (!lookup.method) {
        ezom_error("Super method not found");
        return 0;
    }
    
    // Execute method with original receiver
    if (lookup.is_primitive) {
        uint8_t prim_num = (uint8_t)lookup.method->code;
        if (prim_num < MAX_PRIMITIVES && g_primitives[prim_num]) {
            return g_primitives[prim_num](msg->receiver, msg->args, msg->arg_count);
        }
    } else {
        // Execute bytecode method (to be implemented)
        return ezom_execute_method(lookup.method, msg->receiver, msg->args, msg->arg_count);
    }
    
    return 0;
}
```

### 2.6 AST Evaluation Engine

#### 2.6.1 Expression Evaluation

```c
// ezom_evaluator.c

// Evaluate AST node in given context
uint24_t ezom_evaluate_ast_in_context(ezom_ast_node_t* node, uint24_t context) {
    if (!node) return g_nil;
    
    switch (node->type) {
        case AST_LITERAL:
            return ezom_evaluate_literal(node);
            
        case AST_MESSAGE_SEND:
            return ezom_evaluate_message_send(node, context);
            
        case AST_ASSIGNMENT:
            return ezom_evaluate_assignment(node, context);
            
        case AST_RETURN:
            return ezom_evaluate_return(node, context);
            
        case AST_BLOCK:
            return ezom_create_block(node, context);
            
        default:
            ezom_error("Unknown AST node type");
            return g_nil;
    }
}

// Evaluate message send
uint24_t ezom_evaluate_message_send(ezom_ast_node_t* node, uint24_t context) {
    // Evaluate receiver
    uint24_t receiver = ezom_evaluate_ast_in_context(node->data.message_send.receiver, context);
    
    // Evaluate arguments
    uint24_t* args = NULL;
    uint8_t arg_count = 0;
    
    if (node->data.message_send.arguments) {
        arg_count = ezom_count_arguments(node->data.message_send.arguments);
        args = ezom_allocate_temp_array(arg_count * sizeof(uint24_t));
        
        ezom_ast_node_t* arg_node = node->data.message_send.arguments;
        for (uint8_t i = 0; i < arg_count; i++) {
            args[i] = ezom_evaluate_ast_in_context(arg_node, context);
            arg_node = arg_node->next; // Assuming linked list of arguments
        }
    }
    
    // Create and send message
    uint24_t selector = ezom_create_symbol(node->data.message_send.selector, 
                                          strlen(node->data.message_send.selector));
    
    ezom_message_t msg = {
        .selector = selector,
        .receiver = receiver,
        .args = args,
        .arg_count = arg_count
    };
    
    uint24_t result = ezom_send_message(&msg);
    
    // Clean up temp array
    if (args) ezom_free_temp_array(args);
    
    return result;
}

// Evaluate assignment
uint24_t ezom_evaluate_assignment(ezom_ast_node_t* node, uint24_t context) {
    uint24_t value = ezom_evaluate_ast_in_context(node->data.assignment.value, context);
    
    // Set variable in context
    ezom_context_set_variable(context, node->data.assignment.variable, value);
    
    return value;
}
```

### 2.7 Built-in Class Definitions

#### 2.7.1 Object Class Bootstrap

```c
// ezom_bootstrap.c

void ezom_bootstrap_object_class() {
    // Create Object class (bootstrap)
    g_object_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_init_object(g_object_class, 0, EZOM_TYPE_CLASS); // Self-referential
    
    ezom_class_t* object_class = (ezom_class_t*)g_object_class;
    object_class->superclass = 0; // No superclass
    object_class->method_dict = ezom_create_method_dictionary();
    object_class->instance_vars = 0; // No instance variables
    object_class->instance_size = sizeof(ezom_object_t);
    object_class->instance_var_count = 0;
    
    // Install Object methods
    ezom_install_method(g_object_class, "class", PRIM_OBJECT_CLASS, 0, true);
    ezom_install_method(g_object_class, "=", PRIM_OBJECT_EQUALS, 1, true);
    ezom_install_method(g_object_class, "hash", PRIM_OBJECT_HASH, 0, true);
    ezom_install_method(g_object_class, "println", PRIM_OBJECT_PRINTLN, 0, true);
}

void ezom_bootstrap_integer_class() {
    g_integer_class = ezom_create_class("Integer", g_object_class, NULL, 0);
    
    // Install Integer methods
    ezom_install_method(g_integer_class, "+", PRIM_INTEGER_ADD, 1, true);
    ezom_install_method(g_integer_class, "-", PRIM_INTEGER_SUB, 1, true);
    ezom_install_method(g_integer_class, "*", PRIM_INTEGER_MUL, 1, true);
    ezom_install_method(g_integer_class, "/", PRIM_INTEGER_DIV, 1, true);
    ezom_install_method(g_integer_class, "<", PRIM_INTEGER_LT, 1, true);
    ezom_install_method(g_integer_class, ">", PRIM_INTEGER_GT, 1, true);
    ezom_install_method(g_integer_class, "=", PRIM_INTEGER_EQ, 1, true);
    ezom_install_method(g_integer_class, "to:do:", PRIM_INTEGER_TO_DO, 2, true);
}

void ezom_bootstrap_string_class() {
    g_string_class = ezom_create_class("String", g_object_class, NULL, 0);
    
    // Install String methods
    ezom_install_method(g_string_class, "length", PRIM_STRING_LENGTH, 0, true);
    ezom_install_method(g_string_class, "at:", PRIM_STRING_AT, 1, true);
    ezom_install_method(g_string_class, "+", PRIM_STRING_CONCAT, 1, true);
    ezom_install_method(g_string_class, "=", PRIM_STRING_EQUALS, 1, true);
}

void ezom_bootstrap_boolean_classes() {
    g_boolean_class = ezom_create_class("Boolean", g_object_class, NULL, 0);
    g_true_class = ezom_create_class("True", g_boolean_class, NULL, 0);
    g_false_class = ezom_create_class("False", g_boolean_class, NULL, 0);
    
    // Create singleton instances
    g_true = ezom_allocate(sizeof(ezom_object_t));
    ezom_init_object(g_true, g_true_class, EZOM_TYPE_OBJECT);
    
    g_false = ezom_allocate(sizeof(ezom_object_t));
    ezom_init_object(g_false, g_false_class, EZOM_TYPE_OBJECT);
    
    // Install Boolean methods
    ezom_install_method(g_true_class, "ifTrue:", PRIM_BOOLEAN_IF_TRUE, 1, true);
    ezom_install_method(g_true_class, "ifFalse:", PRIM_BOOLEAN_IF_FALSE, 1, true);
    ezom_install_method(g_true_class, "ifTrue:ifFalse:", PRIM_BOOLEAN_IF_TRUE_IF_FALSE, 2, true);
    
    ezom_install_method(g_false_class, "ifTrue:", PRIM_BOOLEAN_IF_TRUE, 1, true);
    ezom_install_method(g_false_class, "ifFalse:", PRIM_BOOLEAN_IF_FALSE, 1, true);
    ezom_install_method(g_false_class, "ifTrue:ifFalse:", PRIM_BOOLEAN_IF_TRUE_IF_FALSE, 2, true);
}
```

### 2.8 Phase 2 Testing

#### 2.8.1 Class Definition Tests

```c
// test_phase2.c

bool test_class_definition() {
    char* source = 
        "TestClass = Object (\n"
        "    | instanceVar |\n"
        "    \n"
        "    getValue = (\n"
        "        ^instanceVar\n"
        "    )\n"
        "    \n"
        "    setValue: newValue = (\n"
        "        instanceVar := newValue.\n"
        "        ^self\n"
        "    )\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, source);
    
    ezom_ast_node_t* class_ast = ezom_parse_class_definition(&lexer);
    if (!class_ast) return false;
    
    // Verify parsed structure
    return strcmp(class_ast->data.class_def.name, "TestClass") == 0 &&
           class_ast->data.class_def.methods != NULL;
}

bool test_inheritance() {
    // Create test classes
    uint24_t parent_class = ezom_create_class("Parent", g_object_class, NULL, 0);
    ezom_install_method(parent_class, "parentMethod", PRIM_OBJECT_PRINTLN, 0, true);
    
    uint24_t child_class = ezom_create_class("Child", parent_class, NULL, 0);
    ezom_install_method(child_class, "childMethod", PRIM_OBJECT_PRINTLN, 0, true);
    
    // Create instance of child
    uint24_t child_instance = ezom_create_instance(child_class);
    
    // Test method lookup in hierarchy
    uint24_t parent_selector = ezom_create_symbol("parentMethod", 12);
    ezom_method_lookup_t lookup = ezom_lookup_method(child_class, parent_selector);
    
    return lookup.method != NULL && lookup.class_ptr == parent_class;
}

bool test_block_evaluation() {
    // Create a simple block [ 42 ]
    ezom_ast_node_t* block_ast = ezom_create_block_ast();
    ezom_ast_add_literal(block_ast, ezom_create_integer(42));
    
    uint24_t block = ezom_create_block(block_ast, 0);
    
    // Evaluate block
    uint24_t result = ezom_send_unary_message(block, g_value_selector);
    
    if (!result) return false;
    
    ezom_integer_t* int_result = (ezom_integer_t*)result;
    return int_result->value == 42;
}

bool test_control_flow() {
    // Test: true ifTrue: [ 'yes' println ]
    uint24_t yes_string = ezom_create_string("yes", 3);
    
    // Create block that prints 'yes'
    ezom_ast_node_t* block_ast = ezom_create_block_ast();
    ezom_ast_add_message_send(block_ast, yes_string, "println", NULL, 0);
    uint24_t print_block = ezom_create_block(block_ast, 0);
    
    // Send ifTrue: message to true
    uint24_t args[] = {print_block};
    ezom_message_t msg = {
        .selector = ezom_create_symbol("ifTrue:", 7),
        .receiver = g_true,
        .args = args,
        .arg_count = 1
    };
    
    uint24_t result = ezom_send_message(&msg);
    
    // Should execute without error
    return result != 0;
}

bool test_method_compilation() {
    char* method_source = 
        "add: a to: b = (\n"
        "    | result |\n"
        "    result := a + b.\n"
        "    ^result\n"
        ")\n";
    
    ezom_lexer_t lexer;
    ezom_lexer_init(&lexer, method_source);
    
    ezom_ast_node_t* method_ast = ezom_parse_method_definition(&lexer);
    if (!method_ast) return false;
    
    // Install method in test class
    uint24_t test_class = ezom_create_class("TestClass", g_object_class, NULL, 0);
    ezom_install_compiled_method(test_class, method_ast);
    
    // Create instance and test method
    uint24_t instance = ezom_create_instance(test_class);
    uint24_t args[] = {ezom_create_integer(5), ezom_create_integer(3)};
    
    ezom_message_t msg = {
        .selector = ezom_create_symbol("add:to:", 6),
        .receiver = instance,
        .args = args,
        .arg_count = 2
    };
    
    uint24_t result = ezom_send_message(&msg);
    if (!result) return false;
    
    ezom_integer_t* int_result = (ezom_integer_t*)result;
    return int_result->value == 8;
}

int main() {
    printf("EZOM Phase 2 Tests\n");
    printf("==================\n");
    
    // Initialize VM
    ezom_init_vm();
    
    // Run tests
    TEST(class_definition);
    TEST(inheritance);
    TEST(block_evaluation);
    TEST(control_flow);
    TEST(method_compilation);
    TEST(parser_integration);
    TEST(ast_evaluation);
    
    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_total);
    return tests_passed == tests_total ? 0 : 1;
}
```

### 2.9 Build System

#### 2.9.1 Enhanced Makefile

```makefile
# Makefile for EZOM Phases 1-2
CC = ez80-clang
CFLAGS = -Wall -Oz -mllvm -profile-guided-section-prefix=false
INCLUDES = -Isrc/include
TARGET = ezom.bin
TEST_TARGET = test_ezom.bin

# Source files
VM_SOURCES = src/vm/main.c src/vm/object.c src/vm/memory.c src/vm/primitives.c \
             src/vm/dispatch.c src/vm/class.c src/vm/bootstrap.c

COMPILER_SOURCES = src/compiler/lexer.c src/compiler/parser.c src/compiler/ast.c \
                   src/compiler/evaluator.c

TEST_SOURCES = tests/test_phase1.c tests/test_phase2.c tests/test_runner.c

ALL_SOURCES = $(VM_SOURCES) $(COMPILER_SOURCES)
ALL_OBJECTS = $(ALL_SOURCES:.c=.o)

# Targets
all: $(TARGET)

$(TARGET): $(ALL_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_TARGET)
	fab-agon-emulator $<

$(TEST_TARGET): $(ALL_OBJECTS) $(TEST_SOURCES:.c=.o)
	$(CC) $(CFLAGS) -DEZOM_TESTING -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(ALL_OBJECTS) $(TEST_SOURCES:.c=.o) $(TARGET) $(TEST_TARGET)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# File dependencies
src/vm/object.o: src/include/ezom_object.h src/include/ezom_memory.h
src/vm/memory.o: src/include/ezom_memory.h src/include/ezom_object.h
src/vm/primitives.o: src/include/ezom_primitives.h src/include/ezom_object.h
src/vm/dispatch.o: src/include/ezom_dispatch.h src/include/ezom_object.h
src/compiler/lexer.o: src/include/ezom_lexer.h
src/compiler/parser.o: src/include/ezom_parser.h src/include/ezom_lexer.h

.PHONY: all test clean debug
```

### 2.10 Phase 2 Deliverables

**Additional Files:**
- `ezom_lexer.c/h` - Complete lexical analyzer
- `ezom_parser.c/h` - Class and method parser
- `ezom_ast.c/h` - Abstract syntax tree implementation
- `ezom_evaluator.c/h` - AST evaluation engine
- `ezom_class.c/h` - Class creation and inheritance
- `ezom_bootstrap.c/h` - Built-in class definitions
- `test_phase2.c` - Phase 2 test suite

**Test Criteria:**
- [ ] Parse complete class definitions from source
- [ ] Create classes with inheritance hierarchy
- [ ] Define and call methods with arguments
- [ ] Create and evaluate block objects
- [ ] Execute control flow constructs (ifTrue:, loops)
- [ ] Handle method override and super calls
- [ ] Support instance variables and assignment

### 2.11 Integration Points

**Phase 1 → Phase 2:**
- Object system provides foundation for class definitions
- Method dispatch extended to handle compiled methods
- Memory allocation used for AST nodes and contexts

**Phase 2 → Phase 3:**
- AST evaluation creates objects that need garbage collection
- Block contexts require proper cleanup
- Method compilation generates objects to be managed

**Success Metrics:**
- Complete Hello World program execution
- Class definition, instantiation, and method calls
- Block evaluation with proper lexical scoping
- Control flow constructs working correctly
- Memory usage within 64KB for basic programs

This completes the specification for Phases 1 and 2, providing a solid foundation for implementing the core EZOM virtual machine and language features.```
