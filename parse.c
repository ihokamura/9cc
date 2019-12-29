/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser (tokenizer)
*/

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// function prototype
static Token *new_token(TokenKind kind, Token *cur_tok, char *str, int len);
static int is_operator(const char *str);
static int is_keyword(const char *str, TokenKind *kind);
static int is_ident(const char *str);


// global variable
static const char *keyword_string_list[] = {
    "return",
    "if",
    "else",
    "while",
    "for",
};
static const TokenKind keyword_kind_list[] = {
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
};
static const char *operator_string_list[] = {
    "==",
    "!=",
    "<=",
    ">=",
    "+",
    "-",
    "*",
    "/",
    "(",
    ")",
    "<",
    ">",
    "=",
    "{",
    "}",
    ";",
    ",",
};
static char *user_input; // input of compiler
static Token *token; // currently parsing token


/*
consume an operator
* If the next token is a given operator, this function parses the token and returns true.
* Otherwise, it returns false.
*/
bool consume_operator(const char *op)
{
    if(
        (token->kind != TK_RESERVED) || 
        (token->len != strlen(op)) || 
        (strncmp(token->str, op, token->len) != 0)
        )
    {
        return false;
    }

    token = token->next;

    return true;
}


/*
consume a keyword
* If the next token is a keyword, this function parses the keyword and returns true.
* Otherwise, it returns false.
*/
bool consume_keyword(TokenKind kind)
{
    if(token->kind != kind)
    {
        return false;
    }

    token = token->next;

    return true;
}


/*
consume an identifier
* If the next token is an identifier, this function parses the identifier and returns the token.
* Otherwise, it returns NULL.
*/
Token *consume_ident(void)
{
    if(token->kind != TK_IDENT)
    {
        return NULL;
    }

    Token *ident_tok = token;

    token = token->next;

    return ident_tok;
}


/*
parse an operator
* If the next token is a given operator, this function parses the token.
* Otherwise, it reports an error.
*/
void expect_operator(const char *op)
{
    if(
        (token->kind != TK_RESERVED) || 
        (token->len != strlen(op)) || 
        (strncmp(token->str, op, token->len) != 0)
        )
    {
        report_error(token->str, "not '%s'.", op);
    }

    token = token->next;
}


/*
parse a number
* If the next token is a number, this function parses the token and returns the value.
* Otherwise, it reports an error.
*/
int expect_number(void)
{
    if(token->kind != TK_NUM)
    {
        report_error(token->str, "not a number.");
    }

    int val = token->val;

    token = token->next;

    return val;
}


/*
tokenize a given string
*/
void tokenize(char *str)
{
    // save input
    user_input = str;

    // initialize token stream
    Token head;
    head.next = NULL;
    Token *current = &head;

    while(*str)
    {
        // ignore space
        if(isspace(*str))
        {
            str++;
            continue;
        }

        // parse an operator
        int len = is_operator(str);
        if(len > 0)
        {
            current = new_token(TK_RESERVED, current, str, len);
            str += len;
            continue;
        }

        // parse keyword
        TokenKind tok_kind;
        len = is_keyword(str, &tok_kind);
        if(len > 0)
        {
            current = new_token(tok_kind, current, str, len);
            str += len;
            continue;
        }

        // parse an identifer
        len = is_ident(str);
        if(len > 0)
        {
            current = new_token(TK_IDENT, current, str, len);
            str += len;
            continue;
        }

        // parse a number
        if(isdigit(*str))
        {
            current = new_token(TK_NUM, current, str, 0);
            current->val = strtol(str, &str, 10);
            continue;
        }

        // Other characters are not accepted as a token.
        report_error(token->str, "cannot tokenize.");
    }

    // end token stream
    new_token(TK_EOF, current, str, 0);
    token = head.next;
}


/*
check if the current token is the end of input
*/
bool at_eof(void)
{
    return (token->kind == TK_EOF);
}


/*
report an error
* This function never returns.
*/
void report_error(char *loc, const char *fmt, ...)
{
    if(loc != NULL)
    {
        // emphasize the position where an error is detected
        int pos = (loc - user_input) / sizeof(user_input[0]);
        fprintf(stderr, "%s\n", user_input);
        fprintf(stderr, "%*s ^", pos, "");
    }

    // print the error message
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    exit(1);
}


/*
make a new token and concatenate it to the current token
*/
static Token *new_token(TokenKind kind, Token *cur_tok, char *str, int len)
{
    Token *new_tok = calloc(1, sizeof(Token));

    new_tok->kind = kind;
    new_tok->str = str;
    new_tok->len = len;
    cur_tok->next = new_tok;

    return new_tok;
}


/*
check if the following string is an operator
*/
static int is_operator(const char *str)
{
    for(int i = 0; i < sizeof(operator_string_list) / sizeof(operator_string_list[0]); i++)
    {
        const char *op = operator_string_list[i];
        size_t len = strlen(op);
        if(strncmp(str, op, len) == 0)
        {
            return len;
        }
    }

    return 0;
}


/*
check if the following string is a keyword
*/
static int is_keyword(const char *str, TokenKind *kind)
{
    for(int i = 0; i < sizeof(keyword_string_list) / sizeof(keyword_string_list[0]); i++)
    {
        // parse keyword
        const char *keyword = keyword_string_list[i];
        size_t len = strlen(keyword);
        if((strncmp(str, keyword, len) == 0) && (!isalnum(str[len]) && (str[len] != '_')))
        {
            *kind = keyword_kind_list[i];
            return len;
        }
    }

    return 0;
}


/*
check if the following string is an identifier
*/
static int is_ident(const char *str)
{
    int len = 0;

    if(isalpha(*str) || (*str == '_'))
    {
        len++;
        str++;

        // there may be a digit after second character
        while(isalnum(*str) || (*str == '_'))
        {
            len++;
            str++;
        }
    }

    return len;
}