/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* definition of macro
* definition of enumeration
* definition of structure
* definition of type
* declaration of function prototype
* declaration of global variable
*/

#include <stdbool.h>


// indicator of feature enabled or disabled
#define ENABLED     (1)
#define DISABLED    (0)


// features
#define WARN_IMPLICIT_DECLARATION_OF_FUNCTION    (DISABLED) // warn implicit declaration of function


// type of source code
typedef enum {
    SRC_FILE, // file
    SRC_CMD,  // command line
} SourceType;

// kind of token
typedef enum {
    TK_RESERVED, // reserved token
    TK_IDENT,    // identifier
    TK_NUM,      // integer
    TK_STR,      // string-literal
    TK_EOF,      // end of input
} TokenKind;

// kind of type
typedef enum {
    TY_CHAR,  // char
    TY_SHORT, // short
    TY_INT,   // int
    TY_LONG,  // long
    TY_PTR,   // pointer
    TY_ARRAY, // array
} TypeKind;

// kind of node in AST(abstract syntax tree)
typedef enum {
    ND_ADD,        // addition (num + num)
    ND_PTR_ADD,    // pointer addition (ptr + num | num + ptr)
    ND_SUB,        // subtraction (num - num)
    ND_PTR_SUB,    // pointer subtraction (ptr - num)
    ND_MUL,        // multiplication (*)
    ND_DIV,        // division (/)
    ND_EQ,         // equality comparision (==)
    ND_NEQ,        // inequality comparision (!=)
    ND_L,          // strict order comparision (<)
    ND_LEQ,        // order comparision (<=)
    ND_ASSIGN,     // assignment expression (=)
    ND_ADD_EQ,     // compound assignment expression for addition (+=)
    ND_PTR_ADD_EQ, // compound assignment expression for pointer addition (+=)
    ND_SUB_EQ,     // compound assignment expression for subtraction (-=)
    ND_PTR_SUB_EQ, // compound assignment expression for pointer subtraction (-=)
    ND_MUL_EQ,     // compound assignment expression for multiplication (*=)
    ND_DIV_EQ,     // compound assignment expression for division (/=)
    ND_POST_INC,   // post increment operator
    ND_POST_DEC,   // post decrement operator
    ND_GOTO,       // goto statement
    ND_BREAK,      // break statement
    ND_CONTINUE,   // continue statement
    ND_RETURN,     // return statement
    ND_IF,         // if statement
    ND_SWITCH,     // switch statement
    ND_CASE,       // case label of switch statement
    ND_LABEL,      // labeled statement
    ND_WHILE,      // while statement
    ND_DO,         // do statement
    ND_FOR,        // for statement
    ND_BLOCK,      // block (compound statement)
    ND_NULL,       // null statement
    ND_GVAR,       // global variable
    ND_FUNC,       // function call
    ND_DECL,       // declaration
    ND_LVAR,       // local variable
    ND_ADDR,       // address (&)
    ND_DEREF,      // dereference (*)
    ND_NUM,        // integer
} NodeKind;


// structure for token
typedef struct Token Token;
struct Token {
    Token *prev;    // previous token
    Token *next;    // next token
    TokenKind kind; // kind of token
    long val;       // value of token (only for TK_NUM)
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

// structure for node in AST (forward declaration)
typedef struct Node Node;

// structure for global variable
typedef struct GVar GVar;
struct GVar {
    GVar *next; // next element
    char *name; // name of global variable
    Type *type; // type of variable
    char *content; // content of string-literal including '\0' (only for string-literal)
    Node *init; // initializer
};

// structure for local variable
typedef struct LVar LVar;
struct LVar {
    LVar *next; // next element
    char *str;  // token string
    int len;    // length of token string
    int offset; // offset from base pointer (rbp)
    Type *type; // type of variable
    Node *init; // initializer
};

// structure for node in AST
struct Node {
    Node *next;         // next element
    NodeKind kind;      // kind of node
    Node *lhs;          // left hand side
    Node *rhs;          // right hand side
    Type *type;         // type of node
    long val;           // value of node (only for ND_NUM, ND_CASE)
    GVar *gvar;         // information of global variable (only for ND_GVAR)
    LVar *lvar;         // information of local variable (only for ND_LVAR)
    Node *cond;         // condition (only for ND_IF, ND_WHILE, ND_DO, ND_FOR)
    Node *preexpr;      // pre-expression (only for ND_FOR)
    Node *postexpr;     // post-expression (only for ND_FOR)
    int case_label;     // sequential number of case label (only for ND_CASE)
    Node *next_case;    // next case (only for ND_SWITCH, ND_CASE)
    Node *default_case; // default case (only for ND_SWITCH)
    Node *body;         // body of compound statements (only for ND_BLOCK)
    char *ident;        // identifier (only for ND_FUNC)
    Node *args;         // arguments (only for ND_FUNC)
};

// structure for function
typedef struct Function Function;
struct Function {
    Function *next;    // next element
    char *name;        // name of function
    Type *type;        // type of return value
    LVar *args;        // arguments
    Node *body;        // body of function definition
    LVar *locals;      // list of local variables (including arguments)
    size_t stack_size; // size of stack in bytes
};

// structure for program
typedef struct Program {
    GVar *gvars;     // list of global variables
    Function *funcs; // list of functions
} Program;

// function prototype
// parser.c
void construct(Program *program);
// generator.c
void generate(const Program *program);
// tokenizer.c
bool peek_reserved(const char *str);
bool peek_token(TokenKind kind, Token **token);
bool consume_reserved(const char *str);
bool consume_token(TokenKind kind, Token **token);
void resume_token(void);
void expect_reserved(const char *str);
Token *expect_ident(void);
long expect_number(void);
void tokenize(char *str);
bool at_eof(void);
char *make_ident(const Token *token);
void set_source(SourceType type);
char *read_file(const char *path);
void report_warning(const char *loc, const char *fmt, ...);
void report_error(const char *loc, const char *fmt, ...);
// type.c
Type *new_type(TypeKind kind);
bool is_integer(const Type *type);
bool is_pointer(const Type *type);
bool is_array(const Type *type);
bool is_pointer_or_array(const Type *type);
Type *new_type_pointer(Type *base);
Type *new_type_array(Type *base, size_t len);