/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include "9cc.h"
#include "type.h"

// kind of expressions
typedef enum ExpressionKind ExpressionKind;
enum ExpressionKind
{
    EXPR_CONST,      // constant
    EXPR_VAR,        // variable
    EXPR_GENERIC,    // generic-selection
    EXPR_FUNC,       // function call
    EXPR_MEMBER,     // member
    EXPR_POST_INC,   // post increment operator
    EXPR_POST_DEC,   // post decrement operator
    EXPR_COMPOUND,   // compound-literal
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
};

// structure for expression
struct Expression
{
    ExpressionKind kind;              // kind of expression
    Type *type;                       // type of expression
    Expression *lhs;                  // left hand side of binary operation
    Expression *rhs;                  // right hand side of binary operation
    Expression *operand;              // operand of unary operation or condition of conditional expression
    long value;                       // value of expression (only for EXPR_CONST)
    StringLiteral *str;               // information of string-literal (only for EXPR_STR)
    Variable *var;                    // information of variable (only for EXPR_VAR)
    Member *member;                   // member (only for EXPR_MEMBER)
    List(Expression) *args;           // arguments (only for EXPR_FUNC)
    bool lvalue;                      // flag indicating that the expression is lvalue
};

// structure for generic-association
struct GenericAssociation
{
    Type *type;         // type
    Expression *assign; // result expression
};

Expression *new_expression(ExpressionKind kind, Type *type);
Expression *new_node_constant(TypeKind kind, long value);
Expression *new_node_subscript(Expression *base, size_t index);
Expression *new_node_member(Expression *expr, Member *member);
Expression *new_node_binary(ExpressionKind kind, Expression *lhs, Expression *rhs);
Expression *assign(void);
Expression *expression(void);
long const_expression(void);
long evaluate(const Expression *expr, const Expression **base);

#endif /* !__EXPRESSION_H__ */
