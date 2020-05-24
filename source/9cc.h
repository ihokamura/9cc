/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __9CC_H__
#define __9CC_H__

#include <stdbool.h>
#include <stddef.h>


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
    TK_CONST,    // constant
    TK_STR,      // string-literal
} TokenKind;

// kind of type
typedef enum {
    TY_VOID,    // void
    TY_CHAR,    // char
    TY_SCHAR,   // signed char
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
    EXPR_CONST,      // constant
    EXPR_VAR,        // variable
    EXPR_FUNC,       // function call
    EXPR_MEMBER,     // member
    EXPR_POST_INC,   // post increment operator
    EXPR_POST_DEC,   // post decrement operator
    EXPR_ADDR,       // address (&)
    EXPR_DEREF,      // dereference (*)
    EXPR_PLUS,       // plus (+)
    EXPR_MINUS,      // minus (-)
    EXPR_COMPL,      // bitwise complement (~)
    EXPR_NEG,        // logical negation (!)
    EXPR_CAST,       // cast operator
    EXPR_MUL,        // multiplication (*)
    EXPR_DIV,        // division (/)
    EXPR_MOD,        // remainder (%)
    EXPR_ADD,        // addition (arithmetic + arithmetic)
    EXPR_PTR_ADD,    // pointer addition (pointer + integer or integer + pointer)
    EXPR_SUB,        // subtraction (arithmetic - arithmetic)
    EXPR_PTR_SUB,    // pointer subtraction (pointer - integer)
    EXPR_PTR_DIFF,   // pointer difference (pointer - pointer)
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

typedef enum {
    SC_TYPEDEF,  // "typdef"
    SC_EXTERN,   // "extern"
    SC_STATIC,   // "static"
    SC_AUTO,     // "auto"
    SC_REGISTER, // "register"
} StorageClassSpecifier;


// forward declaration of structure types
typedef struct Token Token;
typedef struct DataSegment DataSegment;
typedef struct Member Member;
typedef struct Tag Tag;
typedef struct Type Type;
typedef struct Statement Statement;
typedef struct StringLiteral StringLiteral;
typedef struct Variable Variable;
typedef struct Expression Expression;
typedef struct Declaration Declaration;
typedef struct Function Function;
typedef struct Identifier Identifier;
#include "list.h"

// structure for token
struct Token {
    TokenKind kind; // kind of token
    char *str;      // pointer to token string
    size_t len;     // length of token string
    TypeKind type;  // type of token (only for TK_CONST)
    long value;     // value of token (only for TK_CONST)
};

// structure for contents in data segment
struct DataSegment {
    DataSegment *next;   // next element
    const char *label;   // label (symbol)
    size_t size;         // size of data
    bool zero;           // flag indicating that data is zero or non-zero
    long value;          // value of data
};

// structure for member
struct Member {
    Type *type;       // type of member
    const char *name; // name of member
    size_t offset;    // offset of member (only for TY_STRUCT, TY_UNION)
    int value;        // value of member (only for TY_ENUM)
};

// structure for type
struct Type {
    TypeKind kind;         // kind of type
    size_t size;           // size of type
    size_t align;          // alignment of type
    TypeQualifier qual;    // qualification of type
    bool complete;         // flag indicating that the tyee is complete or incomplete
    Type *base;            // base type (only for TY_PTR, TY_ARRAY, TY_FUNC)
    size_t len;            // length of array (only for TY_ARRAY)
    List(Type) *args;      // type of arguments (only for TY_FUNC)
    Tag *tag;              // tag (only for TY_STRUCT, TY_UNION, TY_ENUM)
    List(Member) *members; // members (only for TY_STRUCT, TY_UNION, TY_ENUM)
};

// structure for variable
struct Variable {
    const char *name;   // name of variable
    Type *type;         // type of variable
    Statement *init;    // initializer
    bool local;         // flag indicating that the variable is local or global
    size_t offset;      // offset from base pointer (rbp) (only for local variable)
    StringLiteral *str; // information of string-literal (only for string-literal)
    bool entity;        // flag indicating that the variable has an entity in the current translation unit (only for global variable)
    DataSegment *data;  // contents of data segment (only for global variable)
};

// structure for enumerator
typedef struct Enumerator {
    const char *name; // name of enumerator
    int value;        // value of enumerator
} Enumerator;

// structure for string-literal
struct StringLiteral {
    Variable *var;       // global variable which points to string-literal
    const char *content; // array of characters consisting of string-literal
};

// structure for expression
struct Expression {
    ExpressionKind kind;    // kind of expression
    Type *type;             // type of expression
    Expression *lhs;        // left hand side of binary operation
    Expression *rhs;        // right hand side of binary operation
    Expression *operand;    // operand of unary operation or condition of conditional expression
    long value;             // value of expression (only for EXPR_CONST)
    StringLiteral *str;     // information of string-literal (only for EXPR_STR)
    Variable *var;          // information of variable (only for EXPR_VAR)
    Member *member;         // member (only for EXPR_MEMBER)
    List(Expression) *args; // arguments (only for EXPR_FUNC)
    bool lvalue;            // flag indicating that the expression is lvalue
};

// structure for declaration
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
struct Function {
    char *name;             // name of function
    Type *type;             // type of function
    List(Variable) *args;   // arguments
    Statement *body;        // body of function definition
    List(Variable) *locals; // list of local variables (including arguments)
    size_t stack_size;      // size of stack in bytes
};

// structure for program
typedef struct Program {
    List(Variable) *gvars; // list of global variables
    List(Function) *funcs; // list of functions
} Program;

// structure for identifier
struct Identifier {
    const char *name; // identifier
    Variable *var;    // variable
    Enumerator *en;   // enumerator
    Type *type_def;   // type definition
    int depth;        // depth of scope
};

// structure for tag
struct Tag {
    const char *name; // name of tag
    Type *type;       // type 
    int depth;        // depth of scope
};

// structure for scope
typedef struct Scope {
    ListEntry(Identifier) *ident_list; // list of ordinary identifiers visible in the current scope
    ListEntry(Tag) *tag_list;          // list of tags visible in the current scope
    int depth;                         // depth of the current scope
} Scope;

#endif /* !__9CC_H__ */
