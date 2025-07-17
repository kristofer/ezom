// ============================================================================
// File: src/parser.c
// Parser implementation for EZOM language
// ============================================================================

#include "../include/ezom_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
static void ezom_parser_skip_newlines(ezom_parser_t* parser);

void ezom_parser_init(ezom_parser_t* parser, ezom_lexer_t* lexer) {
    parser->lexer = lexer;
    parser->has_error = false;
    parser->error_message[0] = '\0';
    parser->error_count = 0;
}

// Class definition parsing
// Syntax: ClassName = SuperClass ( | instanceVars | methods ---- classMethods )
ezom_ast_node_t* ezom_parse_class_definition(ezom_parser_t* parser) {
    ezom_parser_skip_newlines(parser);
    
    // Parse class name
    if (!ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
        ezom_parser_error(parser, "Expected class name");
        return NULL;
    }
    
    char* class_name = ezom_copy_current_token_text(parser);
    ezom_parser_advance(parser);
    
    // Parse = SuperClass
    ezom_ast_node_t* superclass = NULL;
    if (ezom_parser_match(parser, TOKEN_EQUALS)) {
        if (!ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
            ezom_parser_error(parser, "Expected superclass name");
            free(class_name);
            return NULL;
        }
        superclass = ezom_ast_create_identifier(ezom_copy_current_token_text(parser));
        ezom_parser_advance(parser);
    }
    
    ezom_ast_node_t* class_def = ezom_ast_create_class_def(class_name, superclass);
    free(class_name);
    
    // Parse (
    ezom_parser_consume(parser, TOKEN_LPAREN, "Expected '(' after class declaration");
    ezom_parser_skip_newlines(parser);
    
    // Parse instance variables | var1 var2 |
    if (ezom_parser_match(parser, TOKEN_PIPE)) {
        class_def->data.class_def.instance_vars = ezom_parse_variable_list(parser);
        ezom_parser_consume(parser, TOKEN_PIPE, "Expected '|' after instance variables");
        ezom_parser_skip_newlines(parser);
    }
    
    // Parse instance methods
    ezom_ast_node_t* instance_methods = ezom_ast_create_statement_list();
    while (!ezom_parser_check(parser, TOKEN_SEPARATOR) && 
           !ezom_parser_check(parser, TOKEN_RPAREN) && 
           !ezom_parser_check(parser, TOKEN_EOF)) {
        
        ezom_ast_node_t* method = ezom_parse_method_definition(parser, false);
        if (method) {
            ezom_ast_add_statement(instance_methods, method);
        } else {
            // If method parsing failed, synchronize to avoid infinite loop
            ezom_parser_synchronize(parser);
            // If we're still stuck, advance one token
            if (!ezom_parser_check(parser, TOKEN_SEPARATOR) && 
                !ezom_parser_check(parser, TOKEN_RPAREN) && 
                !ezom_parser_check(parser, TOKEN_EOF)) {
                ezom_parser_advance(parser);
            }
        }
        ezom_parser_skip_newlines(parser);
    }
    class_def->data.class_def.instance_methods = instance_methods;
    
    // Parse class methods (after ----)
    if (ezom_parser_match(parser, TOKEN_SEPARATOR)) {
        ezom_parser_skip_newlines(parser);
        
        ezom_ast_node_t* class_methods = ezom_ast_create_statement_list();
        while (!ezom_parser_check(parser, TOKEN_RPAREN) && 
               !ezom_parser_check(parser, TOKEN_EOF)) {
            
            ezom_ast_node_t* method = ezom_parse_method_definition(parser, true);
            if (method) {
                ezom_ast_add_statement(class_methods, method);
            } else {
                // If method parsing failed, synchronize to avoid infinite loop
                ezom_parser_synchronize(parser);
                // If we're still stuck, advance one token
                if (!ezom_parser_check(parser, TOKEN_RPAREN) && 
                    !ezom_parser_check(parser, TOKEN_EOF)) {
                    ezom_parser_advance(parser);
                }
            }
            ezom_parser_skip_newlines(parser);
        }
        class_def->data.class_def.class_methods = class_methods;
    }
    
    // Parse )
    ezom_parser_consume(parser, TOKEN_RPAREN, "Expected ')' after class definition");
    
    return class_def;
}

// Method definition parsing
// Syntax: selector = ( | locals | statements )
ezom_ast_node_t* ezom_parse_method_definition(ezom_parser_t* parser, bool is_class_method) {
    ezom_parser_skip_newlines(parser);
    
    // Parse method selector and parameters
    char* selector = NULL;
    ezom_ast_node_t* parameters = NULL;
    
    if (ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
        // Unary or keyword selector
        if (ezom_is_keyword_start(parser)) {
            selector = ezom_parse_keyword_method_signature(parser, &parameters);
        } else {
            selector = ezom_parse_unary_selector(parser);
        }
    } else if (ezom_is_binary_operator(parser->lexer->current_token.type)) {
        selector = ezom_parse_binary_method_signature(parser, &parameters);
    } else {
        ezom_parser_error(parser, "Expected method selector");
        return NULL;
    }
    
    ezom_ast_node_t* method = ezom_ast_create_method_def(selector, is_class_method);
    method->data.method_def.parameters = parameters;
    free(selector);
    
    // Parse = (
    ezom_parser_consume(parser, TOKEN_EQUALS, "Expected '=' after method selector");
    ezom_parser_consume(parser, TOKEN_LPAREN, "Expected '(' after '='");
    ezom_parser_skip_newlines(parser);
    
    // Parse local variables | local1 local2 |
    if (ezom_parser_match(parser, TOKEN_PIPE)) {
        method->data.method_def.locals = ezom_parse_variable_list(parser);
        ezom_parser_consume(parser, TOKEN_PIPE, "Expected '|' after local variables");
        ezom_parser_skip_newlines(parser);
    }
    
    // Parse method body (statements)
    method->data.method_def.body = ezom_parse_statement_list(parser);
    
    // Parse )
    ezom_parser_consume(parser, TOKEN_RPAREN, "Expected ')' after method body");
    
    return method;
}

// Expression parsing with precedence
ezom_ast_node_t* ezom_parse_expression(ezom_parser_t* parser) {
    ezom_ast_node_t* expr = ezom_parse_primary(parser);
    if (!expr) return NULL;
    
    // Parse message sends with precedence:
    // 1. Unary messages (highest precedence)
    // 2. Binary messages 
    // 3. Keyword messages (lowest precedence)
    
    // Parse unary messages
    while (ezom_parser_check(parser, TOKEN_IDENTIFIER) && 
           !ezom_is_keyword_start(parser)) {
        expr = ezom_parse_unary_message(parser, expr);
        if (!expr) return NULL;
    }
    
    // Parse binary messages
    while (ezom_is_binary_operator(parser->lexer->current_token.type)) {
        expr = ezom_parse_binary_message(parser, expr);
        if (!expr) return NULL;
    }
    
    // Parse keyword messages
    if (ezom_is_keyword_start(parser)) {
        expr = ezom_parse_keyword_message(parser, expr);
    }
    
    return expr;
}

ezom_ast_node_t* ezom_parse_primary(ezom_parser_t* parser) {
    switch (parser->lexer->current_token.type) {
        case TOKEN_INTEGER:
        case TOKEN_STRING:
        case TOKEN_SYMBOL:
            return ezom_parse_literal(parser);
            
        case TOKEN_IDENTIFIER:
            {
                char* name = ezom_copy_current_token_text(parser);
                ezom_parser_advance(parser);
                return ezom_ast_create_identifier(name);
            }
            
        case TOKEN_LPAREN:
            // Parenthesized expression
            ezom_parser_advance(parser);
            ezom_ast_node_t* expr = ezom_parse_expression(parser);
            ezom_parser_consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
            return expr;
            
        case TOKEN_LBRACKET:
            return ezom_parse_block(parser);
            
        case TOKEN_HASH:
            if (ezom_parser_check(parser, TOKEN_LPAREN)) {
                return ezom_parse_array_literal(parser);
            }
            // Fall through for symbol
            
        default:
            ezom_parser_error(parser, "Expected expression");
            return NULL;
    }
}

ezom_ast_node_t* ezom_parse_unary_message(ezom_parser_t* parser, ezom_ast_node_t* receiver) {
    char* selector = ezom_copy_current_token_text(parser);
    ezom_parser_advance(parser);
    
    ezom_ast_node_t* message = ezom_ast_create_message_send(receiver, selector);
    free(selector);
    
    return message;
}

ezom_ast_node_t* ezom_parse_binary_message(ezom_parser_t* parser, ezom_ast_node_t* receiver) {
    char* selector = ezom_copy_current_token_text(parser);
    ezom_parser_advance(parser);
    
    ezom_ast_node_t* argument = ezom_parse_primary(parser);
    if (!argument) {
        free(selector);
        return NULL;
    }
    
    // Parse any unary messages on the argument
    while (ezom_parser_check(parser, TOKEN_IDENTIFIER) && 
           !ezom_is_keyword_start(parser)) {
        argument = ezom_parse_unary_message(parser, argument);
        if (!argument) {
            free(selector);
            return NULL;
        }
    }
    
    ezom_ast_node_t* message = ezom_ast_create_message_send(receiver, selector);
    ezom_ast_add_argument(message, argument);
    free(selector);
    
    return message;
}

ezom_ast_node_t* ezom_parse_keyword_message(ezom_parser_t* parser, ezom_ast_node_t* receiver) {
    char selector[256] = "";
    ezom_ast_node_t* arguments = NULL;
    
    // Parse keyword:argument pairs
    while (ezom_parser_check(parser, TOKEN_IDENTIFIER) && 
           ezom_is_keyword_start(parser)) {
        
        // Parse keyword
        char* keyword = ezom_copy_current_token_text(parser);
        strcat(selector, keyword);
        free(keyword);
        ezom_parser_advance(parser);
        
        ezom_parser_consume(parser, TOKEN_COLON, "Expected ':' after keyword");
        strcat(selector, ":");
        
        // Parse argument
        ezom_ast_node_t* arg = ezom_parse_primary(parser);
        if (!arg) return NULL;
        
        // Parse unary and binary messages on argument
        while (ezom_parser_check(parser, TOKEN_IDENTIFIER) && 
               !ezom_is_keyword_start(parser)) {
            arg = ezom_parse_unary_message(parser, arg);
            if (!arg) return NULL;
        }
        
        while (ezom_is_binary_operator(parser->lexer->current_token.type)) {
            arg = ezom_parse_binary_message(parser, arg);
            if (!arg) return NULL;
        }
        
        // Add argument to list
        if (!arguments) {
            arguments = arg;
        } else {
            ezom_ast_node_t* current = arguments;
            while (current->next) {
                current = current->next;
            }
            current->next = arg;
        }
    }
    
    ezom_ast_node_t* message = ezom_ast_create_message_send(receiver, selector);
    message->data.message_send.arguments = arguments;
    
    return message;
}

// Statement parsing
ezom_ast_node_t* ezom_parse_statement(ezom_parser_t* parser) {
    ezom_parser_skip_newlines(parser);
    
    if (ezom_parser_check(parser, TOKEN_EOF) || 
        ezom_parser_check(parser, TOKEN_RPAREN) ||
        ezom_parser_check(parser, TOKEN_RBRACKET)) {
        return NULL;
    }
    
    // Check for return statement
    if (ezom_parser_match(parser, TOKEN_CARET)) {
        return ezom_parse_return_statement(parser);
    }
    
    // Parse expression
    ezom_ast_node_t* expr = ezom_parse_expression(parser);
    if (!expr) return NULL;
    
    // Check for assignment
    if (ezom_parser_match(parser, TOKEN_ASSIGN)) {
        ezom_ast_node_t* value = ezom_parse_expression(parser);
        if (!value) {
            ezom_ast_free(expr);
            return NULL;
        }
        
        // Consume optional dot after assignment
        ezom_parser_match(parser, TOKEN_DOT);
        
        return ezom_ast_create_assignment(expr, value);
    }
    
    // Consume optional dot
    ezom_parser_match(parser, TOKEN_DOT);
    
    return expr;
}

ezom_ast_node_t* ezom_parse_statement_list(ezom_parser_t* parser) {
    ezom_ast_node_t* statements = ezom_ast_create_statement_list();
    
    while (!ezom_parser_check(parser, TOKEN_EOF) && 
           !ezom_parser_check(parser, TOKEN_RPAREN) &&
           !ezom_parser_check(parser, TOKEN_RBRACKET)) {
        
        ezom_ast_node_t* stmt = ezom_parse_statement(parser);
        if (stmt) {
            ezom_ast_add_statement(statements, stmt);
        } else {
            break;
        }
        
        ezom_parser_skip_newlines(parser);
    }
    
    return statements;
}

ezom_ast_node_t* ezom_parse_return_statement(ezom_parser_t* parser) {
    ezom_ast_node_t* expr = ezom_parse_expression(parser);
    return ezom_ast_create_return(expr);
}

// Block parsing
ezom_ast_node_t* ezom_parse_block(ezom_parser_t* parser) {
    ezom_parser_consume(parser, TOKEN_LBRACKET, "Expected '['");
    ezom_parser_skip_newlines(parser);
    
    ezom_ast_node_t* block = ezom_ast_create_block();
    
    // Parse block parameters :param1 :param2 |
    if (ezom_parser_check(parser, TOKEN_COLON)) {
        block->data.block.parameters = ezom_parse_block_parameters(parser);
        ezom_parser_consume(parser, TOKEN_PIPE, "Expected '|' after block parameters");
        ezom_parser_skip_newlines(parser);
    }
    
    // Parse block body
    block->data.block.body = ezom_parse_statement_list(parser);
    
    ezom_parser_consume(parser, TOKEN_RBRACKET, "Expected ']' after block");
    
    return block;
}

ezom_ast_node_t* ezom_parse_block_parameters(ezom_parser_t* parser) {
    ezom_ast_node_t* params = ezom_ast_create_variable_list();
    
    while (ezom_parser_match(parser, TOKEN_COLON)) {
        if (!ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
            ezom_parser_error(parser, "Expected parameter name after ':'");
            break;
        }
        
        char* param_name = ezom_copy_current_token_text(parser);
        ezom_ast_add_variable(params, param_name);
        free(param_name);
        ezom_parser_advance(parser);
    }
    
    return params;
}

// Literal parsing
ezom_ast_node_t* ezom_parse_literal(ezom_parser_t* parser) {
    switch (parser->lexer->current_token.type) {
        case TOKEN_INTEGER:
            {
                int16_t value = parser->lexer->current_token.value.int_value;
                ezom_parser_advance(parser);
                return ezom_ast_create_literal_integer(value);
            }
            
        case TOKEN_STRING:
            {
                char* value = strdup(parser->lexer->current_token.value.string_value);
                ezom_parser_advance(parser);
                ezom_ast_node_t* node = ezom_ast_create_literal_string(value);
                free(value);
                return node;
            }
            
        case TOKEN_SYMBOL:
            {
                char* value = strdup(parser->lexer->current_token.value.string_value);
                ezom_parser_advance(parser);
                ezom_ast_node_t* node = ezom_ast_create_literal_symbol(value);
                free(value);
                return node;
            }
            
        default:
            ezom_parser_error(parser, "Expected literal value");
            return NULL;
    }
}

ezom_ast_node_t* ezom_parse_array_literal(ezom_parser_t* parser) {
    ezom_parser_consume(parser, TOKEN_HASH, "Expected '#'");
    ezom_parser_consume(parser, TOKEN_LPAREN, "Expected '(' after '#'");
    
    ezom_ast_node_t* array = ezom_ast_create(AST_LITERAL);
    array->data.literal.type = LITERAL_ARRAY;
    array->data.literal.value.array_elements = ezom_ast_create_statement_list();
    
    while (!ezom_parser_check(parser, TOKEN_RPAREN) && 
           !ezom_parser_check(parser, TOKEN_EOF)) {
        
        ezom_ast_node_t* element = ezom_parse_literal(parser);
        if (element) {
            ezom_ast_add_statement(array->data.literal.value.array_elements, element);
        }
    }
    
    ezom_parser_consume(parser, TOKEN_RPAREN, "Expected ')' after array elements");
    
    return array;
}

// Variable and parameter parsing
ezom_ast_node_t* ezom_parse_variable_list(ezom_parser_t* parser) {
    ezom_ast_node_t* vars = ezom_ast_create_variable_list();
    
    while (ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
        char* var_name = ezom_copy_current_token_text(parser);
        ezom_ast_add_variable(vars, var_name);
        free(var_name);
        ezom_parser_advance(parser);
    }
    
    return vars;
}

// Selector parsing
char* ezom_parse_unary_selector(ezom_parser_t* parser) {
    char* selector = ezom_copy_current_token_text(parser);
    ezom_parser_advance(parser);
    return selector;
}

char* ezom_parse_binary_selector(ezom_parser_t* parser) {
    char* selector = ezom_copy_current_token_text(parser);
    ezom_parser_advance(parser);
    return selector;
}

char* ezom_parse_keyword_selector(ezom_parser_t* parser) {
    char* selector = malloc(256);
    selector[0] = '\0';
    
    while (ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
        char* keyword = ezom_copy_current_token_text(parser);
        strcat(selector, keyword);
        free(keyword);
        ezom_parser_advance(parser);
        
        if (ezom_parser_match(parser, TOKEN_COLON)) {
            strcat(selector, ":");
        } else {
            break;
        }
    }
    
    return selector;
}

// Instance variable resolution functions
ezom_variable_type_t ezom_resolve_variable_type(const char* name, ezom_variable_context_t* context) {
    if (!name || !context) return VAR_UNKNOWN;
    
    // Check parameters first
    if (context->current_parameters && 
        ezom_find_parameter_index(name, context->current_parameters) >= 0) {
        return VAR_PARAMETER;
    }
    
    // Check local variables
    if (context->current_locals && 
        ezom_find_local_variable_index(name, context->current_locals) >= 0) {
        return VAR_LOCAL;
    }
    
    // Check instance variables
    if (context->class_def && 
        ezom_find_instance_variable_index(name, context->class_def) >= 0) {
        return VAR_INSTANCE;
    }
    
    return VAR_UNKNOWN;
}

int ezom_find_instance_variable_index(const char* name, ezom_ast_node_t* class_def) {
    if (!name || !class_def || !class_def->data.class_def.instance_vars) return -1;
    
    ezom_ast_node_t* vars = class_def->data.class_def.instance_vars;
    for (int i = 0; i < vars->data.variable_list.count; i++) {
        if (strcmp(vars->data.variable_list.names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

int ezom_find_local_variable_index(const char* name, ezom_ast_node_t* locals) {
    if (!name || !locals) return -1;
    
    for (int i = 0; i < locals->data.variable_list.count; i++) {
        if (strcmp(locals->data.variable_list.names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

int ezom_find_parameter_index(const char* name, ezom_ast_node_t* parameters) {
    if (!name || !parameters) return -1;
    
    for (int i = 0; i < parameters->data.variable_list.count; i++) {
        if (strcmp(parameters->data.variable_list.names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// Context-aware variable creation
ezom_ast_node_t* ezom_create_variable_with_context(const char* name, ezom_variable_context_t* context) {
    ezom_variable_type_t var_type = ezom_resolve_variable_type(name, context);
    
    ezom_ast_node_t* var_node = ezom_ast_create_variable(name);
    
    switch (var_type) {
        case VAR_INSTANCE:
            var_node->data.variable.is_instance_var = true;
            var_node->data.variable.index = ezom_find_instance_variable_index(name, context->class_def);
            break;
        case VAR_LOCAL:
            var_node->data.variable.is_local = true;
            var_node->data.variable.index = ezom_find_local_variable_index(name, context->current_locals);
            break;
        case VAR_PARAMETER:
            var_node->data.variable.is_local = false;
            var_node->data.variable.is_instance_var = false;
            var_node->data.variable.index = ezom_find_parameter_index(name, context->current_parameters);
            break;
        case VAR_UNKNOWN:
            // Default to instance variable if we can't determine
            var_node->data.variable.is_instance_var = false;
            var_node->data.variable.is_local = false;
            var_node->data.variable.index = 0;
            break;
    }
    
    return var_node;
}

// Utility functions
bool ezom_parser_match(ezom_parser_t* parser, ezom_token_type_t type) {
    if (ezom_parser_check(parser, type)) {
        ezom_parser_advance(parser);
        return true;
    }
    return false;
}

bool ezom_parser_check(ezom_parser_t* parser, ezom_token_type_t type) {
    return parser->lexer->current_token.type == type;
}

void ezom_parser_consume(ezom_parser_t* parser, ezom_token_type_t type, const char* error_msg) {
    if (parser->lexer->current_token.type == type) {
        ezom_parser_advance(parser);
    } else {
        ezom_parser_error(parser, error_msg);
    }
}

void ezom_parser_advance(ezom_parser_t* parser) {
    ezom_lexer_next_token(parser->lexer);
}

void ezom_parser_error(ezom_parser_t* parser, const char* message) {
    parser->has_error = true;
    parser->error_count++;
    snprintf(parser->error_message, sizeof(parser->error_message), 
             "Line %d, Column %d: %s", 
             parser->lexer->current_token.line, 
             parser->lexer->current_token.column, 
             message);
    printf("Parse error: %s\n", parser->error_message);
}

void ezom_parser_synchronize(ezom_parser_t* parser) {
    // Skip tokens until we find a safe point to continue parsing
    while (!ezom_parser_check(parser, TOKEN_EOF)) {
        // Look for tokens that might indicate the start of a new construct
        if (ezom_parser_check(parser, TOKEN_IDENTIFIER) ||
            ezom_parser_check(parser, TOKEN_SEPARATOR) ||
            ezom_parser_check(parser, TOKEN_RPAREN) ||
            ezom_parser_check(parser, TOKEN_RBRACKET) ||
            ezom_parser_check(parser, TOKEN_NEWLINE)) {
            return;
        }
        ezom_parser_advance(parser);
    }
}

static void ezom_parser_skip_newlines(ezom_parser_t* parser) {
    while (ezom_parser_match(parser, TOKEN_NEWLINE)) {
        // Skip newlines
    }
}

// Helper functions
bool ezom_is_binary_operator(ezom_token_type_t type) {
    switch (type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_EQUALS:
            return true;
        default:
            return false;
    }
}

bool ezom_is_keyword_start(ezom_parser_t* parser) {
    if (parser->lexer->current_token.type != TOKEN_IDENTIFIER) {
        return false;
    }
    
    // Look ahead to see if there's a colon
    ezom_lexer_t temp_lexer = *parser->lexer;
    ezom_lexer_next_token(&temp_lexer);
    return temp_lexer.current_token.type == TOKEN_COLON;
}

char* ezom_copy_current_token_text(ezom_parser_t* parser) {
    ezom_token_t* token = &parser->lexer->current_token;
    if (token->type == TOKEN_STRING || token->type == TOKEN_IDENTIFIER || token->type == TOKEN_SYMBOL) {
        return strdup(token->value.string_value);
    } else if (token->length > 0) {
        return strndup(token->text, token->length);
    } else {
        // For single character tokens
        char* result = malloc(2);
        result[0] = token->text ? token->text[0] : '?';
        result[1] = '\0';
        return result;
    }
}

char* ezom_parse_keyword_method_signature(ezom_parser_t* parser, ezom_ast_node_t** parameters) {
    char* selector = malloc(256);
    selector[0] = '\0';
    
    *parameters = ezom_ast_create_variable_list();
    
    while (ezom_parser_check(parser, TOKEN_IDENTIFIER) && ezom_is_keyword_start(parser)) {
        char* keyword = ezom_copy_current_token_text(parser);
        strcat(selector, keyword);
        free(keyword);
        ezom_parser_advance(parser);
        
        if (ezom_parser_match(parser, TOKEN_COLON)) {
            strcat(selector, ":");
            
            // Parse parameter name
            if (ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
                char* param_name = ezom_copy_current_token_text(parser);
                ezom_ast_add_variable(*parameters, param_name);
                free(param_name);
                ezom_parser_advance(parser);
            } else {
                ezom_parser_error(parser, "Expected parameter name after ':'");
                break;
            }
        } else {
            break;
        }
    }
    
    return selector;
}

char* ezom_parse_binary_method_signature(ezom_parser_t* parser, ezom_ast_node_t** parameters) {
    char* selector = ezom_parse_binary_selector(parser);
    
    *parameters = ezom_ast_create_variable_list();
    
    // Parse parameter name for binary method
    if (ezom_parser_check(parser, TOKEN_IDENTIFIER)) {
        char* param_name = ezom_copy_current_token_text(parser);
        ezom_ast_add_variable(*parameters, param_name);
        free(param_name);
        ezom_parser_advance(parser);
    } else {
        ezom_parser_error(parser, "Expected parameter name after binary operator");
    }
    
    return selector;
}