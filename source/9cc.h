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

#ifndef __9CC_H__
#define __9CC_H__

#include <stdbool.h>


// indicator of feature enabled or disabled
#define ENABLED     (1)
#define DISABLED    (0)


// features
#define WARN_IMPLICIT_DECLARATION_OF_FUNCTION    (ENABLED) // warn implicit declaration of function


// type of source code
typedef enum {
    SRC_FILE, // file
    SRC_CMD,  // command line
} SourceType;

// kind of token
typedef enum {
    TK_RESERVED, // reserved token
    TK_IDENT,    // identifier
    TK_CONST,    // integer-constant
    TK_STR,      // string-literal
    TK_EOF,      // end of input
} TokenKind;

// kind of type
typedef enum {
    TY_VOID,    // void
    TY_CHAR,    // char
    TY_UCHAR,   // unsigned char
    TY_SHORT,   // short
    TY_USHORT,  // unsigned short
    TY_INT,     // int
    TY_UINT,    // unsigned int
    TY_LONG,    // long
    TY_ULONG,   // unsigned long
    TY_PTR,     // pointer
    TY_ARRAY,   // array
    TY_STRUCT,  // structure
    TY_UNION,   // union
    TY_ENUM,    // enumeration
    TY_TYPEDEF, // typedef name
    TY_FUNC,    // function
} TypeKind;

// kind of node in AST(abstract syntax tree)
typedef enum {
    ND_COMPL,      // bitwise complement (~)
    ND_NEG,        // logical negation (!)
    ND_CAST,       // cast operator
    ND_ADD,        // addition (num + num)
    ND_PTR_ADD,    // pointer addition (ptr + num | num + ptr)
    ND_SUB,        // subtraction (num - num)
    ND_PTR_SUB,    // pointer subtraction (ptr - num)
    ND_MUL,        // multiplication (*)
    ND_DIV,        // division (/)
    ND_MOD,        // remainder (%)
    ND_LSHIFT,     // left shift (<<)
    ND_RSHIFT,     // right shift (>>)
    ND_EQ,         // equality comparision (==)
    ND_NEQ,        // inequality comparision (!=)
    ND_L,          // strict order comparision (<)
    ND_LEQ,        // order comparision (<=)
    ND_BIT_AND,    // bitwise AND (&)
    ND_BIT_XOR,    // exclusive OR (^)
    ND_BIT_OR,     // inclusive OR (|)
    ND_LOG_AND,    // logical AND (&&)
    ND_LOG_OR,     // logical OR (||)
    ND_COND,       // conditional expression
    ND_ASSIGN,     // assignment expression (=)
    ND_ADD_EQ,     // compound assignment expression for addition (+=)
    ND_PTR_ADD_EQ, // compound assignment expression for pointer addition (+=)
    ND_SUB_EQ,     // compound assignment expression for subtraction (-=)
    ND_PTR_SUB_EQ, // compound assignment expression for pointer subtraction (-=)
    ND_MUL_EQ,     // compound assignment expression for multiplication (*=)
    ND_DIV_EQ,     // compound assignment expression for division (/=)
    ND_MOD_EQ,     // compound assignment expression for remainder (%=)
    ND_LSHIFT_EQ,  // compound assignment expression for left shift (<<=)
    ND_RSHIFT_EQ,  // compound assignment expression for right shift (>>=)
    ND_AND_EQ,     // compound assignment expression for bitwise AND (&=)
    ND_XOR_EQ,     // compound assignment expression for bitwise exclusive OR (^=)
    ND_OR_EQ,      // compound assignment expression for bitwise inclusive OR (|=)
    ND_COMMA,      // comma operator (,)
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
    ND_FUNC,       // function call
    ND_DECL,       // declaration
    ND_VAR,        // variable
    ND_MEMBER,     // member
    ND_ADDR,       // address (&)
    ND_DEREF,      // dereference (*)
    ND_CONST,      // integer-constant
} NodeKind;


// structure for token
typedef struct Token Token;
struct Token {
    Token *next;    // next token
    TokenKind kind; // kind of token
    char *str;      // pointer to token string
    size_t len;     // length of token string
};

// structure for integer-constant
typedef struct {
    TypeKind kind;               // type of integer-constant
    union {
        int int_val;             // value for 'int' type
        unsigned int uint_val;   // value for 'unsigned int' type
        long long_val;           // value for 'long' type
        unsigned long ulong_val; // value for 'unsigned long' type
    };
} IntegerConstant;

// structure for type (forward declaration)
typedef struct Type Type;

// structure for member
typedef struct Member Member;
struct Member {
    Member *next;  // next element
    Type *type;    // type of member
    char *name;    // name of member
    size_t offset; // offset of member
};

// structure for type
struct Type {
    TypeKind kind;  // kind of type
    size_t size;    // size of type
    size_t align;   // alignment of type
    bool complete;  // flag indicating that the tyee is complete or incomplete
    Type *base;     // base type (only for TY_PTR, TY_ARRAY, TY_FUNC)
    size_t len;     // length of array (only for TY_ARRAY)
    Type *args;     // type of arguments (only for TY_FUNC)
    Type *next;     // next element (only for TY_FUNC)
    Member *member; // members (only for TY_STRUCT, TY_UNION)
};

// structure for node in AST (forward declaration)
typedef struct Node Node;

// structure for variable
typedef struct Variable Variable;
struct Variable {
    Variable *next;   // next element
    const char *name; // name of variable
    Type *type;       // type of variable
    Node *init;       // initializer
    bool local;       // flag indicating that the variable is local or global
    size_t offset;    // offset from base pointer (rbp) (only for local variable)
    char *content;    // content of string-literal including '\0' (only for string-literal)
    bool entity;      // flag indicating that the variable has an entity in the current translation unit (only for global variable)
};

// structure for enumerator
typedef struct {
    const char *name; // name of enumerator
    int val;          // value of enumerator
} Enumerator;

// structure for node in AST
struct Node {
    Node *next;          // next element
    NodeKind kind;       // kind of node
    Node *lhs;           // left hand side
    Node *rhs;           // right hand side
    Type *type;          // type of node
    IntegerConstant val; // value of node (only for ND_CONST, ND_CASE)
    Variable *var;       // information of variable (only for ND_GVAR, ND_LVAR)
    Node *cond;          // condition (only for ND_IF, ND_WHILE, ND_DO, ND_FOR)
    Node *preexpr;       // pre-expression (only for ND_FOR)
    Node *postexpr;      // post-expression (only for ND_FOR)
    int case_label;      // sequential number of case label (only for ND_CASE)
    Node *next_case;     // next case (only for ND_SWITCH, ND_CASE)
    Node *default_case;  // default case (only for ND_SWITCH)
    Node *body;          // body of compound statements (only for ND_BLOCK, ND_DECL)
    char *ident;         // identifier (only for ND_FUNC, ND_GOTO, ND_LABEL)
    Node *args;          // arguments (only for ND_FUNC)
    Member *member;      // member (only for ND_MEMBER)
};

// structure for function
typedef struct Function Function;
struct Function {
    Function *next;    // next element
    char *name;        // name of function
    Type *type;        // type of function
    Variable *args;    // arguments
    Node *body;        // body of function definition
    Variable *locals;  // list of local variables (including arguments)
    size_t stack_size; // size of stack in bytes
};

// structure for program
typedef struct Program {
    Variable *gvars; // list of global variables
    Function *funcs; // list of functions
} Program;

// function prototype
// parser.c
void construct(Program *prog);
// generator.c
void generate(const Program *program);
// tokenizer.c
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
// type.c
Type *new_type(TypeKind kind);
int get_conversion_rank(const Type *type);
Type *discard_sign(const Type *type);
bool is_integer(const Type *type);
bool is_signed(const Type *type);
bool is_unsigned(const Type *type);
bool is_pointer(const Type *type);
bool is_array(const Type *type);
bool is_pointer_or_array(const Type *type);
Type *new_type_enum(void);
Type *new_type_pointer(Type *base);
Type *new_type_array(Type *base, size_t len);
Type *new_type_function(Type *base, Type *args);
Member *new_member(const Token *token, Type *type);
Member *find_member(const Token *token, const Type *type);

#endif /* !__9CC_H__ */
