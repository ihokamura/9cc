/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser for statement
*/

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "declaration.h"
#include "expression.h"
#include "parser.h"
#include "statement.h"
#include "tokenizer.h"


// function prototype
static Statement *statement(void);


// global variable
static Statement *current_switch = NULL; // currently parsing switch statement


/*
make a new statement
*/
Statement *new_statement(StatementKind kind)
{
    Statement *stmt = calloc(1, sizeof(Statement));
    stmt->next = NULL;
    stmt->kind = kind;
    stmt->value = 0;
    stmt->cond = NULL;
    stmt->preexpr = NULL;
    stmt->postexpr = NULL;
    stmt->body = NULL;
    stmt->ident = NULL;

    return stmt;
}


/*
make a statement
```
statement ::= identifier ":" statement
            | "case" const-expression ":" statement
            | "default" ":" statement
            | compound-statement
            | expr? ";"
            | "if" "(" expression ")" statement ("else" statement)?
            | "switch" "(" expression ")" statement
            | "while" "(" expression ")" statement
            | "do" statement "while" "(" expression ")" ";"
            | "for" "(" expression? ";" expression? ";" expression? ")" statement
            | "goto" identifier ";"
            | "continue" ";"
            | "break" ";"
            | "return" expression? ";"
```
*/
static Statement *statement(void)
{
    Statement *stmt;

    if(peek_reserved("{"))
    {
        // save the current scope
        Scope scope = enter_scope();

        stmt = new_statement(STMT_COMPOUND);
        stmt->body = compound_statement();

        // restore the scope
        leave_scope(scope);
    }
    else if(consume_reserved("break"))
    {
        stmt = new_statement(STMT_BREAK);
        expect_reserved(";");
    }
    else if(consume_reserved("case"))
    {
        // parse label expression
        long value = const_expression();
        expect_reserved(":");

        // parse statement for the case label
        stmt = new_statement(STMT_CASE);
        stmt->body = statement();

        // save the value of label expression and update node of currently parsing switch statement
        stmt->value = value;
        stmt->next_case = current_switch->next_case;
        current_switch->next_case = stmt;
    }
    else if(consume_reserved("continue"))
    {
        stmt = new_statement(STMT_CONTINUE);
        expect_reserved(";");
    }
    else if(consume_reserved("default"))
    {
        expect_reserved(":");

        // parse statement for the default label
        stmt = new_statement(STMT_CASE);
        stmt->body = statement();

        // update node of currently parsing switch statement
        current_switch->default_case = stmt;
    }
    else if(consume_reserved("do"))
    {
        stmt = new_statement(STMT_DO);

        // parse loop body
        stmt->body = statement();

        expect_reserved("while");
        expect_reserved("(");

        // parse loop condition
        stmt->cond = expression();

        expect_reserved(")");
        expect_reserved(";");
    }
    else if(consume_reserved("for"))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        stmt = new_statement(STMT_FOR);
        expect_reserved("(");

        // parse clause-1
        if(!consume_reserved(";"))
        {
            stmt->preexpr = expression();
            expect_reserved(";");
        }

        // parse expression-2
        if(!consume_reserved(";"))
        {
            stmt->cond = expression();
            expect_reserved(";");
        }

        // parse expression-3
        if(!consume_reserved(")"))
        {
            stmt->postexpr = expression();
            expect_reserved(")");
        }

        // parse loop body
        stmt->body = statement();
    }
    else if(consume_reserved("goto"))
    {
        stmt = new_statement(STMT_GOTO);
        stmt->ident = make_identifier(expect_identifier());
        expect_reserved(";");
    }
    else if(consume_reserved("if"))
    {
        stmt = new_statement(STMT_IF);
        expect_reserved("(");

        // parse condition
        stmt->cond = expression();

        expect_reserved(")");

        // parse statement in case of condition being true
        stmt->true_case = statement();

        // parse statement in case of condition being false
        if(consume_reserved("else"))
        {
            stmt->false_case = statement();
        }
    }
    else if(consume_reserved("return"))
    {
        stmt = new_statement(STMT_RETURN);
        if(!consume_reserved(";"))
        {
            // return statement with an expression
            stmt->expr = expression();
            expect_reserved(";");
        }
    }
    else if(consume_reserved("switch"))
    {
        // save node of previous switch statement
        Statement *prev_switch = current_switch;

        // parse controlling expression
        stmt = new_statement(STMT_SWITCH);
        expect_reserved("(");
        stmt->cond = expression();
        expect_reserved(")");

        // update node of currently parsing switch statement and parse body
        current_switch = stmt;
        stmt->body = statement();

        // restore node of previous switch statement
        current_switch = prev_switch;
    }
    else if(consume_reserved("while"))
    {
        stmt = new_statement(STMT_WHILE);
        expect_reserved("(");

        // parse loop condition
        stmt->cond = expression();

        expect_reserved(")");

        // parse loop body
        stmt->body = statement();
    }
    else if(consume_reserved(";"))
    {
        // null statement
        stmt = new_statement(STMT_NULL);
    }
    else
    {
        Token *saved_token = get_token();
        Token *token;
        if(consume_token(TK_IDENT, &token))
        {
            if(consume_reserved(":"))
            {
                // labeled statement
                stmt = new_statement(STMT_LABEL);
                stmt->body = statement();
                stmt->ident = make_identifier(token);
                goto statement_end;
            }
            else
            {
                // resume the token since it is not a label
                set_token(saved_token);
            }
        }

        // expression statement
        stmt = new_statement(STMT_EXPR);
        stmt->expr = expression();
        expect_reserved(";");
    }

statement_end:
    return stmt;
}


/*
make a compound statement
```
compound-statement ::= "{" (declaration | statement)* "}"
```
*/
Statement *compound_statement(void)
{
    expect_reserved("{");

    // parse declaration and/or statement until reaching '}'
    Statement head = {};
    Statement *cursor = &head;
    while(!consume_reserved("}"))
    {
        if(peek_declaration_specifiers())
        {
            // declaration
            cursor->next = declaration(true);
        }
        else
        {
            // statement
            cursor->next = statement();
        }
        cursor = cursor->next;
    }

    return head.next;
}
