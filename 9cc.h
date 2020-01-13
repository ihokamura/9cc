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
    TK_RESERVED, // reserved token
    TK_IDENT,    // identifier
    TK_NUM,      // integer
    TK_EOF,      // end of input
} TokenKind;

// kind of type
typedef enum {
    TY_INT,   // int
    TY_PTR,   // pointer
    TY_ARRAY, // array
} TypeKind;

// kind of node in AST(abstract syntax tree)
typedef enum {
    ND_ADD,     // addition (num + num)
    ND_PTR_ADD, // pointer addition (ptr + num | num + ptr)
    ND_SUB,     // subtraction (num - num)
    ND_PTR_SUB, // pointer subtraction (ptr - num)
    ND_MUL,     // multiplication (*)
    ND_DIV,     // division (/)
    ND_EQ,      // equality comparision (==)
    ND_NEQ,     // inequality comparision (!=)
    ND_L,       // strict order comparision (<)
    ND_LEQ,     // order comparision (<=)
    ND_ASSIGN,  // assignment expression (=)
    ND_RETURN,  // return statement
    ND_IF,      // if statement
    ND_WHILE,   // while statement
    ND_DO,      // do statement
    ND_FOR,     // for statement
    ND_BLOCK,   // block (compound statement)
    ND_FUNC,    // function call
    ND_DECL,    // declaration
    ND_LVAR,    // local variable
    ND_ADDR,    // address (&)
    ND_DEREF,   // dereference (*)
    ND_NUM,     // integer
} NodeKind;


// structure for token
typedef struct Token Token;
struct Token {
    Token *next;    // next input token
    TokenKind kind; // kind of token
    int val;        // value of token (only for TK_NUM)
    char *str;      // token string
    int len;        // length of token string
};

// structure for type
typedef struct Type Type;
struct Type {
    TypeKind kind;       // kind of type
    size_t size;         // size of type
    struct Type *base;   // base type (only for TY_PTR and TY_ARRAY)
    size_t len;          // length of array (only for TY_ARRAY)
};

// structure for local variable
typedef struct LVar LVar;
struct LVar {
    LVar *next; // next element
    char *str;  // token string
    int len;    // length of token string
    int offset; // offset from base pointer (rbp)
    Type *type; // type of variable
};

// structure for node in AST
typedef struct Node Node;
struct Node {
    Node *next;     // next element
    NodeKind kind;  // kind of node
    Node *lhs;      // left hand side
    Node *rhs;      // right hand side
    Type *type;     // type of node
    int val;        // value of node (only for ND_NUM)
    LVar *lvar;     // information of local variable (only for ND_LVAR)
    Node *cond;     // condition (only for ND_IF, ND_WHILE, ND_DO, ND_FOR)
    Node *preexpr;  // pre-expression (only for ND_FOR)
    Node *postexpr; // post-expression (only for ND_FOR)
    Node *body;     // body of compound statements (only for ND_BLOCK)
    char *ident;    // identifier (only for ND_FUNC)
    Node *args[6];  // arguments (only for ND_FUNC)
};

// structure for function
typedef struct Function Function;
struct Function {
    Function *next;    // next element
    char *name;        // name of function
    size_t argc;       // number of arguments
    LVar *args[6];     // arguments
    Type *type;        // type of return value
    Node *body;        // body of function definition
    LVar *locals;      // list of local variables (including arguments)
    size_t stack_size; // size of stack in bytes
};


// function prototype
// parser.c
void construct(Function **functions);
// generator.c
void generate(Function *functions);
// tokenizer.c
bool consume_reserved(const char *str);
Token *consume_ident(void);
void expect_reserved(const char *str);
int expect_number(void);
void expect_declarator(Type **type, Token **token);
void tokenize(char *str);
bool at_eof(void);
void report_error(char *loc, const char *fmt, ...);
// type.c
Type *new_type(TypeKind kind);
bool is_pointer(const Node *node);


// global variable
extern const size_t LVAR_SIZE; // size of a local variable in bytes
extern const char *arg_registers[]; // name of registers for function arguments
extern const size_t ARG_REGISTERS_SIZE; // number of registers for function arguments