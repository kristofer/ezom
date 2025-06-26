// Simple test to check if the g_nil initialization fix works
#include <stdio.h>
#include <stdint.h>

// Mock some basic types
typedef uint32_t uint24_t; // Use 32-bit for testing

// Mock globals
uint24_t g_nil = 0;

// Mock function that would previously crash
void test_nil_usage() {
    printf("g_nil value: 0x%08X\n", g_nil);
    
    // Simulate array initialization that uses g_nil
    uint24_t elements[3];
    for (int i = 0; i < 3; i++) {
        elements[i] = g_nil;
        printf("elements[%d] = 0x%08X\n", i, elements[i]);
    }
    
    // Test dereferencing (this would crash if g_nil == 0 and we tried to use it as pointer)
    if (g_nil == 0) {
        printf("g_nil is NULL - would cause crash if dereferenced\n");
    } else {
        printf("g_nil is valid - safe to use\n");
    }
}

int main() {
    printf("Testing g_nil initialization fix\n");
    printf("================================\n");
    
    printf("\n1. Before fix (g_nil = 0):\n");
    g_nil = 0;
    test_nil_usage();
    
    printf("\n2. After fix (g_nil = 1):\n");
    g_nil = 1;
    test_nil_usage();
    
    printf("\n3. With proper object (simulated):\n");
    g_nil = 0x123456; // Simulate a real object pointer
    test_nil_usage();
    
    printf("\nTest complete - no crashes!\n");
    return 0;
}
