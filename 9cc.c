/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// kind of token
typedef enum {
    TK_RESERVED, // operator
    TK_NUM,      // integer
    TK_EOF,      // end of input
} TokenKind;

// token type
typedef struct Token Token;
struct Token {
    TokenKind kind; // kind of token
    Token *next;    // next input token
    int val;        // value of token if its kind is TK_NUM
    char *str;      // token string
};

// currently parsing token
Token *token;


/*
report an error
*/
void error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    exit(1);
}


/*
consume a operator
* If the next token is a given operator, this function parses the token and returns true.
* Otherwise, it returns false.
*/
bool consume(char op)
{
    if((token->kind != TK_RESERVED) || (token->str[0] != op))
    {
        return false;
    }

    token = token->next;

    return true;
}


/*
parse an operator
* If the next token is a given operator, this function parses the token.
* Otherwise, it reports an error.
*/
void expect(char op)
{
    if((token->kind != TK_RESERVED) || token->str[0] != op)
    {
        error("not '%c'.", op);
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
        error("not a number.");
    }

    int val = token->val;

    token = token->next;

    return val;
}


/*
check if the current token is the end of input
*/
bool at_eof(void)
{
    return (token->kind == TK_EOF);
}


/*
make a new token and concatenate it to the current token
*/
Token *new_token(TokenKind kind, Token *cur_token, char *str)
{
    Token *new_token = calloc(1, sizeof(Token));

    new_token->kind = kind;
    new_token->str = str;
    cur_token->next = new_token;

    return new_token;
}


/*
tokenize a given string
*/
Token *tokenize(char *str)
{
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
        if((*str == '+') || (*str == '-'))
        {
            current = new_token(TK_RESERVED, current, str++);
            continue;
        }

        // parse a number
        if(isdigit(*str))
        {
            current = new_token(TK_NUM, current, str);
            current->val = strtol(str, &str, 10);
            continue;
        }

        // Other characters are not accepted as a token.
        error("cannot tokenize.");
    }

    new_token(TK_EOF, current, str);

    return head.next;
}


/*
main function of 9cc
*/
int main(int argc, char **argv)
{
    // check arguments
    if(argc != 2)
    {
        fprintf(stderr, "wrong number of arguments.\n");
        return 1;
    }

    // tokenize input
    token = tokenize(argv[1]);

    // use Intel syntax
    printf(".intel_syntax noprefix\n");

    // start main function
    printf(".global _main\n");
    printf("_main:\n");

    // The first token of the expression should be a number.
    printf("  mov rax, %d\n", expect_number());

    // output assembler code consuming token streams of the form `+ [number]` or `- [number]'
    while(!at_eof())
    {
        // addition
        if(consume('+'))
        {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        // subtraction
        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    // exit main function
    printf("  ret\n");

    return 0;
}