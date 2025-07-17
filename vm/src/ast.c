// ============================================================================
// File: src/ast.c
// Abstract Syntax Tree implementation
// ============================================================================

#include "../include/ezom_ast.h"
#include "../include/ezom_ast_memory.h"
#include "../include/ezom_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
static void ezom_ast_print_indent(int indent);
static void ezom_ast_free_variable_list(ezom_ast_node_t* node);

ezom_ast_node_t* ezom_ast_create(ezom_ast_type_t type) {
    // Use AST memory pool instead of malloc
    ezom_ast_node_t* node = ezom_ast_create_node(type);
    if (!node) return NULL;
    
    node->line = 0;
    node->column = 0;
    node->next = NULL;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_class_def(char* name, ezom_ast_node_t* superclass) {
    ezom_ast_node_t* node = ezom_ast_create(AST_CLASS_DEF);
    if (!node) return NULL;
    
    node->data.class_def.name = ezom_ast_strdup(name);  // Use AST pool strdup
    node->data.class_def.superclass = superclass;
    node->data.class_def.instance_vars = NULL;
    node->data.class_def.instance_methods = NULL;
    node->data.class_def.class_methods = NULL;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_method_def(char* selector, bool is_class_method) {
    ezom_ast_node_t* node = ezom_ast_create(AST_METHOD_DEF);
    if (!node) return NULL;
    
    node->data.method_def.selector = ezom_ast_strdup(selector);  // Use AST pool strdup
    node->data.method_def.parameters = NULL;
    node->data.method_def.locals = NULL;
    node->data.method_def.body = NULL;
    node->data.method_def.is_class_method = is_class_method;
    node->data.method_def.is_primitive = false;
    node->data.method_def.primitive_number = 0;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_message_send(ezom_ast_node_t* receiver, char* selector) {
    ezom_ast_node_t* node = ezom_ast_create(AST_MESSAGE_SEND);
    if (!node) return NULL;
    
    node->data.message_send.receiver = receiver;
    node->data.message_send.selector = strdup(selector);
    node->data.message_send.arguments = NULL;
    node->data.message_send.is_super = false;
    node->data.message_send.arg_count = 0;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_block(void) {
    ezom_ast_node_t* node = ezom_ast_create(AST_BLOCK);
    if (!node) return NULL;
    
    node->data.block.parameters = NULL;
    node->data.block.locals = NULL;
    node->data.block.body = NULL;
    node->data.block.param_count = 0;
    node->data.block.local_count = 0;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_return(ezom_ast_node_t* expression) {
    ezom_ast_node_t* node = ezom_ast_create(AST_RETURN);
    if (!node) return NULL;
    
    node->data.return_stmt.expression = expression;
    node->data.return_stmt.is_block_return = false;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_assignment(ezom_ast_node_t* variable, ezom_ast_node_t* value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_ASSIGNMENT);
    if (!node) return NULL;
    
    node->data.assignment.variable = variable;
    node->data.assignment.value = value;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_literal_integer(int16_t value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_LITERAL);
    if (!node) return NULL;
    
    node->data.literal.type = LITERAL_INTEGER;
    node->data.literal.value.integer_value = value;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_literal_string(const char* value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_LITERAL);
    if (!node) return NULL;
    if (!value) {
        free(node);
        return NULL;
    }
    
    node->data.literal.type = LITERAL_STRING;
    node->data.literal.value.string_value = strdup(value);
    if (!node->data.literal.value.string_value) {
        free(node);
        return NULL;
    }
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_literal_symbol(const char* value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_LITERAL);
    if (!node) return NULL;
    if (!value) {
        free(node);
        return NULL;
    }
    
    node->data.literal.type = LITERAL_SYMBOL;
    node->data.literal.value.symbol_value = strdup(value);
    if (!node->data.literal.value.symbol_value) {
        free(node);
        return NULL;
    }
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_identifier(const char* name) {
    ezom_ast_node_t* node = ezom_ast_create(AST_IDENTIFIER);
    if (!node) return NULL;
    if (!name) {
        free(node);
        return NULL;
    }
    
    node->data.identifier.name = strdup(name);
    if (!node->data.identifier.name) {
        free(node);
        return NULL;
    }
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_variable_list(void) {
    ezom_ast_node_t* node = ezom_ast_create(AST_VARIABLE_LIST);
    if (!node) return NULL;
    
    node->data.variable_list.names = NULL;
    node->data.variable_list.count = 0;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_statement_list(void) {
    ezom_ast_node_t* node = ezom_ast_create(AST_STATEMENT_LIST);
    if (!node) return NULL;
    
    node->data.statement_list.statements = NULL;
    node->data.statement_list.count = 0;
    
    return node;
}

ezom_ast_node_t* ezom_ast_create_variable(const char* name) {
    ezom_ast_node_t* node = ezom_ast_create(AST_VARIABLE_DEF);
    if (!node) return NULL;
    if (!name) {
        free(node);
        return NULL;
    }
    
    node->data.variable.name = strdup(name);
    node->data.variable.is_instance_var = false;
    node->data.variable.is_local = false;
    node->data.variable.index = 0;
    
    if (!node->data.variable.name) {
        free(node);
        return NULL;
    }
    
    return node;
}

// AST manipulation functions

void ezom_ast_add_statement(ezom_ast_node_t* list, ezom_ast_node_t* statement) {
    if (!list || list->type != AST_STATEMENT_LIST || !statement) return;
    
    // Add to linked list
    if (!list->data.statement_list.statements) {
        list->data.statement_list.statements = statement;
    } else {
        ezom_ast_node_t* current = list->data.statement_list.statements;
        while (current->next) {
            current = current->next;
        }
        current->next = statement;
    }
    
    list->data.statement_list.count++;
}

void ezom_ast_add_variable(ezom_ast_node_t* list, char* name) {
    if (!list || list->type != AST_VARIABLE_LIST || !name) return;
    
    // Resize array
    uint16_t new_count = list->data.variable_list.count + 1;
    char** new_names = realloc(list->data.variable_list.names, 
                               new_count * sizeof(char*));
    if (!new_names) return;
    
    new_names[list->data.variable_list.count] = strdup(name);
    list->data.variable_list.names = new_names;
    list->data.variable_list.count = new_count;
}

void ezom_ast_add_argument(ezom_ast_node_t* message, ezom_ast_node_t* argument) {
    if (!message || message->type != AST_MESSAGE_SEND || !argument) return;
    
    // Add to linked list
    if (!message->data.message_send.arguments) {
        message->data.message_send.arguments = argument;
    } else {
        ezom_ast_node_t* current = message->data.message_send.arguments;
        while (current->next) {
            current = current->next;
        }
        current->next = argument;
    }
    
    message->data.message_send.arg_count++;
}

void ezom_ast_set_line_info(ezom_ast_node_t* node, uint16_t line, uint16_t column) {
    if (!node) return;
    node->line = line;
    node->column = column;
}

// Utility functions

uint16_t ezom_ast_count_parameters(ezom_ast_node_t* params) {
    if (!params || params->type != AST_VARIABLE_LIST) return 0;
    return params->data.variable_list.count;
}

uint16_t ezom_ast_count_locals(ezom_ast_node_t* locals) {
    if (!locals || locals->type != AST_VARIABLE_LIST) return 0;
    return locals->data.variable_list.count;
}

uint16_t ezom_ast_count_variables(ezom_ast_node_t* variables) {
    if (!variables || variables->type != AST_VARIABLE_LIST) return 0;
    return variables->data.variable_list.count;
}

// Memory management

void ezom_ast_free(ezom_ast_node_t* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_CLASS_DEF:
            free(node->data.class_def.name);
            ezom_ast_free(node->data.class_def.superclass);
            ezom_ast_free(node->data.class_def.instance_vars);
            ezom_ast_free(node->data.class_def.instance_methods);
            ezom_ast_free(node->data.class_def.class_methods);
            break;
            
        case AST_METHOD_DEF:
            free(node->data.method_def.selector);
            ezom_ast_free(node->data.method_def.parameters);
            ezom_ast_free(node->data.method_def.locals);
            ezom_ast_free(node->data.method_def.body);
            break;
            
        case AST_MESSAGE_SEND:
            free(node->data.message_send.selector);
            ezom_ast_free(node->data.message_send.receiver);
            ezom_ast_free(node->data.message_send.arguments);
            break;
            
        case AST_BLOCK:
            ezom_ast_free(node->data.block.parameters);
            ezom_ast_free(node->data.block.locals);
            ezom_ast_free(node->data.block.body);
            break;
            
        case AST_RETURN:
            ezom_ast_free(node->data.return_stmt.expression);
            break;
            
        case AST_ASSIGNMENT:
            ezom_ast_free(node->data.assignment.variable);
            ezom_ast_free(node->data.assignment.value);
            break;
            
        case AST_LITERAL:
            if (node->data.literal.type == LITERAL_STRING) {
                free(node->data.literal.value.string_value);
            } else if (node->data.literal.type == LITERAL_SYMBOL) {
                free(node->data.literal.value.symbol_value);
            } else if (node->data.literal.type == LITERAL_ARRAY) {
                ezom_ast_free(node->data.literal.value.array_elements);
            }
            break;
            
        case AST_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case AST_VARIABLE_LIST:
            ezom_ast_free_variable_list(node);
            break;
            
        case AST_STATEMENT_LIST:
            ezom_ast_free(node->data.statement_list.statements);
            break;
            
        default:
            break;
    }
    
    // Free next node in linked list
    ezom_ast_free(node->next);
    
    free(node);
}

static void ezom_ast_free_variable_list(ezom_ast_node_t* node) {
    if (node->data.variable_list.names) {
        for (uint16_t i = 0; i < node->data.variable_list.count; i++) {
            free(node->data.variable_list.names[i]);
        }
        free(node->data.variable_list.names);
    }
}

// Debug printing

void ezom_ast_print(ezom_ast_node_t* node, int indent) {
    if (!node) return;
    
    ezom_ast_print_indent(indent);
    
    switch (node->type) {
        case AST_CLASS_DEF:
            printf("ClassDef: %s\n", node->data.class_def.name);
            if (node->data.class_def.superclass) {
                ezom_ast_print_indent(indent + 1);
                printf("Superclass:\n");
                ezom_ast_print(node->data.class_def.superclass, indent + 2);
            }
            if (node->data.class_def.instance_vars) {
                ezom_ast_print_indent(indent + 1);
                printf("Instance vars:\n");
                ezom_ast_print(node->data.class_def.instance_vars, indent + 2);
            }
            if (node->data.class_def.instance_methods) {
                ezom_ast_print_indent(indent + 1);
                printf("Instance methods:\n");
                ezom_ast_print(node->data.class_def.instance_methods, indent + 2);
            }
            break;
            
        case AST_METHOD_DEF:
            printf("MethodDef: %s (%s)\n", 
                   node->data.method_def.selector,
                   node->data.method_def.is_class_method ? "class" : "instance");
            break;
            
        case AST_MESSAGE_SEND:
            printf("MessageSend: %s\n", node->data.message_send.selector);
            if (node->data.message_send.receiver) {
                ezom_ast_print_indent(indent + 1);
                printf("Receiver:\n");
                ezom_ast_print(node->data.message_send.receiver, indent + 2);
            }
            break;
            
        case AST_LITERAL:
            printf("Literal: ");
            switch (node->data.literal.type) {
                case LITERAL_INTEGER:
                    printf("integer %d\n", node->data.literal.value.integer_value);
                    break;
                case LITERAL_STRING:
                    printf("string '%s'\n", node->data.literal.value.string_value);
                    break;
                case LITERAL_SYMBOL:
                    printf("symbol #%s\n", node->data.literal.value.symbol_value);
                    break;
                default:
                    printf("unknown\n");
                    break;
            }
            break;
            
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->data.identifier.name);
            break;
            
        case AST_VARIABLE_LIST:
            printf("Variables: ");
            for (uint16_t i = 0; i < node->data.variable_list.count; i++) {
                printf("%s ", node->data.variable_list.names[i]);
            }
            printf("\n");
            break;
            
        default:
            printf("Node type: %d\n", node->type);
            break;
    }
    
    // Print next node in list
    if (node->next) {
        ezom_ast_print(node->next, indent);
    }
}

static void ezom_ast_print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

// Validation

bool ezom_ast_validate(ezom_ast_node_t* node, char* error_buffer, size_t buffer_size) {
    if (!node) {
        snprintf(error_buffer, buffer_size, "NULL AST node");
        return false;
    }
    
    switch (node->type) {
        case AST_CLASS_DEF:
            if (!node->data.class_def.name) {
                snprintf(error_buffer, buffer_size, "Class definition missing name");
                return false;
            }
            break;
            
        case AST_METHOD_DEF:
            if (!node->data.method_def.selector) {
                snprintf(error_buffer, buffer_size, "Method definition missing selector");
                return false;
            }
            break;
            
        case AST_MESSAGE_SEND:
            if (!node->data.message_send.selector) {
                snprintf(error_buffer, buffer_size, "Message send missing selector");
                return false;
            }
            break;
            
        case AST_IDENTIFIER:
            if (!node->data.identifier.name) {
                snprintf(error_buffer, buffer_size, "Identifier missing name");
                return false;
            }
            break;
            
        default:
            break;
    }
    
    return true;
}

// Simple AST creation functions for testing
ezom_ast_node_t* ezom_ast_create_integer_literal(int value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_LITERAL);
    if (!node) return NULL;
    
    node->data.literal.type = LITERAL_INTEGER;
    node->data.literal.value.integer_value = (int16_t)value;
    
    printf("DEBUG: Created integer literal AST node: %d\n", value);
    return node;
}

ezom_ast_node_t* ezom_ast_create_string_literal(char* value) {
    ezom_ast_node_t* node = ezom_ast_create(AST_LITERAL);
    if (!node) return NULL;
    
    node->data.literal.type = LITERAL_STRING;
    node->data.literal.value.string_value = ezom_ast_strdup(value);
    
    printf("DEBUG: Created string literal AST node: '%s'\n", value);
    return node;
}

ezom_ast_node_t* ezom_ast_create_binary_message(ezom_ast_node_t* receiver, char* selector, ezom_ast_node_t* argument) {
    ezom_ast_node_t* node = ezom_ast_create(AST_BINARY_MESSAGE);
    if (!node) return NULL;
    
    node->data.message_send.receiver = receiver;
    node->data.message_send.selector = ezom_ast_strdup(selector);
    node->data.message_send.arguments = argument;  // Single argument for binary message
    node->data.message_send.is_super = false;
    node->data.message_send.arg_count = 1;
    
    printf("DEBUG: Created binary message AST node: selector='%s'\n", selector);
    return node;
}