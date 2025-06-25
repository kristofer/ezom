// ============================================================================
// File: include/ezom_lexer.h
// Lexical analyzer for EZOM language
// ============================================================================

#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_SYMBOL,
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    TOKEN_PIPE,         // |
    TOKEN_DOT,          // .
    TOKEN_CARET,        // ^
    TOKEN_ASSIGN,       // :=
    TOKEN_COLON,        // :
    TOKEN_SEPARATOR,    // ----
    TOKEN_EQUALS,       // =
    TOKEN_COMMENT,      // " ... "
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLY,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_LT,           // <
    TOKEN_GT,           // >
    TOKEN_HASH,         // #
    TOKEN_QUOTE,        // '
    TOKEN_NEWLINE,
    TOKEN_ERROR
} ezom_token_type_t;

typedef struct ezom_token {
    ezom_token_type_t type;
    uint16_t         line;
    uint16_t         column;
    uint16_t         length;
    char*            text;
    union {
        int16_t      int_value;    // For TOKEN_INTEGER
        char*        string_value; // For TOKEN_STRING, TOKEN_IDENTIFIER
    } value;
} ezom_token_t;

typedef struct ezom_lexer {
    char*    source;
    char*    current;
    uint16_t position;
    uint16_t line;
    uint16_t column;
    ezom_token_t current_token;
    bool     has_error;
    char     error_message[128];
} ezom_lexer_t;

// Lexer functions
void ezom_lexer_init(ezom_lexer_t* lexer, char* source);
void ezom_lexer_next_token(ezom_lexer_t* lexer);
bool ezom_lexer_match(ezom_lexer_t* lexer, ezom_token_type_t type);
bool ezom_lexer_check(ezom_lexer_t* lexer, ezom_token_type_t type);
void ezom_lexer_consume(ezom_lexer_t* lexer, ezom_token_type_t type, const char* error_msg);
void ezom_lexer_error(ezom_lexer_t* lexer, const char* message);

// Utility functions
const char* ezom_token_type_name(ezom_token_type_t type);
void ezom_token_print(ezom_token_t* token);