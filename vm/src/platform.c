// ============================================================================
// File: vm/src/platform.c
// Platform abstraction layer implementation
// ============================================================================

#include "../include/ezom_platform.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef EZOM_PLATFORM_NATIVE
void* g_heap_base = NULL;
#endif

void ezom_platform_init(void) {
#ifdef EZOM_PLATFORM_NATIVE
    // For native development, we'll initialize this when memory is set up
    printf("EZOM: Native platform initialized\n");
#else
    // ez80 platform initialization
    printf("EZOM: ez80 platform initialized\n");
#endif
}

void ezom_platform_cleanup(void) {
#ifdef EZOM_PLATFORM_NATIVE
    // Native cleanup is handled by memory management
    printf("EZOM: Native platform cleanup\n");
#else
    // ez80 cleanup (if needed)
    printf("EZOM: ez80 platform cleanup\n");
#endif
}

void ezom_platform_print(const char* str) {
    printf("%s", str);
}

void ezom_platform_print_char(char c) {
    putchar(c);
}

void ezom_platform_print_int(int value) {
    printf("%d", value);
}

void ezom_platform_print_ptr(void* ptr) {
    printf("0x%p", ptr);
}