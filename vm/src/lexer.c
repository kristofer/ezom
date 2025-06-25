// ============================================================================
// File: src/lexer.c
// Lexical analyzer implementation
// ============================================================================

#include "../include/ezom_lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Forward declarations
static char ezom_lexer_advance(ezom_lexer_t* lexer);
static char ezom_lexer_peek(ezom_lexer_t* lexer);
static char ezom_lexer_peek_next(ezom_lexer_t* lexer);
static bool ezom_lexer_is_at_end(ezom_lexer_t* lexer);
static void ezom_lexer_skip_whitespace(ezom_lexer_t* lexer);
static void ezom_lexer_read_string(ezom_lexer_t* lexer);
static void ezom_lexer_read_symbol(ezom_lexer_t* lexer);
static void ezom_lexer_read_comment(ezom_lexer_t* lexer);
static void ezom_lexer_read_identifier(ezom_lexer_t* lexer);
static void ezom_lexer_read_integer(ezom_lexer_t* lexer);
static void ezom_lexer_read_separator(ezom_lexer_t* lexer);
static void ezom_lexer_make_token(ezom_lexer_t* lexer, ezom_token_type_t type);

void ezom_lexer_init(ezom_lexer_t* lexer, char* source) {
    lexer->source = source;
    lexer->current = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->has_error = false;
    lexer->error_message[0] = '\0';
    
    // Read first token
    ezom_lexer_next_token(lexer);
}

void ezom_lexer_next_token(ezom_lexer_t* lexer) {
    ezom_lexer_skip_whitespace(lexer);
    
    if (ezom_lexer_is_at_end(lexer)) {
        ezom_lexer_make_token(lexer, TOKEN_EOF);
        return;
    }
    
    char ch = ezom_lexer_advance(lexer);
    
    switch (ch) {
        case '(': ezom_lexer_make_token(lexer, TOKEN_LPAREN); break;
        case ')': ezom_lexer_make_token(lexer, TOKEN_RPAREN); break;
        case '[': ezom_lexer_make_token(lexer, TOKEN_LBRACKET); break;
        case ']': ezom_lexer_make_token(lexer, TOKEN_RBRACKET); break;
        case '|': ezom_lexer_make_token(lexer, TOKEN_PIPE); break;
        case '.': ezom_lexer_make_token(lexer, TOKEN_DOT); break;
        case '^': ezom_lexer_make_token(lexer, TOKEN_CARET); break;
        case '+': ezom_lexer_make_token(lexer, TOKEN_PLUS); break;
        case '*': ezom_lexer_make_token(lexer, TOKEN_MULTIPLY); break;
        case '/': ezom_lexer_make_token(lexer, TOKEN_DIVIDE); break;
        case '<': ezom_lexer_make_token(lexer, TOKEN_LT); break;
        case '>': ezom_lexer_make_token(lexer, TOKEN_GT); break;
        case '\n': 
            lexer->line++;
            lexer->column = 1;
            ezom_lexer_make_token(lexer, TOKEN_NEWLINE);
            break;
            
        case ':':
            if (ezom_lexer_peek(lexer) == '=') {
                ezom_lexer_advance(lexer);
                ezom_lexer_make_token(lexer, TOKEN_ASSIGN);
            } else {
                ezom_lexer_make_token(lexer, TOKEN_COLON);
            }
            break;
            
        case '=':
            ezom_lexer_make_token(lexer, TOKEN_EQUALS);
            break;
            
        case '-':
            // Check for separator ----
            if (ezom_lexer_peek(lexer) == '-' && 
                ezom_lexer_peek_next(lexer) == '-') {
                ezom_lexer_read_separator(lexer);
            } else if (isdigit(ezom_lexer_peek(lexer))) {
                // Negative number
                ezom_lexer_read_integer(lexer);
            } else {
                ezom_lexer_make_token(lexer, TOKEN_MINUS);
            }
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
            } else if (isdigit(ch)) {
                ezom_lexer_read_integer(lexer);
            } else {
                ezom_lexer_error(lexer, "Unexpected character");
                ezom_lexer_make_token(lexer, TOKEN_ERROR);
            }
            break;
    }
}

bool ezom_lexer_match(ezom_lexer_t* lexer, ezom_token_type_t type) {
    if (ezom_lexer_check(lexer, type)) {
        ezom_lexer_next_token(lexer);
        return true;
    }
    return false;
}

bool ezom_lexer_check(ezom_lexer_t* lexer, ezom_token_type_t type) {
    return lexer->current_token.type == type;
}

void ezom_lexer_consume(ezom_lexer_t* lexer, ezom_token_type_t type, const char* error_msg) {
    if (lexer->current_token.type == type) {
        ezom_lexer_next_token(lexer);
    } else {
        ezom_lexer_error(lexer, error_msg);
    }
}

void ezom_lexer_error(ezom_lexer_t* lexer, const char* message) {
    lexer->has_error = true;
    snprintf(lexer->error_message, sizeof(lexer->error_message), 
             "Line %d, Column %d: %s", lexer->line, lexer->column, message);
}

// Private functions

static char ezom_lexer_advance(ezom_lexer_t* lexer) {
    lexer->position++;
    lexer->column++;
    return *lexer->current++;
}

static char ezom_lexer_peek(ezom_lexer_t* lexer) {
    return *lexer->current;
}

static char ezom_lexer_peek_next(ezom_lexer_t* lexer) {
    if (ezom_lexer_is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

static bool ezom_lexer_is_at_end(ezom_lexer_t* lexer) {
    return *lexer->current == '\0';
}

static void ezom_lexer_skip_whitespace(ezom_lexer_t* lexer) {
    while (true) {
        char ch = ezom_lexer_peek(lexer);
        switch (ch) {
            case ' ':
            case '\r':
            case '\t':
                ezom_lexer_advance(lexer);
                break;
            default:
                return;
        }
    }
}

static void ezom_lexer_read_string(ezom_lexer_t* lexer) {
    char* start = lexer->current;
    
    while (ezom_lexer_peek(lexer) != '\'' && !ezom_lexer_is_at_end(lexer)) {
        if (ezom_lexer_peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
        ezom_lexer_advance(lexer);
    }
    
    if (ezom_lexer_is_at_end(lexer)) {
        ezom_lexer_error(lexer, "Unterminated string");
        ezom_lexer_make_token(lexer, TOKEN_ERROR);
        return;
    }
    
    // Consume closing '
    ezom_lexer_advance(lexer);
    
    // Create string token
    uint16_t length = lexer->current - start - 1;
    lexer->current_token.type = TOKEN_STRING;
    lexer->current_token.text = start;
    lexer->current_token.length = length;
    lexer->current_token.value.string_value = strndup(start, length);
}

static void ezom_lexer_read_symbol(ezom_lexer_t* lexer) {
    char* start = lexer->current;
    
    // Skip opening #
    if (ezom_lexer_peek(lexer) == '(') {
        // Array literal #(...)
        ezom_lexer_advance(lexer); // consume (
        ezom_lexer_make_token(lexer, TOKEN_HASH);
        return;
    }
    
    // Read symbol name
    while (isalnum(ezom_lexer_peek(lexer)) || ezom_lexer_peek(lexer) == '_') {
        ezom_lexer_advance(lexer);
    }
    
    uint16_t length = lexer->current - start;
    lexer->current_token.type = TOKEN_SYMBOL;
    lexer->current_token.text = start;
    lexer->current_token.length = length;
    lexer->current_token.value.string_value = strndup(start, length);
}

static void ezom_lexer_read_comment(ezom_lexer_t* lexer) {
    char* start = lexer->current;
    
    while (ezom_lexer_peek(lexer) != '"' && !ezom_lexer_is_at_end(lexer)) {
        if (ezom_lexer_peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
        ezom_lexer_advance(lexer);
    }
    
    if (ezom_lexer_is_at_end(lexer)) {
        ezom_lexer_error(lexer, "Unterminated comment");
        ezom_lexer_make_token(lexer, TOKEN_ERROR);
        return;
    }
    
    // Consume closing "
    ezom_lexer_advance(lexer);
    
    // Skip comments - read next token
    ezom_lexer_next_token(lexer);
}

static void ezom_lexer_read_identifier(ezom_lexer_t* lexer) {
    char* start = lexer->current - 1; // Back up to include first character
    
    while (isalnum(ezom_lexer_peek(lexer)) || ezom_lexer_peek(lexer) == '_') {
        ezom_lexer_advance(lexer);
    }
    
    uint16_t length = lexer->current - start;
    lexer->current_token.type = TOKEN_IDENTIFIER;
    lexer->current_token.text = start;
    lexer->current_token.length = length;
    lexer->current_token.value.string_value = strndup(start, length);
}

static void ezom_lexer_read_integer(ezom_lexer_t* lexer) {
    char* start = lexer->current - 1; // Back up to include first character
    bool negative = (*start == '-');
    
    while (isdigit(ezom_lexer_peek(lexer))) {
        ezom_lexer_advance(lexer);
    }
    
    uint16_t length = lexer->current - start;
    lexer->current_token.type = TOKEN_INTEGER;
    lexer->current_token.text = start;
    lexer->current_token.length = length;
    
    // Parse integer value
    char* temp = strndup(start, length);
    lexer->current_token.value.int_value = (int16_t)atoi(temp);
    free(temp);
}

static void ezom_lexer_read_separator(ezom_lexer_t* lexer) {
    // We already consumed one -, consume the rest
    while (ezom_lexer_peek(lexer) == '-') {
        ezom_lexer_advance(lexer);
    }
    
    ezom_lexer_make_token(lexer, TOKEN_SEPARATOR);
}

static void ezom_lexer_make_token(ezom_lexer_t* lexer, ezom_token_type_t type) {
    lexer->current_token.type = type;
    lexer->current_token.line = lexer->line;
    lexer->current_token.column = lexer->column;
    lexer->current_token.text = NULL;
    lexer->current_token.length = 0;
}

// Utility functions

const char* ezom_token_type_name(ezom_token_type_t type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_SYMBOL: return "SYMBOL";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_PIPE: return "PIPE";
        case TOKEN_DOT: return "DOT";
        case TOKEN_CARET: return "CARET";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_COLON: return "COLON";
        case TOKEN_SEPARATOR: return "SEPARATOR";
        case TOKEN_EQUALS: return "EQUALS";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_HASH: return "HASH";
        case TOKEN_QUOTE: return "QUOTE";
        case TOKEN_NEWLINE: return "NEWLINE";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void ezom_token_print(ezom_token_t* token) {
    printf("Token: %s at %d:%d", 
           ezom_token_type_name(token->type), 
           token->line, 
           token->column);
    
    if (token->length > 0) {
        printf(" text='%.*s'", token->length, token->text);
    }
    
    if (token->type == TOKEN_INTEGER) {
        printf(" value=%d", token->value.int_value);
    } else if (token->type == TOKEN_STRING || token->type == TOKEN_IDENTIFIER || token->type == TOKEN_SYMBOL) {
        if (token->value.string_value) {
            printf(" value='%s'", token->value.string_value);
        }
    }
    
    printf("\n");
}