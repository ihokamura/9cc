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
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// function prototype
static int is_space(const char *str);
static int is_comment(const char *str);
static int is_reserved(const char *str);
static int is_ident(const char *str);
static int is_str(const char *str);
static Token *new_token(TokenKind kind, Token *cur_tok, char *str, int len);
static void report_position(const char *loc);


// global variable
// list of punctuators
// It is necessary to put longer strings above than shorter strings.
static const char *punctuator_list[] = {
    "==",
    "!=",
    "<=",
    ">=",
    "+=",
    "-=",
    "*=",
    "/=",
    "++",
    "--",
    "<<",
    ">>",
    "&&",
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
    "%",
    "<",
    ">",
    "^",
    "|",
    ":",
    ";",
    "=",
    ",",
};
static const size_t PUNCTUATOR_LIST_SIZE = sizeof(punctuator_list) / sizeof(punctuator_list[0]); // number of punctuators
// list of keywords
static const char *keyword_list[] = {
    "break",
    "case",
    "char",
    "continue",
    "default",
    "do",
    "else",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "return",
    "sizeof",
    "short",
    "switch",
    "void",
    "while",
};
static const size_t KEYWORD_LIST_SIZE = sizeof(keyword_list) / sizeof(keyword_list[0]); // number of keywords
static char *user_input; // input of compiler
static Token *current_token; // currently parsing token
static int source_type; // type of source
static const char *file_name; // name of source file


/*
peek a reserved string
* If the next token is a given string, this function returns true.
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
peek a token
* If the next token is a given kind of token, this function returns the token by argument and true by value.
* Otherwise, it returns false.
*/
bool peek_token(TokenKind kind, Token **token)
{
    if(current_token->kind == kind)
    {
        *token = current_token;
        return true;
    }
    else
    {
        *token = NULL;
        return false;
    }
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
    if(!peek_token(kind, token))
    {
        return false;
    }

    current_token = current_token->next;

    return true;
}


/*
resume a token
*/
void resume_token(void)
{
    current_token = current_token->prev;
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
parse an identifier
* If the next token is an identifier, this function parses the token and returns the token.
* Otherwise, it reports an error.
*/
Token *expect_ident(void)
{
    if(current_token->kind != TK_IDENT)
    {
        report_error(current_token->str, "expected an identifier.");
    }

    Token *token = current_token;
    current_token = current_token->next;

    return token;
}


/*
parse a number
* If the next token is a number, this function parses the token and returns the value.
* Otherwise, it reports an error.
*/
long expect_number(void)
{
    if(current_token->kind != TK_NUM)
    {
        report_error(current_token->str, "expected a number.");
    }

    long val = current_token->val;
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
        len = is_space(str);
        if(len > 0)
        {
            str += len;
            continue;
        }

        // ignore comment
        len = is_comment(str);
        if(len > 0)
        {
            str += len;
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
set type of source code
*/
void set_source(SourceType type)
{
    source_type = type;
}


/*
read source code from a file
*/
char *read_file(const char *path)
{
    // open source file
    FILE *stream = fopen(path, "r");
    if(stream == NULL)
    {
        fprintf(stderr, "cannot open %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // get length of source code
    if(fseek(stream, 0, SEEK_END) == -1)
    {
        fprintf(stderr, "%s: fseek: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    size_t size = ftell(stream);
    if(fseek(stream, 0, SEEK_SET) == -1)
    {
        fprintf(stderr, "%s: fseek: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // read source code and make it end with "\n\0"
    char *buffer = calloc(size + 2, sizeof(char));
    fread(buffer, size, 1, stream);
    if((size == 0) || (buffer[size - 1] != '\n'))
    {
        buffer[size] = '\n';
        buffer[size + 1] = '\0';
    }
    else
    {
        buffer[size] = '\0';
    }

    // save name of source file
    file_name = path;

    return buffer;
}


/*
report a warning
*/
void report_warning(const char *loc, const char *fmt, ...)
{
    if(loc != NULL)
    {
        // report the position where a warning is detected
        report_position(loc);
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
        // report the position where an error is detected
        report_position(loc);
    }

    // print the error message
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    exit(1);
}


/*
check if the following string is space
*/
static int is_space(const char *str)
{
    int len = 0;

    while(isspace(str[len]))
    {
        len++;
    }

    return len;
}


/*
check if the following string is comment
*/
static int is_comment(const char *str)
{
    int len = 0;

    if(strncmp(str, "//", 2) == 0)
    {
        // line comment
        len = 2;
        while(str[len] != '\n')
        {
            len++;
        }
    }
    else if(strncmp(str, "/*", 2) == 0)
    {
        // block comment
        const char *start = &str[2];
        char *end = strstr(start, "*/");
        if(end == NULL)
        {
            report_error(str, "block comment is not closed with \"*/\".\n");
        }

        len = (end - start) + 4; // add 4 to count "/*" and "*/"
    }

    return len;
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
    new_tok->prev = cur_tok;
    cur_tok->next = new_tok;

    return new_tok;
}


/*
report the position at which a warning or an error is detected
*/
static void report_position(const char *loc)
{
    int pos;

    if(source_type == SRC_FILE)
    {
        // search the first and last character of the line including the given location
        int start_pos = 0;
        while((user_input < &loc[start_pos]) && (loc[start_pos-1] != '\n'))
        {
            start_pos--;
        }
        int end_pos = 0;
        while(loc[end_pos] != '\n')
        {
            end_pos++;
        }

        // search the line number including the given location
        int line_number = 1;
        for(char *p = user_input; p < &loc[start_pos]; p++)
        {
            if(*p == '\n')
            {
                line_number++;
            }
        }

        // output file name, line number and the line including the given location
        int indent = fprintf(stderr, "%s:%d: ", file_name, line_number);
        fprintf(stderr, "%.*s\n", end_pos - start_pos, &loc[start_pos]);
        pos = indent + (-start_pos);
    }
    else
    {
        // output all the input
        fprintf(stderr, "%s\n", user_input);
        pos = loc - user_input;
    }

    // emphasize the position
    fprintf(stderr, "%*s^\n", pos, "");
}