#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>

#include "type.h"

// type of source code
typedef enum SourceType SourceType;
enum SourceType
{
    SRC_FILE, // file
    SRC_CMD,  // command line
};

// kind of constant
typedef enum ConstantKind ConstantKind;
enum ConstantKind
{
    CN_INT,     // integer constant
    CN_FLOAT,   // floating constant
    CN_ENUM,    // enumeration constant
    CN_CHAR,    // character constant
};

// kind of token
typedef enum TokenKind TokenKind;
enum TokenKind
{
    TK_RESERVED, // reserved token
    TK_IDENT,    // identifier
    TK_CONST,    // constant
    TK_STR,      // string-literal
};

// structure for constant
struct Constant
{
    ConstantKind kind;               // kind of constant
    const Type *type;                // type of constant
    union
    {
        long int_value;              // value of integer constant (only for CN_INT, CN_ENUM, CN_CHAR)
        struct
        {
            double float_value;      // value of floating constant (only for CN_FLOAT)
            const char *float_label; // label for floating constant (only for CN_FLOAT)
        };
    };
};

// structure for token
struct Token
{
    TokenKind kind;   // kind of token
    char *str;        // pointer to token string
    size_t len;       // length of token string
    Constant *value;  // value of token (only for TK_CONST)
};

Constant *new_constant(const Constant *value);
bool peek_reserved(const char *str);
bool peek_token(TokenKind kind, Token **token);
bool consume_reserved(const char *str);
bool consume_token(TokenKind kind, Token **token);
Token *get_token(void);
void set_token(Token *token);
void expect_reserved(const char *str);
Token *expect_identifier(void);
Token *expect_constant(void);
void tokenize(char *str);
bool at_eof(void);
char *make_identifier(const Token *token);
void set_source(SourceType type);
char *read_file(const char *path);
void report_warning(const char *loc, const char *fmt, ...);
void report_error(const char *loc, const char *fmt, ...);

#endif /* !TOKENIZER_H */
