// Debug program to test symbol lookup issue
#include "include/ezom_memory.h"
#include "include/ezom_object.h"
#include "include/ezom_primitives.h"
#include "include/ezom_dispatch.h"
#include <stdio.h>
#include <string.h>

// Forward declarations
extern void ezom_bootstrap_classes(void);
extern void ezom_bootstrap_enhanced_classes(void);

void debug_symbol_creation() {
    printf("=== Debug Symbol Creation ===\n");
    
    // Create two identical symbols
    uint24_t symbol1 = ezom_create_symbol("+", 1);
    uint24_t symbol2 = ezom_create_symbol("+", 1);
    
    printf("Symbol1 address: 0x%06X\n", symbol1);
    printf("Symbol2 address: 0x%06X\n", symbol2);
    
    if (symbol1 && symbol2) {
        ezom_symbol_t* s1 = (ezom_symbol_t*)symbol1;
        ezom_symbol_t* s2 = (ezom_symbol_t*)symbol2;
        
        printf("Symbol1 data: '%.*s', length: %d\n", s1->length, s1->data, s1->length);
        printf("Symbol2 data: '%.*s', length: %d\n", s2->length, s2->data, s2->length);
        
        printf("Pointer equality: %s\n", (symbol1 == symbol2) ? "true" : "false");
        printf("Symbol comparison: %s\n", ezom_symbols_equal(symbol1, symbol2) ? "true" : "false");
        
        // Check string data comparison
        bool data_equal = (s1->length == s2->length) && 
                         (strncmp(s1->data, s2->data, s1->length) == 0);
        printf("Data comparison: %s\n", data_equal ? "true" : "false");
    }
}

void debug_method_installation() {
    printf("\n=== Debug Method Installation ===\n");
    
    if (!g_integer_class) {
        printf("ERROR: g_integer_class is NULL!\n");
        return;
    }
    
    ezom_class_t* int_class = (ezom_class_t*)g_integer_class;
    if (!int_class->method_dict) {
        printf("ERROR: Integer class has no method dictionary!\n");
        return;
    }
    
    ezom_method_dict_t* dict = (ezom_method_dict_t*)int_class->method_dict;
    printf("Integer class method dictionary:\n");
    printf("  Size: %d/%d\n", dict->size, dict->capacity);
    
    for (uint16_t i = 0; i < dict->size; i++) {
        ezom_method_t* method = &dict->methods[i];
        if (method->selector) {
            ezom_symbol_t* selector = (ezom_symbol_t*)method->selector;
            printf("  Method %d: '%.*s' -> primitive %d, args: %d\n", 
                   i, selector->length, selector->data, 
                   (int)method->code, method->arg_count);
        }
    }
}

void debug_method_lookup() {
    printf("\n=== Debug Method Lookup ===\n");
    
    // Create a symbol for lookup
    uint24_t plus_symbol = ezom_create_symbol("+", 1);
    printf("Created lookup symbol: 0x%06X\n", plus_symbol);
    
    if (plus_symbol) {
        ezom_symbol_t* sym = (ezom_symbol_t*)plus_symbol;
        printf("Lookup symbol data: '%.*s', length: %d\n", 
               sym->length, sym->data, sym->length);
    }
    
    // Try to find the method
    printf("Looking up '+' method in Integer class...\n");
    ezom_method_lookup_t lookup = ezom_lookup_method(g_integer_class, plus_symbol);
    
    if (lookup.method) {
        printf("SUCCESS: Found method!\n");
        printf("  Method address: %p\n", lookup.method);
        printf("  Is primitive: %s\n", lookup.is_primitive ? "true" : "false");
        printf("  Primitive code: %d\n", (int)lookup.method->code);
    } else {
        printf("FAILED: Method not found!\n");
        
        // Debug: Check each method in the dictionary
        ezom_class_t* int_class = (ezom_class_t*)g_integer_class;
        ezom_method_dict_t* dict = (ezom_method_dict_t*)int_class->method_dict;
        
        printf("Checking each method in dictionary:\n");
        for (uint16_t i = 0; i < dict->size; i++) {
            ezom_method_t* method = &dict->methods[i];
            if (method->selector) {
                ezom_symbol_t* method_selector = (ezom_symbol_t*)method->selector;
                printf("  Method %d selector: 0x%06X, data: '%.*s'\n", 
                       i, method->selector, method_selector->length, method_selector->data);
                
                bool equal = ezom_symbols_equal(method->selector, plus_symbol);
                printf("    Comparison with lookup symbol: %s\n", equal ? "EQUAL" : "NOT EQUAL");
                
                if (!equal) {
                    // Debug the comparison
                    ezom_symbol_t* s1 = (ezom_symbol_t*)method->selector;
                    ezom_symbol_t* s2 = (ezom_symbol_t*)plus_symbol;
                    printf("    Detailed comparison:\n");
                    printf("      Length equal: %s (%d vs %d)\n", 
                           (s1->length == s2->length) ? "true" : "false",
                           s1->length, s2->length);
                    printf("      Data equal: %s\n", 
                           (strncmp(s1->data, s2->data, s1->length) == 0) ? "true" : "false");
                }
            }
        }
    }
}

int main() {
    printf("EZOM Symbol Lookup Debug Tool\n");
    printf("=============================\n");
    
    // Initialize the VM
    g_nil = 1; // Temporary non-zero value
    ezom_init_memory();
    ezom_init_object_system();
    ezom_init_primitives();
    
    // Bootstrap classes
    ezom_bootstrap_classes();
    ezom_bootstrap_enhanced_classes();
    
    // Debug symbol creation
    debug_symbol_creation();
    
    // Debug method installation
    debug_method_installation();
    
    // Debug method lookup
    debug_method_lookup();
    
    return 0;
}