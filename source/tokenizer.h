/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <stddef.h>

#include "9cc.h"

bool peek_reserved(const char *str);
bool peek_token(TokenKind kind, Token **token);
bool consume_reserved(const char *str);
bool consume_token(TokenKind kind, Token **token);
Token *get_token(void);
void set_token(Token *token);
void expect_reserved(const char *str);
Token *expect_identifier(void);
Token *expect_integer_constant(void);
void tokenize(char *str);
bool at_eof(void);
char *make_identifier(const Token *token);
void set_source(SourceType type);
char *read_file(const char *path);
void report_warning(const char *loc, const char *fmt, ...);
void report_error(const char *loc, const char *fmt, ...);

#endif /* !__TOKENIZER_H__ */
