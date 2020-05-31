/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <stddef.h>

#include "type.h"

// type of source code
typedef enum SourceType SourceType;
enum SourceType
{
    SRC_FILE, // file
    SRC_CMD,  // command line
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

// structure for token
struct Token
{
    TokenKind kind; // kind of token
    char *str;      // pointer to token string
    size_t len;     // length of token string
    TypeKind type;  // type of token (only for TK_CONST)
    long value;     // value of token (only for TK_CONST)
};

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

#endif /* !__TOKENIZER_H__ */
