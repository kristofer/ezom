// Simple debug test to isolate bootstrap crash
#include "vm/include/ezom_platform.h"
#include "vm/include/ezom_memory.h"
#include "vm/include/ezom_object.h"
// #include "vm/include/ezom_objects.h"
#include <stdio.h>

// External globals
extern uint24_t g_object_class;
extern uint24_t g_nil;

// Forward declarations
void ezom_init_memory(void);
void ezom_init_object_system(void);
uint24_t ezom_create_method_dictionary(uint16_t capacity);

int main() {
    printf("Debug Bootstrap Simple Test\n");
    printf("===========================\n");
    
    // Initialize platform and memory
    ezom_platform_init();
    ezom_init_memory();
    
    printf("Memory initialized\n");
    
    // Initialize nil as a temporary non-zero value
    g_nil = 1;
    
    printf("Setting up object system...\n");
    ezom_init_object_system();
    
    printf("Creating Object class...\n");
    
    // Test basic allocation
    uint24_t addr = ezom_allocate(sizeof(ezom_class_t));
    printf("Allocated ezom_class_t at: 0x%06X\n", addr);
    
    if (addr == 0) {
        printf("ERROR: Allocation failed!\n");
        return 1;
    }
    
    // Test pointer conversion
    ezom_class_t* class_ptr = EZOM_OBJECT_PTR(addr);
    printf("Converted to native pointer: %p\n", class_ptr);
    
    // Test writing basic fields  
    printf("Setting basic fields...\n");
    class_ptr->superclass = 0;
    class_ptr->method_dict = 0;
    class_ptr->instance_vars = 0;
    class_ptr->instance_size = sizeof(ezom_object_t);
    class_ptr->instance_var_count = 0;
    
    printf("Basic fields set successfully\n");
    
    // Test method dictionary creation
    printf("Creating method dictionary...\n");
    uint24_t method_dict = ezom_create_method_dictionary(8);
    printf("Method dictionary created at: 0x%06X\n", method_dict);
    
    if (method_dict == 0) {
        printf("ERROR: Method dictionary creation failed!\n");
        return 1;
    }
    
    // Test setting method dictionary
    printf("Setting method dictionary...\n");
    class_ptr->method_dict = method_dict;
    
    printf("Method dictionary set successfully\n");
    
    // Test setting as global
    printf("Setting as g_object_class...\n");
    g_object_class = addr;
    
    printf("Test completed successfully!\n");
    
    // Cleanup
    ezom_cleanup_memory();
    ezom_platform_cleanup();
    
    return 0;
}