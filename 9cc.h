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
    TK_RETURN,   // keyword `return`
    TK_IF,       // keyword `if`
    TK_ELSE,     // keyword `else`
    TK_WHILE,    // keyword `while`
    TK_FOR,      // keyword `for`
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
    ND_RETURN, // return statement
    ND_IF,     // if statement
    ND_IFELSE, // if-else statement
    ND_WHILE,  // while statement
    ND_FOR,    // for statement
    ND_BLOCK,  // block (compound statement)
    ND_FUNC,   // function
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

// structure for node in AST (forward declaration)
typedef struct Node Node;

// structure for block
typedef struct Block {
    const struct Node **statements; // container of statements in block
    size_t size;              // current number of statements in block
    size_t reserved;          // reserved number of elements in container of statements
} Block;

// structure for node in AST
struct Node {
    NodeKind kind;  // kind of node
    Node *lhs;      // left hand side
    Node *rhs;      // right hand side
    int val;        // value of node (only for ND_NUM)
    int offset;     // offset from base pointer (only for ND_LVAR)
    Node *cond;     // condition (only for ND_IF, ND_IFELSE, ND_WHILE, ND_FOR)
    Node *preexpr;  // pre-expression (only for ND_FOR)
    Node *postexpr; // post-expression (only for ND_FOR)
    Block block;    // block of statements (only for ND_BLOCK)
    char *ident;    // identifier (only for ND_FUNC)
    Node *args[6];  // arguments (only for ND_FUNC)
};

// structure for local variable
typedef struct LVar LVar;
struct LVar {
    LVar *next; // next element
    char *str;  // token string
    int len;    // length of token string
    int offset; // offset from base pointer (rbp)
};

// structure for function
typedef struct Function Function;
struct Function {
    Function *next;    // next element
    char *name;        // name of function
    size_t argc;       // number of arguments
    Block body;        // body of function definition
    LVar *locals;      // list of local variables (including arguments)
    size_t stack_size; // size of stack in bytes
};


// function prototype
// codegen.c
void construct(void);
void generate(void);
// parse.c
bool consume_operator(const char *op);
bool consume_keyword(TokenKind kind);
Token *consume_ident(void);
void expect_operator(const char *op);
int expect_number(void);
void tokenize(char *str);
bool at_eof(void);
void report_error(char *loc, const char *fmt, ...);