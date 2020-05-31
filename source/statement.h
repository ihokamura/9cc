/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __STATEMENT_H__
#define __STATEMENT_H__

#include "9cc.h"

// kind of statements
typedef enum StatementKind StatementKind;
enum StatementKind
{
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
};

// structure for statement
struct Statement
{
    StatementKind kind;         // kind of statement
    Statement *body;            // body of statement (only for STMT_CASE, STMT_WHILE, STMT_DO, STMT_FOR)
    Expression *cond;           // condition (only for STMT_IF, STMT_WHILE, STMT_DO, STMT_FOR)
    Expression *preexpr;        // pre-expression (only for STMT_FOR)
    Expression *postexpr;       // post-expression (only for STMT_FOR)
    List(Statement) *case_list; // list of case statements (only for STMT_SWITCH)
    Statement *default_case;    // default case (only for STMT_SWITCH)
    Statement *true_case;       // statements of true case (only for STMT_IF)
    Statement *false_case;      // statements of false case (only for STMT_IF)
    List(Statement) *compound;  // compound statement
    List(Declaration) *decl;    // declaration (only for STMT_DECL)
    Expression *expr;           // expression (only for STMT_EXPR)
    Variable *var;              // information of variable (only for STMT_DECL)
    char *ident;                // identifier (only for STMT_LABEL, STMT_GOTO)
    long value;                 // value of node (only for STMT_CASE)
    int case_label;             // sequential number of case label (only for STMT_CASE)
};

Statement *new_statement(StatementKind kind);
List(Statement) *compound_statement(void);

#endif /* !__STATEMENT_H__ */
