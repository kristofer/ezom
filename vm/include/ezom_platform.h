// ============================================================================
// File: vm/include/ezom_platform.h
// Platform abstraction layer for native and ez80 development
// ============================================================================

#pragma once
#include <stdint.h>
#include <stddef.h>

// Platform-specific definitions
#ifdef __ez80__
    // ez80 Agon Light 2 platform
    #define EZOM_PLATFORM_EZ80
    typedef unsigned long uint24_t;
    #define EZOM_PTR_SIZE 3
    #define EZOM_HEAP_FIXED_ADDRESS 0x042000
    #define EZOM_DEFAULT_HEAP_SIZE 0x0E000  // 56KB
    
    // On ez80, uint24_t IS the pointer type
    typedef uint24_t ezom_ptr_t;
    #define EZOM_PTR_TO_NATIVE(ptr) ((void*)(ptr))
    #define EZOM_PTR_FROM_NATIVE(ptr) ((ezom_ptr_t)(ptr))
    #define EZOM_PTR_NULL 0
    
#else
    // Native platform (development)
    #define EZOM_PLATFORM_NATIVE
    typedef uint32_t uint24_t;
    #define EZOM_PTR_SIZE sizeof(void*)
    #define EZOM_DEFAULT_HEAP_SIZE 0x40000  // 256KB for native
    
    // On native, use actual pointers but provide conversion
    typedef void* ezom_ptr_t;
    #define EZOM_PTR_TO_NATIVE(ptr) (ptr)
    #define EZOM_PTR_FROM_NATIVE(ptr) (ptr)
    #define EZOM_PTR_NULL NULL
    
    // For native, we need to track the heap base for conversions
    extern void* g_heap_base;
    
    // Convert between uint24_t addresses and native pointers
    static inline void* ezom_ptr_to_native(uint24_t addr) {
        if (addr == 0) return NULL;
        return (char*)g_heap_base + (addr - 0x042000);
    }
    
    static inline uint24_t ezom_ptr_from_native(void* ptr) {
        if (ptr == NULL) return 0;
        return 0x042000 + ((char*)ptr - (char*)g_heap_base);
    }
#endif

// Platform-specific memory functions
void ezom_platform_init(void);
void ezom_platform_cleanup(void);

// Platform-specific I/O
void ezom_platform_print(const char* str);
void ezom_platform_print_char(char c);
void ezom_platform_print_int(int value);
void ezom_platform_print_ptr(void* ptr);