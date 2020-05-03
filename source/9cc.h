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

// type qualifiers
typedef enum {
    TQ_NONE     = 0,      // no qualifier
    TQ_CONST    = 1 << 0, // const
    TQ_RESTRICT = 1 << 1, // restrict
    TQ_VOLATILE = 1 << 2, // volatile
} TypeQualifier;

// expressions
typedef enum {
    EXPR_CONST,      // integer-constant
    EXPR_VAR,        // variable
    EXPR_FUNC,       // function call
    EXPR_MEMBER,     // member
    EXPR_POST_INC,   // post increment operator
    EXPR_POST_DEC,   // post decrement operator
    EXPR_ADDR,       // address (&)
    EXPR_DEREF,      // dereference (*)
    EXPR_COMPL,      // bitwise complement (~)
    EXPR_NEG,        // logical negation (!)
    EXPR_CAST,       // cast operator
    EXPR_ADD,        // addition (num + num)
    EXPR_PTR_ADD,    // pointer addition (ptr + num | num + ptr)
    EXPR_SUB,        // subtraction (num - num)
    EXPR_PTR_SUB,    // pointer subtraction (ptr - num)
    EXPR_MUL,        // multiplication (*)
    EXPR_DIV,        // division (/)
    EXPR_MOD,        // remainder (%)
    EXPR_LSHIFT,     // left shift (<<)
    EXPR_RSHIFT,     // right shift (>>)
    EXPR_EQ,         // equality comparision (==)
    EXPR_NEQ,        // inequality comparision (!=)
    EXPR_L,          // strict order comparision (<)
    EXPR_LEQ,        // order comparision (<=)
    EXPR_BIT_AND,    // bitwise AND (&)
    EXPR_BIT_XOR,    // exclusive OR (^)
    EXPR_BIT_OR,     // inclusive OR (|)
    EXPR_LOG_AND,    // logical AND (&&)
    EXPR_LOG_OR,     // logical OR (||)
    EXPR_COND,       // conditional expression
    EXPR_ASSIGN,     // assignment expression (=)
    EXPR_ADD_EQ,     // compound assignment expression for addition (+=)
    EXPR_PTR_ADD_EQ, // compound assignment expression for pointer addition (+=)
    EXPR_SUB_EQ,     // compound assignment expression for subtraction (-=)
    EXPR_PTR_SUB_EQ, // compound assignment expression for pointer subtraction (-=)
    EXPR_MUL_EQ,     // compound assignment expression for multiplication (*=)
    EXPR_DIV_EQ,     // compound assignment expression for division (/=)
    EXPR_MOD_EQ,     // compound assignment expression for remainder (%=)
    EXPR_LSHIFT_EQ,  // compound assignment expression for left shift (<<=)
    EXPR_RSHIFT_EQ,  // compound assignment expression for right shift (>>=)
    EXPR_AND_EQ,     // compound assignment expression for bitwise AND (&=)
    EXPR_XOR_EQ,     // compound assignment expression for bitwise exclusive OR (^=)
    EXPR_OR_EQ,      // compound assignment expression for bitwise inclusive OR (|=)
    EXPR_COMMA,      // comma operator (,)
} ExpressionKind;

// statements
typedef enum {
    STMT_LABEL,      // labeled statement
    STMT_CASE,       // case label of switch statement
    STMT_COMPOUND,   // compound statement
    STMT_DECL,       // declaration
    STMT_EXPR,       // expression statement
    STMT_NULL,       // null statement
    STMT_IF,         // if statement
    STMT_SWITCH,     // switch statement
    STMT_WHILE,      // while statement
    STMT_DO,         // do statement
    STMT_FOR,        // for statement
    STMT_GOTO,       // goto statement
    STMT_CONTINUE,   // continue statement
    STMT_BREAK,      // break statement
    STMT_RETURN,     // return statement
} StatementKind;


// structure for token
typedef struct Token Token;
struct Token {
    Token *next;    // next token
    TokenKind kind; // kind of token
    char *str;      // pointer to token string
    size_t len;     // length of token string
};

// structure for contents in data segment
typedef struct DataSegment DataSegment;
struct DataSegment {
    DataSegment *next;   // next element
    const char *label;   // label (symbol)
    size_t size;         // size of data
    bool zero;           // flag indicating that data is zero or non-zero
    long value;          // value of data
};

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
    TypeKind kind;      // kind of type
    size_t size;        // size of type
    size_t align;       // alignment of type
    TypeQualifier qual; // qualification of type
    bool complete;      // flag indicating that the tyee is complete or incomplete
    Type *base;         // base type (only for TY_PTR, TY_ARRAY, TY_FUNC)
    size_t len;         // length of array (only for TY_ARRAY)
    Type *args;         // type of arguments (only for TY_FUNC)
    Type *next;         // next element (only for TY_FUNC)
    Member *member;     // members (only for TY_STRUCT, TY_UNION)
};

// structure for statement (forward declaration)
typedef struct Statement Statement;

// structure for variable
typedef struct Variable Variable;
struct Variable {
    Variable *next;    // next element
    const char *name;  // name of variable
    Type *type;        // type of variable
    Statement *init;   // initializer
    bool local;        // flag indicating that the variable is local or global
    size_t offset;     // offset from base pointer (rbp) (only for local variable)
    char *content;     // content of string-literal including '\0' (only for string-literal)
    bool entity;       // flag indicating that the variable has an entity in the current translation unit (only for global variable)
    DataSegment *data; // contents of data segment (only for global variable)
};

// structure for enumerator
typedef struct {
    const char *name; // name of enumerator
    int value;        // value of enumerator
} Enumerator;

// structure for expression (forward declaration)
typedef struct Expression Expression;

// structure for expression
struct Expression {
    Expression *next;    // next element
    ExpressionKind kind; // kind of expression
    Type *type;          // type of expression
    Expression *lhs;     // left hand side
    Expression *rhs;     // right hand side
    Expression *cond;    // conditional part (only for EXPR_COND)
    long value;          // value of expression (only for EXPR_CONST)
    Variable *var;       // information of variable (only for EXPR_VAR)
    Member *member;      // member (only for EXPR_MEMBER)
    Expression *args;    // arguments (only for EXPR_FUNC)
    char *ident;         // identifier (only for EXPR_FUNC)
};

// structure for declaration
typedef struct Declaration Declaration;
struct Declaration {
    Declaration *next; // next element
    Variable *var;     // declared variable
};

// structure for statement
struct Statement {
    Statement *next;         // next element
    StatementKind kind;      // kind of statement
    Statement *body;         // body of compound statements
    Expression *cond;        // condition (only for STMT_IF, STMT_WHILE, STMT_DO, STMT_FOR)
    Expression *preexpr;     // pre-expression (only for STMT_FOR)
    Expression *postexpr;    // post-expression (only for STMT_FOR)
    Statement *next_case;    // next case (only for STMT_SWITCH, STMT_CASE)
    Statement *default_case; // default case (only for STMT_SWITCH)
    Statement *true_case;    // statements of true case (only for STMT_IF)
    Statement *false_case;   // statements of false case (only for STMT_IF)
    Declaration *decl;       // declaration (only for STMT_DECL)
    Expression *expr;        // expression (only for STMT_EXPR)
    Variable *var;           // information of variable (only for STMT_DECL)
    char *ident;             // identifier (only for STMT_LABEL, STMT_GOTO)
    long value;              // value of node (only for STMT_CASE)
    int case_label;          // sequential number of case label (only for STMT_CASE)
};

// structure for function
typedef struct Function Function;
struct Function {
    Function *next;    // next element
    char *name;        // name of function
    Type *type;        // type of function
    Variable *args;    // arguments
    Statement *body;   // body of function definition
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
Type *new_type(TypeKind kind, TypeQualifier qual);
int get_conversion_rank(const Type *type);
Type *discard_sign(const Type *type);
bool is_integer(const Type *type);
bool is_signed(const Type *type);
bool is_unsigned(const Type *type);
bool is_pointer(const Type *type);
bool is_array(const Type *type);
bool is_pointer_or_array(const Type *type);
bool is_struct(const Type *type);
bool is_union(const Type *type);
Type *new_type_enum(void);
Type *new_type_pointer(Type *base);
Type *new_type_array(Type *base, size_t len);
Type *new_type_function(Type *base, Type *args);
Member *new_member(const Token *token, Type *type);
Member *find_member(const Token *token, const Type *type);

#endif /* !__9CC_H__ */
