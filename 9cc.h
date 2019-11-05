/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* definition of enumeration
* definition of structure
* definition of type
* declaration of function prototype
* declaration of global variable
*/

#include <stdbool.h>


// kind of token
typedef enum {
    TK_RESERVED, // operator
    TK_IDENT,    // identifier
    TK_NUM,      // integer
    TK_EOF,      // end of input
} TokenKind;

// kind of node in AST(abstract syntax tree)
typedef enum {
    ND_ADD,    // addition (+)
    ND_SUB,    // subtraction (-)
    ND_MUL,    // multiplication (*)
    ND_DIV,    // division (/)
    ND_EQ,     // equality comparision (==)
    ND_NEQ,    // inequality comparision (!=)
    ND_L,      // strict order comparision (<)
    ND_LEQ,    // order comparision (<=)
    ND_ASSIGN, // assignment expression (=)
    ND_LVAR,   // local variable
    ND_NUM,    // integer
} NodeKind;


// structure for token
typedef struct Token Token;
struct Token {
    TokenKind kind; // kind of token
    Token *next;    // next input token
    int val;        // value of token (only for TK_NUM)
    char *str;      // token string
    int len;        // length of token string
};

// structure for node in AST
typedef struct Node Node;
struct Node {
    NodeKind kind; // kind of node
    Node *lhs;     // left hand side
    Node *rhs;     // right hand side
    int val;       // value of node (only for ND_NUM)
    int offset;    // offset from base pointer (only for ND_LVAR)
};


// function prototype
// codegen.c
void construct(void);
void generate(void);
// parse.c
bool consume(const char *op);
Token *consume_ident(void);
void expect(const char *op);
int expect_number(void);
void tokenize(char *str);
bool at_eof(void);
void report_error(char *loc, const char *fmt, ...);