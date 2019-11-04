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

// kind of node in AST(abstract syntax tree)
typedef enum {
    ND_ADD, // addition(+)
    ND_SUB, // subtraction(-)
    ND_MUL, // multiplication(*)
    ND_DIV, // division(/)
    ND_NUM, // integer
} NodeKind;

// structure for token
typedef struct Token Token;
struct Token {
    TokenKind kind; // kind of token
    Token *next;    // next input token
    int val;        // value of token (only for TK_NUM)
    char *str;      // token string
};

// structure for node in AST
typedef struct Node Node;
struct Node {
    NodeKind kind; // kind of node
    Node *lhs;     // left hand side
    Node *rhs;     // right hand side
    int val;       // value of node (only for ND_NUM)
};


// currently parsing token
Token *token;
// input expression
char *user_input;


// function prototype
Node *expr(void);
Node *mul(void);
Node *primary(void);


/*
report an error
* This function never returns.
*/
void error_at(char *loc, const char *fmt, ...)
{
    // emphasize the position where the error is detected
    int pos = (loc - user_input) / sizeof(user_input[0]);
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s ^", pos, "");

    // print the error message
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
        error_at(token->str, "not '%c'.", op);
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
        error_at(token->str, "not a number.");
    }

    int val = token->val;

    token = token->next;

    return val;
}


/*
make a new token and concatenate it to the current token
*/
Token *new_token(TokenKind kind, Token *cur_tok, char *str)
{
    Token *new_tok = calloc(1, sizeof(Token));

    new_tok->kind = kind;
    new_tok->str = str;
    cur_tok->next = new_tok;

    return new_tok;
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
        if(
            (*str == '+') || 
            (*str == '-') || 
            (*str == '*') || 
            (*str == '/') || 
            (*str == '(') || 
            (*str == ')')
            )
        {
            current = new_token(TK_RESERVED, current, str);
            str++;
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
        error_at(token->str, "cannot tokenize.");
    }

    new_token(TK_EOF, current, str);

    return head.next;
}


/*
make a new node for non-number
*/
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}


/*
make a new node for number
*/
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->val = val;

    return node;
}


/*
make an expression
* expr = mul ("+" mul | "-" mul)*
*/
Node *expr(void)
{
    Node *node = mul();

    // parse tokens while finding a term
    while(true)
    {
        if(consume('+'))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume('-'))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}


/*
make a term
* mul = primary ("*" primary | "/" primary)*
*/
Node *mul(void)
{
    Node *node = primary();

    // parse tokens while finding a primary
    while(true)
    {
        if(consume('*'))
        {
            node = new_node(ND_MUL, node, mul());
        }
        else if(consume('/'))
        {
            node = new_node(ND_DIV, node, mul());
        }
        else
        {
            return node;
        }
    }
}


/*
make a primary
* primary = num | "(" expr ")"
*/
Node *primary(void)
{
    if(consume('('))
    {
        // expression in brackets
        Node *node = expr();
 
        expect(')');
 
        return node;
    }

    // number
    return new_node_num(expect_number());
}


/*
generate assembler code which emulates stack machine
*/
void gen(const Node *node)
{
    if(node->kind == ND_NUM)
    {
        printf("  push %d\n", node->val);

        return;
    }

    // compile LHS and RHS
    gen(node->lhs);
    gen(node->rhs);

    // pop RHS to rdi and LHS to rax
    printf("  pop rdi\n");
    printf("  pop rax\n");

    // execute operation
    switch(node->kind)
    {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;

        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;

        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;

        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;

        default:
            break;
    }

    // push return value
    printf("  push rax\n");
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

    // save input
    user_input = argv[1];
    // parse input
    token = tokenize(user_input);
    // construct syntax tree
    Node *root = expr();

    // use Intel syntax
    printf(".intel_syntax noprefix\n");

    // start main function
    printf(".global _main\n");
    printf("_main:\n");

    // output assembler according to the syntax tree
    gen(root);

    // pop return value from stack
    printf("  pop rax\n");

    // exit main function
    printf("  ret\n");

    return 0;
}