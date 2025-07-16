// Simple debug test for bootstrap issue
#include "vm/include/ezom_platform.h"
#include "vm/include/ezom_memory.h"
#include "vm/include/ezom_object.h"
#include <stdio.h>

int main() {
    printf("Debug Bootstrap Test\n");
    printf("====================\n");
    
    // Initialize platform and memory
    ezom_platform_init();
    ezom_init_memory();
    
    printf("Memory initialized\n");
    
    // Test basic allocation
    uint24_t addr = ezom_allocate(sizeof(ezom_class_t));
    printf("Allocated address: 0x%06X\n", addr);
    
    if (addr == 0) {
        printf("ERROR: Allocation failed!\n");
        return 1;
    }
    
    // Test pointer conversion
    #ifdef EZOM_PLATFORM_NATIVE
    void* native_ptr = ezom_ptr_to_native(addr);
    printf("Native pointer: %p\n", native_ptr);
    
    // Test writing to memory
    ezom_class_t* class_ptr = (ezom_class_t*)native_ptr;
    class_ptr->superclass = 0;
    printf("Successfully wrote to memory\n");
    
    // Test reading back
    printf("Superclass: 0x%06X\n", class_ptr->superclass);
    #else
    ezom_class_t* class_ptr = (ezom_class_t*)addr;
    class_ptr->superclass = 0;
    printf("Successfully wrote to memory (ez80)\n");
    #endif
    
    // Cleanup
    ezom_cleanup_memory();
    ezom_platform_cleanup();
    
    printf("Test completed successfully!\n");
    return 0;
}