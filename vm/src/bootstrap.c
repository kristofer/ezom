// ============================================================================
// File: src/vm/bootstrap.c
// Enhanced class bootstrapping with Boolean hierarchy
// ============================================================================

#include "../include/ezom_object.h"
#include "../include/ezom_memory.h"
#include "../include/ezom_primitives.h"
#include <stdio.h>
#include <string.h>

// Forward declare logging function
extern void ezom_log(const char* format, ...);

// Bootstrap basic classes (minimal implementation for Phase 1)
void ezom_bootstrap_classes(void) {
    printf("Bootstrapping basic classes...\n");
    ezom_log("Bootstrapping basic classes...\n");
    
    // Create Object class (bootstrap - self-referential)
    ezom_log("About to allocate Object class\n");
    g_object_class = ezom_allocate(sizeof(ezom_class_t));
    ezom_log("Object class allocated at 0x%06lX\n", (unsigned long)g_object_class);
    if (g_object_class) {
        ezom_log("About to init Object class\n");
        ezom_init_object(g_object_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_log("Object class initialized\n");
        
        ezom_class_t* object_class = EZOM_OBJECT_PTR(g_object_class);
        object_class->superclass = 0; // No superclass
        ezom_log("About to create Object class method dictionary\n");
        object_class->method_dict = ezom_create_method_dictionary(8);
        ezom_log("Object class method dictionary created at 0x%06lX\n", (unsigned long)object_class->method_dict);
        object_class->instance_vars = 0;
        object_class->instance_size = sizeof(ezom_object_t);
        object_class->instance_var_count = 0;
        
        printf("   Object class created at 0x%06X\n", g_object_class);
    }
    
    // Create Symbol class FIRST (needed for method installation)
    g_symbol_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_symbol_class) {
        ezom_init_object(g_symbol_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* symbol_class = EZOM_OBJECT_PTR(g_symbol_class);
        symbol_class->superclass = g_object_class;
        symbol_class->method_dict = ezom_create_method_dictionary(4);
        symbol_class->instance_vars = 0;
        symbol_class->instance_size = sizeof(ezom_symbol_t);
        symbol_class->instance_var_count = 0;
        
        printf("   Symbol class created at 0x%06X\n", g_symbol_class);
    }

    // Create Integer class
    g_integer_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_integer_class) {
        ezom_init_object(g_integer_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* integer_class = EZOM_OBJECT_PTR(g_integer_class);
        integer_class->superclass = g_object_class;
        integer_class->method_dict = ezom_create_method_dictionary(16);
        integer_class->instance_vars = 0;
        integer_class->instance_size = sizeof(ezom_integer_t);
        integer_class->instance_var_count = 0;
        
        // Methods will be installed in enhanced bootstrap
        
        printf("   Integer class created at 0x%06X\n", g_integer_class);
    }
    
    // Create String class
    g_string_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_string_class) {
        ezom_init_object(g_string_class, g_object_class, EZOM_TYPE_CLASS);
        
        ezom_class_t* string_class = EZOM_OBJECT_PTR(g_string_class);
        string_class->superclass = g_object_class;
        string_class->method_dict = ezom_create_method_dictionary(8);
        string_class->instance_vars = 0;
        string_class->instance_size = sizeof(ezom_string_t); // Variable size
        string_class->instance_var_count = 0;
        
        // Methods will be installed in enhanced bootstrap
        
        printf("   String class created at 0x%06X\n", g_string_class);
    }
    
    printf("Bootstrap complete!\n");
}

// Global class pointers (basic classes)
uint24_t g_object_class = 0;
uint24_t g_class_class = 0;
uint24_t g_integer_class = 0;
uint24_t g_string_class = 0;
uint24_t g_symbol_class = 0;

// Enhanced global class pointers
uint24_t g_array_class = 0;
uint24_t g_block_class = 0;
uint24_t g_boolean_class = 0;
uint24_t g_true_class = 0;
uint24_t g_false_class = 0;
uint24_t g_nil_class = 0;
uint24_t g_context_class = 0;

// Global singleton objects
uint24_t g_nil = 0;
uint24_t g_true = 0;
uint24_t g_false = 0;

// Forward declarations
void ezom_install_object_methods(void);
void ezom_install_integer_methods(void);
void ezom_install_string_methods(void);
void ezom_install_array_methods(void);
void ezom_install_boolean_methods(void);
void ezom_install_block_methods(void);

// Internal bootstrap phases
static void ezom_bootstrap_phase1_fundamentals(void);
static void ezom_bootstrap_phase2_hierarchy(void);
static void ezom_bootstrap_phase3_methods(void);

void ezom_bootstrap_enhanced_classes(void) {
    printf("Bootstrapping enhanced SOM-compatible classes...\n");
    
    // PHASE 1: Bootstrap fundamental objects with minimal dependencies
    ezom_bootstrap_phase1_fundamentals();
    
    // PHASE 2: Complete class hierarchy with proper relationships
    ezom_bootstrap_phase2_hierarchy();
    
    printf("Two-phase bootstrap complete! SOM-compatible class hierarchy ready.\n");
}

// PHASE 1: Create minimal objects to break circular dependencies
static void ezom_bootstrap_phase1_fundamentals(void) {
    printf("Phase 1: Creating fundamental objects...\n");
    
    // Step 1: Create nil as a minimal object with NO class initially
    g_nil = ezom_allocate(sizeof(ezom_object_t));
    if (g_nil) {
        ezom_object_t* nil_obj = EZOM_OBJECT_PTR(g_nil);
        nil_obj->class_ptr = 0; // Bootstrap: temporarily no class
        nil_obj->hash = 0;
        nil_obj->flags = EZOM_TYPE_NIL;
        printf("   Nil created (no class yet)\n");
    }
    
    // Step 2: Create Object class as self-referential root
    g_object_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_object_class) {
        ezom_object_t* obj_header = EZOM_OBJECT_PTR(g_object_class);
        obj_header->class_ptr = g_object_class; // Self-referential!
        obj_header->hash = ezom_compute_hash(g_object_class);
        obj_header->flags = EZOM_TYPE_CLASS;
        
        ezom_class_t* object_class = EZOM_OBJECT_PTR(g_object_class);
        object_class->superclass = 0; // Object is the root class, no superclass
        object_class->method_dict = 0; // Bootstrap: no methods yet
        object_class->instance_vars = 0;
        object_class->instance_size = sizeof(ezom_object_t);
        object_class->instance_var_count = 0;
        printf("   Object class created (self-referential)\n");
    }
    
    // Step 3: Fix nil's class pointer now that we have Object class
    if (g_nil && g_object_class) {
        ezom_object_t* nil_obj = EZOM_OBJECT_PTR(g_nil);
        nil_obj->class_ptr = g_object_class; // Nil is instance of Object
        printf("   Nil class pointer fixed\n");
    }
    
    // Step 4: Create true/false as minimal objects
    g_true = ezom_allocate(sizeof(ezom_object_t));
    if (g_true) {
        ezom_object_t* true_obj = (ezom_object_t*)EZOM_OBJECT_PTR(g_true);
        true_obj->class_ptr = g_object_class; // Temporary class
        true_obj->hash = 1;
        true_obj->flags = EZOM_TYPE_BOOLEAN;
        printf("   True created\n");
    }
    
    g_false = ezom_allocate(sizeof(ezom_object_t));
    if (g_false) {
        ezom_object_t* false_obj = (ezom_object_t*)EZOM_OBJECT_PTR(g_false);
        false_obj->class_ptr = g_object_class; // Temporary class
        false_obj->hash = 0;
        false_obj->flags = EZOM_TYPE_BOOLEAN;
        printf("   False created\n");
    }
}

// PHASE 2: Complete the class hierarchy with proper inheritance
static void ezom_bootstrap_phase2_hierarchy(void) {
    printf("Phase 2: Completing class hierarchy...\n");
    
    // Create Symbol class first (needed for method dictionaries)
    g_symbol_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_symbol_class) {
        ezom_init_object(g_symbol_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* symbol_class = EZOM_OBJECT_PTR(g_symbol_class);
        symbol_class->superclass = g_object_class;
        symbol_class->method_dict = 0; // Bootstrap: defer method dictionary
        symbol_class->instance_vars = 0;
        symbol_class->instance_size = sizeof(ezom_symbol_t);
        symbol_class->instance_var_count = 0;
        printf("   Symbol class created\n");
    }
    
    // Create other essential classes
    g_integer_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_integer_class) {
        ezom_init_object(g_integer_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* integer_class = EZOM_OBJECT_PTR(g_integer_class);
        integer_class->superclass = g_object_class;
        integer_class->method_dict = 0; // Bootstrap: defer method dictionary
        integer_class->instance_vars = 0;
        integer_class->instance_size = sizeof(ezom_integer_t);
        integer_class->instance_var_count = 0;
        printf("   Integer class created\n");
    }
    
    g_string_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_string_class) {
        ezom_init_object(g_string_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* string_class = EZOM_OBJECT_PTR(g_string_class);
        string_class->superclass = g_object_class;
        string_class->method_dict = 0; // Bootstrap: defer method dictionary
        string_class->instance_vars = 0;
        string_class->instance_size = sizeof(ezom_string_t);
        string_class->instance_var_count = 0;
        printf("   String class created\n");
    }
    
    // Create Boolean hierarchy
    g_boolean_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_boolean_class) {
        ezom_init_object(g_boolean_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* boolean_class = EZOM_OBJECT_PTR(g_boolean_class);
        boolean_class->superclass = g_object_class;
        boolean_class->method_dict = 0;
        boolean_class->instance_vars = 0;
        boolean_class->instance_size = sizeof(ezom_object_t);
        boolean_class->instance_var_count = 0;
        printf("   Boolean class created\n");
    }
    
    g_true_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_true_class) {
        ezom_init_object(g_true_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* true_class = EZOM_OBJECT_PTR(g_true_class);
        true_class->superclass = g_boolean_class;
        true_class->method_dict = 0;
        true_class->instance_vars = 0;
        true_class->instance_size = sizeof(ezom_object_t);
        true_class->instance_var_count = 0;
        printf("   True class created\n");
    }
    
    g_false_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_false_class) {
        ezom_init_object(g_false_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* false_class = EZOM_OBJECT_PTR(g_false_class);
        false_class->superclass = g_boolean_class;
        false_class->method_dict = 0;
        false_class->instance_vars = 0;
        false_class->instance_size = sizeof(ezom_object_t);
        false_class->instance_var_count = 0;
        printf("   False class created\n");
    }
    
    // Fix true/false objects to have proper classes
    if (g_true && g_true_class) {
        ezom_object_t* true_obj = (ezom_object_t*)EZOM_OBJECT_PTR(g_true);
        true_obj->class_ptr = g_true_class;
        printf("   True object class fixed\n");
    }
    
    if (g_false && g_false_class) {
        ezom_object_t* false_obj = (ezom_object_t*)EZOM_OBJECT_PTR(g_false);
        false_obj->class_ptr = g_false_class;
        printf("   False object class fixed\n");
    }
    
    // Create remaining classes
    g_array_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_array_class) {
        ezom_init_object(g_array_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* array_class = EZOM_OBJECT_PTR(g_array_class);
        array_class->superclass = g_object_class;
        array_class->method_dict = 0;
        array_class->instance_vars = 0;
        array_class->instance_size = sizeof(ezom_array_t);
        array_class->instance_var_count = 0;
        printf("   Array class created\n");
    }
    
    g_block_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_block_class) {
        ezom_init_object(g_block_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* block_class = EZOM_OBJECT_PTR(g_block_class);
        block_class->superclass = g_object_class;
        block_class->method_dict = 0;
        block_class->instance_vars = 0;
        block_class->instance_size = sizeof(ezom_block_t);
        block_class->instance_var_count = 0;
        printf("   Block class created\n");
    }
    
    g_context_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_context_class) {
        ezom_init_object(g_context_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* context_class = EZOM_OBJECT_PTR(g_context_class);
        context_class->superclass = g_object_class;
        context_class->method_dict = 0;
        context_class->instance_vars = 0;
        context_class->instance_size = sizeof(ezom_context_t);
        context_class->instance_var_count = 0;
        printf("   Context class created\n");
    }
    
    // Create Nil class and fix nil object
    g_nil_class = ezom_allocate(sizeof(ezom_class_t));
    if (g_nil_class) {
        ezom_init_object(g_nil_class, g_object_class, EZOM_TYPE_CLASS);
        ezom_class_t* nil_class = EZOM_OBJECT_PTR(g_nil_class);
        nil_class->superclass = g_object_class;
        nil_class->method_dict = 0;
        nil_class->instance_vars = 0;
        nil_class->instance_size = sizeof(ezom_object_t);
        nil_class->instance_var_count = 0;
        
        // Fix nil object to have proper class
        if (g_nil) {
            ezom_object_t* nil_obj = EZOM_OBJECT_PTR(g_nil);
            nil_obj->class_ptr = g_nil_class;
            printf("   Nil class created and nil object fixed\n");
        }
    }
    
    // PHASE 3: Now create method dictionaries safely
    ezom_bootstrap_phase3_methods();
}

// PHASE 3: Create method dictionaries and install methods
static void ezom_bootstrap_phase3_methods(void) {
    printf("Phase 3: Creating method dictionaries...\n");
    ezom_log("Phase 3: Creating method dictionaries...\n");
    
    printf("   g_object_class = 0x%06lX\n", (unsigned long)g_object_class);
    printf("   g_symbol_class = 0x%06lX\n", (unsigned long)g_symbol_class);
    ezom_log("   g_object_class = 0x%06lX\n", (unsigned long)g_object_class);
    ezom_log("   g_symbol_class = 0x%06lX\n", (unsigned long)g_symbol_class);
    
    // Now we can safely create method dictionaries since Symbol class exists
    if (g_object_class && g_symbol_class) {
        printf("   Condition passed - creating method dictionaries...\n");
        ezom_log("   Condition passed - creating method dictionaries...\n");
        ezom_class_t* object_class = EZOM_OBJECT_PTR(g_object_class);
        object_class->method_dict = ezom_create_method_dictionary(8);
        printf("   Object class method dictionary created at 0x%06lX\n", (unsigned long)object_class->method_dict);
        ezom_log("   Object class method dictionary created at 0x%06lX\n", (unsigned long)object_class->method_dict);
    } else {
        printf("   ERROR: Cannot create method dictionaries - missing classes!\n");
        ezom_log("   ERROR: Cannot create method dictionaries - missing classes!\n");
        return;
    }
    
    // Create Symbol class method dictionary
    if (g_symbol_class) {
        ezom_class_t* symbol_class = EZOM_OBJECT_PTR(g_symbol_class);
        symbol_class->method_dict = ezom_create_method_dictionary(4);
        printf("   Symbol class method dictionary created at 0x%06lX\n", (unsigned long)symbol_class->method_dict);
        ezom_log("   Symbol class method dictionary created at 0x%06lX\n", (unsigned long)symbol_class->method_dict);
    }
    
    if (g_integer_class && g_symbol_class) {
        ezom_class_t* integer_class = EZOM_OBJECT_PTR(g_integer_class);
        integer_class->method_dict = ezom_create_method_dictionary(16);
        printf("   Integer class method dictionary created\n");
    }
    
    if (g_string_class && g_symbol_class) {
        ezom_class_t* string_class = EZOM_OBJECT_PTR(g_string_class);
        string_class->method_dict = ezom_create_method_dictionary(8);
        printf("   String class method dictionary created\n");
    }
    
    if (g_boolean_class && g_symbol_class) {
        ezom_class_t* boolean_class = EZOM_OBJECT_PTR(g_boolean_class);
        boolean_class->method_dict = ezom_create_method_dictionary(8);
        printf("   Boolean class method dictionary created\n");
    }
    
    if (g_true_class && g_symbol_class) {
        ezom_class_t* true_class = EZOM_OBJECT_PTR(g_true_class);
        true_class->method_dict = ezom_create_method_dictionary(8);
        printf("   True class method dictionary created\n");
    }
    
    if (g_false_class && g_symbol_class) {
        ezom_class_t* false_class = EZOM_OBJECT_PTR(g_false_class);
        false_class->method_dict = ezom_create_method_dictionary(8);
        printf("   False class method dictionary created\n");
    }
    
    if (g_array_class && g_symbol_class) {
        ezom_class_t* array_class = EZOM_OBJECT_PTR(g_array_class);
        array_class->method_dict = ezom_create_method_dictionary(16);
        printf("   Array class method dictionary created\n");
    }
    
    if (g_block_class && g_symbol_class) {
        ezom_class_t* block_class = EZOM_OBJECT_PTR(g_block_class);
        block_class->method_dict = ezom_create_method_dictionary(8);
        printf("   Block class method dictionary created\n");
    }
    
    if (g_context_class && g_symbol_class) {
        ezom_class_t* context_class = EZOM_OBJECT_PTR(g_context_class);
        context_class->method_dict = ezom_create_method_dictionary(4);
        printf("   Context class method dictionary created\n");
    }
    
    if (g_nil_class && g_symbol_class) {
        ezom_class_t* nil_class = EZOM_OBJECT_PTR(g_nil_class);
        nil_class->method_dict = ezom_create_method_dictionary(4);
        printf("   Nil class method dictionary created\n");
    }
    
    // Install methods in all classes
    printf("   Installing methods in all classes...\n");
    ezom_log("   Installing methods in all classes...\n");
    
    printf("   About to install Object methods...\n");
    ezom_log("   About to install Object methods...\n");
    ezom_install_object_methods();
    
    printf("   About to install Integer methods...\n");
    ezom_log("   About to install Integer methods...\n");
    ezom_install_integer_methods();
    printf("   COMPLETED Integer methods installation\n");
    ezom_log("   COMPLETED Integer methods installation\n");
    
    printf("   About to install String methods...\n");
    ezom_log("   About to install String methods...\n");
    ezom_install_string_methods();
    printf("   COMPLETED String methods installation\n");
    ezom_log("   COMPLETED String methods installation\n");
    
    printf("   About to install Array methods...\n");
    ezom_log("   About to install Array methods...\n");
    ezom_install_array_methods();
    
    printf("   About to install Boolean methods...\n");
    ezom_log("   About to install Boolean methods...\n");
    ezom_install_boolean_methods();
    
    printf("   About to install Block methods...\n");
    ezom_log("   About to install Block methods...\n");
    ezom_install_block_methods();
    
    printf("Enhanced bootstrap complete! SOM-compatible class hierarchy ready.\n");
}

// Helper function for adding methods to method dictionary
static void add_method_to_dict(ezom_method_dict_t* dict, const char* selector, uint8_t prim_num, uint8_t arg_count) {
    printf("     Adding method '%s' (prim %d)...", selector, prim_num);
    ezom_log("     Adding method '%s' (prim %d)...\n", selector, prim_num);
    
    if (!dict) {
        printf(" FAILED - dict is NULL!\n");
        ezom_log(" FAILED - dict is NULL!\n");
        return;
    }
    
    if (!selector) {
        printf(" FAILED - selector is NULL!\n");
        ezom_log(" FAILED - selector is NULL!\n");
        return;
    }
    
    printf(" [dict=0x%06lX, size=%d, capacity=%d]", (unsigned long)dict, dict->size, dict->capacity);
    ezom_log(" [dict=0x%06lX, size=%d, capacity=%d]\n", (unsigned long)dict, dict->size, dict->capacity);
    
    // Add memory corruption check
    if (dict->size >= dict->capacity) {
        printf(" ERROR: Dictionary size >= capacity!\n");
        ezom_log(" ERROR: Dictionary size >= capacity!\n");
        return;
    }
    
    // Skip pointer range check since dict is already converted from ezom pointer
    
    if (dict->size < dict->capacity) {
        printf(" Creating method at index %d...", dict->size);
        ezom_log(" Creating method at index %d...", dict->size);
        
        ezom_method_t* method = &dict->methods[dict->size];
        printf(" method ptr=0x%06lX", (unsigned long)method);
        ezom_log(" method ptr=0x%06lX", (unsigned long)method);
        
        printf(" Creating symbol...");
        ezom_log(" Creating symbol...");
        
        // Add safety check for selector string
        if (!selector) {
            printf(" ERROR: selector string is NULL!\n");
            ezom_log(" ERROR: selector string is NULL!\n");
            return;
        }
        
        size_t selector_len = strlen(selector);
        printf(" selector='%s' len=%d...", selector, (int)selector_len);
        ezom_log(" selector='%s' len=%d...", selector, (int)selector_len);
        
        method->selector = ezom_create_symbol(selector, selector_len);
        printf(" symbol=0x%06lX", (unsigned long)method->selector);
        ezom_log(" symbol=0x%06lX", (unsigned long)method->selector);
        
        if (!method->selector) {
            printf(" FAILED - symbol creation failed\n");
            ezom_log(" FAILED - symbol creation failed\n");
            return;
        }
        
        printf(" Setting method fields...");
        ezom_log(" Setting method fields...");
        method->code = prim_num;
        method->arg_count = arg_count;
        method->flags = EZOM_METHOD_PRIMITIVE;
        
        printf(" Incrementing dict size...");
        ezom_log(" Incrementing dict size...");
        dict->size++;
        
        printf(" SUCCESS (dict size now %d)\n", dict->size);
        ezom_log(" SUCCESS (dict size now %d)\n", dict->size);
    } else {
        printf(" FAILED - dictionary full (%d/%d)\n", dict->size, dict->capacity);
    }
}

// Enhanced method installation functions
void ezom_install_object_methods(void) {
    printf("   Installing Object methods...\n");
    ezom_log("   Installing Object methods...\n");
    
    printf("   g_object_class = 0x%06lX\n", (unsigned long)g_object_class);
    ezom_log("   g_object_class = 0x%06lX\n", (unsigned long)g_object_class);
    
    if (!g_object_class) {
        printf("   ERROR: g_object_class is NULL!\n");
        ezom_log("   ERROR: g_object_class is NULL!\n");
        return;
    }
    
    ezom_class_t* object_class = EZOM_OBJECT_PTR(g_object_class);
    printf("   object_class->method_dict = 0x%06lX\n", (unsigned long)object_class->method_dict);
    ezom_log("   object_class->method_dict = 0x%06lX\n", (unsigned long)object_class->method_dict);
    
    if (!object_class->method_dict) {
        printf("   ERROR: method_dict is NULL!\n");
        ezom_log("   ERROR: method_dict is NULL!\n");
        return;
    }
    
    ezom_method_dict_t* dict = EZOM_OBJECT_PTR(object_class->method_dict);
    printf("   dict capacity: %d, size: %d\n", dict->capacity, dict->size);
    ezom_log("   dict capacity: %d, size: %d\n", dict->capacity, dict->size);
    
    // Install Object methods (enhanced)
    add_method_to_dict(dict, "class", PRIM_OBJECT_CLASS, 0);
    add_method_to_dict(dict, "=", PRIM_OBJECT_EQUALS, 1);
    add_method_to_dict(dict, "hash", PRIM_OBJECT_HASH, 0);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    add_method_to_dict(dict, "isNil", PRIM_OBJECT_IS_NIL, 0);
    add_method_to_dict(dict, "notNil", PRIM_OBJECT_NOT_NIL, 0);
    
    printf("      Installed %d methods in Object\n", dict->size);
}

void ezom_install_integer_methods(void) {
    printf("   Installing Integer methods...\n");
    ezom_class_t* integer_class = EZOM_OBJECT_PTR(g_integer_class);
    if (!integer_class) {
        printf("   ERROR: g_integer_class is NULL!\n");
        return;
    }
    ezom_method_dict_t* dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(integer_class->method_dict);
    if (!dict) {
        printf("   ERROR: Integer class method_dict is NULL!\n");
        return;
    }
    printf("   Integer class method_dict has capacity %d, current size %d\n", dict->capacity, dict->size);
    
    // Arithmetic operations
    add_method_to_dict(dict, "+", PRIM_INTEGER_ADD, 1);
    add_method_to_dict(dict, "-", PRIM_INTEGER_SUB, 1);
    add_method_to_dict(dict, "*", PRIM_INTEGER_MUL, 1);
    add_method_to_dict(dict, "/", PRIM_INTEGER_DIV, 1);
    add_method_to_dict(dict, "\\", PRIM_INTEGER_MOD, 1);
    add_method_to_dict(dict, "abs", PRIM_INTEGER_ABS, 0);
    
    // Comparison operations
    add_method_to_dict(dict, "<", PRIM_INTEGER_LT, 1);
    add_method_to_dict(dict, ">", PRIM_INTEGER_GT, 1);
    add_method_to_dict(dict, "<=", PRIM_INTEGER_LTE, 1);
    add_method_to_dict(dict, ">=", PRIM_INTEGER_GTE, 1);
    add_method_to_dict(dict, "=", PRIM_INTEGER_EQ, 1);
    add_method_to_dict(dict, "~=", PRIM_INTEGER_NEQ, 1);
    
    // Conversion operations
    add_method_to_dict(dict, "asString", PRIM_INTEGER_AS_STRING, 0);
    
    // Iteration operations
    add_method_to_dict(dict, "to:do:", PRIM_INTEGER_TO_DO, 2);
    add_method_to_dict(dict, "timesRepeat:", PRIM_INTEGER_TIMES_REPEAT, 1);
    
    // Standard operations
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in Integer\n", dict->size);
    ezom_log("      Installed %d methods in Integer\n", dict->size);
    
    printf("   END ezom_install_integer_methods function\n");
    ezom_log("   END ezom_install_integer_methods function\n");
}

void ezom_install_string_methods(void) {
    ezom_class_t* string_class = EZOM_OBJECT_PTR(g_string_class);
    ezom_method_dict_t* dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(string_class->method_dict);
    
    add_method_to_dict(dict, "length", PRIM_STRING_LENGTH, 0);
    add_method_to_dict(dict, "+", PRIM_STRING_CONCAT, 1);
    add_method_to_dict(dict, "=", PRIM_STRING_EQUALS, 1);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in String\n", dict->size);
}

void ezom_install_array_methods(void) {
    ezom_class_t* array_class = EZOM_OBJECT_PTR(g_array_class);
    ezom_method_dict_t* dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(array_class->method_dict);
    
    add_method_to_dict(dict, "at:", PRIM_ARRAY_AT, 1);
    add_method_to_dict(dict, "at:put:", PRIM_ARRAY_AT_PUT, 2);
    add_method_to_dict(dict, "length", PRIM_ARRAY_LENGTH, 0);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in Array\n", dict->size);
}

void ezom_install_boolean_methods(void) {
    printf("   Installing Boolean methods...\n");
    ezom_log("   Installing Boolean methods...\n");
    
    printf("   g_true_class = 0x%06lX\n", (unsigned long)g_true_class);
    printf("   g_false_class = 0x%06lX\n", (unsigned long)g_false_class);
    ezom_log("   g_true_class = 0x%06lX\n", (unsigned long)g_true_class);
    ezom_log("   g_false_class = 0x%06lX\n", (unsigned long)g_false_class);
    
    if (!g_true_class) {
        printf("   ERROR: g_true_class is NULL!\n");
        ezom_log("   ERROR: g_true_class is NULL!\n");
        return;
    }
    
    if (!g_false_class) {
        printf("   ERROR: g_false_class is NULL!\n");
        ezom_log("   ERROR: g_false_class is NULL!\n");
        return;
    }
    
    // Install methods for True class
    ezom_class_t* true_class = EZOM_OBJECT_PTR(g_true_class);
    printf("   true_class->method_dict = 0x%06lX\n", (unsigned long)true_class->method_dict);
    ezom_log("   true_class->method_dict = 0x%06lX\n", (unsigned long)true_class->method_dict);
    
    if (!true_class->method_dict) {
        printf("   ERROR: true_class method_dict is NULL!\n");
        ezom_log("   ERROR: true_class method_dict is NULL!\n");
        return;
    }
    
    ezom_method_dict_t* true_dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(true_class->method_dict);
    
    add_method_to_dict(true_dict, "ifTrue:", PRIM_TRUE_IF_TRUE, 1);
    add_method_to_dict(true_dict, "ifFalse:", PRIM_TRUE_IF_FALSE, 1);
    add_method_to_dict(true_dict, "ifTrue:ifFalse:", PRIM_TRUE_IF_TRUE_IF_FALSE, 2);
    add_method_to_dict(true_dict, "not", PRIM_BOOLEAN_NOT, 0);
    add_method_to_dict(true_dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    // Install methods for False class
    ezom_class_t* false_class = EZOM_OBJECT_PTR(g_false_class);
    printf("   false_class->method_dict = 0x%06lX\n", (unsigned long)false_class->method_dict);
    ezom_log("   false_class->method_dict = 0x%06lX\n", (unsigned long)false_class->method_dict);
    
    if (!false_class->method_dict) {
        printf("   ERROR: false_class method_dict is NULL!\n");
        ezom_log("   ERROR: false_class method_dict is NULL!\n");
        return;
    }
    
    ezom_method_dict_t* false_dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(false_class->method_dict);
    
    add_method_to_dict(false_dict, "ifTrue:", PRIM_FALSE_IF_TRUE, 1);
    add_method_to_dict(false_dict, "ifFalse:", PRIM_FALSE_IF_FALSE, 1);
    add_method_to_dict(false_dict, "ifTrue:ifFalse:", PRIM_FALSE_IF_TRUE_IF_FALSE, 2);
    add_method_to_dict(false_dict, "not", PRIM_BOOLEAN_NOT, 0);
    add_method_to_dict(false_dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed methods in True (%d) and False (%d)\n", 
           true_dict->size, false_dict->size);
}

void ezom_install_block_methods(void) {
    ezom_class_t* block_class = EZOM_OBJECT_PTR(g_block_class);
    ezom_method_dict_t* dict = (ezom_method_dict_t*)EZOM_OBJECT_PTR(block_class->method_dict);
    
    add_method_to_dict(dict, "value", PRIM_BLOCK_VALUE, 0);
    add_method_to_dict(dict, "value:", PRIM_BLOCK_VALUE_WITH, 1);
    add_method_to_dict(dict, "println", PRIM_OBJECT_PRINTLN, 0);
    
    printf("      Installed %d methods in Block\n", dict->size);
}
