/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* tokenizer (user input parser)
*/

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// function prototype
static int is_reserved(const char *str);
static int is_ident(const char *str);
static int is_str(const char *str);
static Token *new_token(TokenKind kind, Token *cur_tok, char *str, int len);


// global variable
// list of punctuators
// It is necessary to put longer strings above than shorter strings.
static const char *punctuator_list[] = {
    "==",
    "!=",
    "<=",
    ">=",
    "[",
    "]",
    "(",
    ")",
    "{",
    "}",
    "&",
    "*",
    "+",
    "-",
    "/",
    "<",
    ">",
    ";",
    "=",
    ",",
};
static const size_t PUNCTUATOR_LIST_SIZE = sizeof(punctuator_list) / sizeof(punctuator_list[0]); // number of punctuators
// list of keywords
static const char *keyword_list[] = {
    "char",
    "do",
    "else",
    "for",
    "if",
    "int",
    "return",
    "sizeof",
    "while",
};
static const size_t KEYWORD_LIST_SIZE = sizeof(keyword_list) / sizeof(keyword_list[0]); // number of keywords
static char *user_input; // input of compiler
static Token *current_token; // currently parsing token


/*
peek a reserved string
* If the next token is a given string, this returns true.
* Otherwise, it returns false.
*/
bool peek_reserved(const char *str)
{
    return (
           (current_token->kind == TK_RESERVED)
        && (current_token->len == strlen(str))
        && (strncmp(current_token->str, str, current_token->len) == 0));
}


/*
consume a reserved string
* If the next token is a given string, this function parses the token and returns true.
* Otherwise, it returns false.
*/
bool consume_reserved(const char *str)
{
    if(!peek_reserved(str))
    {
        return false;
    }

    current_token = current_token->next;

    return true;
}


/*
consume a token
* If the next token is a given kind of token, this function parses and returns the token by argument and true by value.
* Otherwise, it returns false.
*/
bool consume_token(TokenKind kind, Token **token)
{
    if(current_token->kind != kind)
    {
        *token = NULL;
        return false;
    }

    *token = current_token;
    current_token = current_token->next;

    return true;
}


/*
parse a reserved string
* If the next token is a given string, this function parses the token.
* Otherwise, it reports an error.
*/
void expect_reserved(const char *str)
{
    if(!consume_reserved(str))
    {
        report_error(current_token->str, "expected '%s'.", str);
    }
}


/*
parse a number
* If the next token is a number, this function parses the token and returns the value.
* Otherwise, it reports an error.
*/
int expect_number(void)
{
    if(current_token->kind != TK_NUM)
    {
        report_error(current_token->str, "expected a number.");
    }

    int val = current_token->val;
    current_token = current_token->next;

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
    Token head = {};
    Token *cursor = &head;

    while(*str)
    {
        int len;

        // ignore space
        if(isspace(*str))
        {
            str++;
            continue;
        }

        // parse a reserved string
        len = is_reserved(str);
        if(len > 0)
        {
            cursor = new_token(TK_RESERVED, cursor, str, len);
            str += len;
            continue;
        }

        // parse an identifer
        len = is_ident(str);
        if(len > 0)
        {
            cursor = new_token(TK_IDENT, cursor, str, len);
            str += len;
            continue;
        }

        // parse a string-literal
        len = is_str(str);
        if(len > 0)
        {
            cursor = new_token(TK_STR, cursor, str + 1, len - 2);
            str += len;
            continue;
        }

        // parse a number
        if(isdigit(*str))
        {
            cursor = new_token(TK_NUM, cursor, str, 0);
            cursor->val = strtol(str, &str, 10);
            continue;
        }

        // Other characters are not accepted as a token.
        report_error(current_token->str, "cannot tokenize.");
    }

    // end token stream
    new_token(TK_EOF, cursor, str, 0);
    current_token = head.next;
}


/*
check if the current token is the end of input
*/
bool at_eof(void)
{
    return (current_token->kind == TK_EOF);
}


/*
make string of an identifier
*/
char *make_ident(const Token *token)
{
    char *ident = calloc(token->len + 1, sizeof(char));

    strncpy(ident, token->str, token->len);

    return ident;
}


/*
report a warning
*/
void report_warning(const char *loc, const char *fmt, ...)
{
    if(loc != NULL)
    {
        // emphasize the position where a warning is detected
        int pos = (loc - user_input) / sizeof(user_input[0]);
        fprintf(stderr, "%s\n", user_input);
        fprintf(stderr, "%*s ^\n", pos, "");
    }

    // print the warning message
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}


/*
report an error
* This function never returns.
*/
void report_error(const char *loc, const char *fmt, ...)
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
check if the following string is reserved
*/
static int is_reserved(const char *str)
{
    // check punctuators
    for(size_t i = 0; i < PUNCTUATOR_LIST_SIZE; i++)
    {
        const char *punc = punctuator_list[i];
        size_t len = strlen(punc);
        if(strncmp(str, punc, len) == 0)
        {
            return len;
        }
    }

    // check keywords
    for(size_t i = 0; i < KEYWORD_LIST_SIZE; i++)
    {
        const char *kw = keyword_list[i];
        size_t len = strlen(kw);
        if((strncmp(str, kw, len) == 0) && (!isalnum(str[len]) && (str[len] != '_')))
        {
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


/*
check if the following string is a string-literal
*/
static int is_str(const char *str)
{
    int len = 0;

    if(*str == '"')
    {
        len++;
        while((str[len] != '\0') && (str[len] != '"'))
        {
            len++;
        }

        if(str[len] == '"')
        {
            len++;
        }
        else
        {
            report_error(str, "expected '\"'");
        }
    }

    return len;
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