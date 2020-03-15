/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser (syntax tree constructor)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// function prototype
static void prg(void);
static void gvar(void);
static void func(void);
static bool peek_typename(void);
static Type *declaration_spec(void);
static Type *declarator(Type *type, Token **token);
static Type *parameter_list(LVar **arg_vars);
static Type *parameter_declaration(LVar **arg_var);
static Type *type_name(void);
static Type *type_spec(void);
static Type *pointer(Type *base);
static Type *direct_declarator(Type *type, Token **token);
static Node *stmt(void);
static Node *compound_stmt(void);
static Node *declaration(void);
static Node *init_declarator_list(Type *type);
static Node *init_declarator(Type *type);
static Node *initializer(void);
static Node *expr(void);
static Node *assign(void);
static Node *conditional(void);
static Node *logical_or_expr(void);
static Node *logical_and_expr(void);
static Node *or_expr(void);
static Node *xor_expr(void);
static Node *and_expr(void);
static Node *equality(void);
static Node *relational(void);
static Node *shift(void);
static Node *additive(void);
static Node *multiplicative(void);
static Node *cast(void);
static Node *unary(void);
static Node *postfix(void);
static Node *arg_expr_list(void);
static Node *primary(void);
static Node *new_node(NodeKind kind);
static Node *new_node_unary(NodeKind kind, Node *lhs);
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static GVar *new_gvar(const Token *token, GVar *cur_gvar, Type *type);
static GVar *new_str(const Token *token);
static GVar *get_gvar(const Token *token);
static LVar *new_lvar(const Token *token, LVar *cur_lvar, Type *type);
static LVar *get_lvar(const Token *token);
static Function *new_function(const Token *token, Function *cur_func, Type *type, LVar *args, size_t stack_size);
static Function *get_function(const Token *token);
static LVar *new_param(const Token *token, Type *type);
static bool peek_func(void);
static char *new_strlabel(void);
static long const_expr(void);
static long evaluate(const Node *node);


// global variable
static GVar *gvar_list; // list of global variables
static GVar *current_gvar; // currently parsing global variable
static int str_label = 0; // label number of string-literal
static Function *function_list; // list of functions
static Function *current_function; // currently constructing function
static LVar *current_args = NULL; // arguments of currently constructing function
static const size_t stack_alignment_size = 8; // alignment size of function stack
static Node *current_switch = NULL; // currently parsing switch statement


/*
construct syntax tree
*/
void construct(Program *program)
{
    prg();
    program->gvars = gvar_list;
    program->funcs = function_list;
}


/*
make a program
```
prg ::= (gvar | func)*
```
*/
static void prg(void)
{
    GVar gvar_head = {};
    current_gvar = &gvar_head;
    gvar_list = &gvar_head;

    Function func_head = {};
    current_function = &func_head;
    function_list = &func_head;

    while(!at_eof())
    {
        if(peek_func())
        {
            // parse function
            func();
        }
        else
        {
            // parse global variable
            gvar();
        }
    }

    gvar_list = gvar_head.next;
    function_list = func_head.next;
}


/*
make a global variable
```
gvar ::= declaration-spec declarator ("=" initializer) ";"
```
*/
static void gvar(void)
{
    // parse declaration specifier and declarator
    Type *base = declaration_spec();
    Token *token;
    base = declarator(base, &token);

    current_gvar = new_gvar(token, current_gvar, base);

    // parse initializer
    if(consume_reserved("="))
    {
        current_gvar->init = initializer();
    }
    expect_reserved(";");
}


/*
make a function
```
func ::= declaration-spec declarator compound-stmt
```
*/
static void func(void)
{
    // parse declaration specifier and declarator
    Type *type = declaration_spec();
    Token *token;
    type = declarator(type, &token);

    // accumulate stack size and set offset of arguments
    size_t stack_size = 0;
    if(type->args->kind != TY_VOID)
    {
        for(LVar *arg = current_args; arg != NULL; arg = arg->next)
        {
            stack_size += arg->type->size;
            arg->offset = stack_size;
        }
    }

    // make a new function
    current_function = new_function(token, current_function, type, current_args, stack_size);

    // parse body
    current_function->body = compound_stmt();
}


/*
peek a type name
*/
static bool peek_typename(void)
{
    return (
           peek_reserved("void")
        || peek_reserved("char")
        || peek_reserved("short")
        || peek_reserved("int")
        || peek_reserved("long")
    );
}


/*
make a declaration specifier
```
declaration-spec ::= type-spec
```
*/
static Type *declaration_spec(void)
{
    Type *type = type_spec();

    return type;
}



/*
make a declarator
```
declarator ::= pointer? direct-declarator
```
*/
static Type *declarator(Type *type, Token **token)
{
    if(peek_reserved("*"))
    {
        type = pointer(type);
    }

    type = direct_declarator(type, token);

    return type;
}


/*
make a parameterlist
```
parameter-list ::= parameter-declaration ("," parameter-declaration)*
```
*/
static Type *parameter_list(LVar **arg_vars)
{
    Type *arg_types;
    Type arg_types_head = {};
    Type *arg_types_cursor = &arg_types_head;
    LVar arg_vars_head = {};
    LVar *arg_vars_cursor = &arg_vars_head;

    arg_types_cursor->next = parameter_declaration(&arg_vars_cursor->next);
    arg_types_cursor = arg_types_cursor->next;
    arg_vars_cursor = arg_vars_cursor->next;

    while(consume_reserved(","))
    {
        arg_types_cursor->next = parameter_declaration(&arg_vars_cursor->next);
        arg_types_cursor = arg_types_cursor->next;
        arg_vars_cursor = arg_vars_cursor->next;
    }

    arg_types = arg_types_head.next;
    *arg_vars = arg_vars_head.next;

    return arg_types;
}


/*
make a parameter declaration
```
parameter-declaration ::= declaration-spec declarator
```
*/
static Type *parameter_declaration(LVar **arg_var)
{
    Type *arg_type = declaration_spec();
    Token *arg_token;

    arg_type = declarator(arg_type, &arg_token);
    *arg_var = new_param(arg_token, arg_type);

    return arg_type;
}


/*
make a type name
```
type-name ::= type-spec pointer?
```
*/
static Type *type_name(void)
{
    Type *type = type_spec();

    if(peek_reserved("*"))
    {
        type = pointer(type);
    }

    return type;
}


/*
make a type specifier
```
type-spec ::= "void" | "char" | "short" | "int" | "long"
```
*/
static Type *type_spec(void)
{
    if(consume_reserved("void"))
    {
        return new_type(TY_VOID);
    }
    else if(consume_reserved("char"))
    {
        return new_type(TY_CHAR);
    }
    else if(consume_reserved("short"))
    {
        return new_type(TY_SHORT);
    }
    else if(consume_reserved("int"))
    {
        return new_type(TY_INT);
    }
    else if(consume_reserved("long"))
    {
        return new_type(TY_LONG);
    }
    else
    {
        return NULL;
    }
}


/*
make a pointer
```
pointer ::= "*" "*"*
```
*/
static Type *pointer(Type *base)
{
    Type *type = base;

    expect_reserved("*");
    type = new_type_pointer(type);

    while(consume_reserved("*"))
    {
        type = new_type_pointer(type);
    }

    return type;
}


/*
make a direct declarator
```
direct-declarator ::= ident
                    | direct-declarator "[" const-expr "]"
                    | direct-declarator "(" ("void" | parameter-list)? ")"
```
*/
static Type *direct_declarator(Type *type, Token **token)
{
    if(consume_token(TK_IDENT, token))
    {
        type = direct_declarator(type, token);
    }
    else
    {
        if(consume_reserved("["))
        {
            size_t len = const_expr();
            expect_reserved("]");
            type = direct_declarator(type, token);
            type = new_type_array(type, len);
        }
        else if(consume_reserved("("))
        {
            // parse arguments
            Type *arg_types = new_type(TY_VOID);
            LVar *arg_vars = NULL;
            if(!consume_reserved(")"))
            {
                if(!consume_reserved("void"))
                {
                    arg_types = parameter_list(&arg_vars);
                }
                expect_reserved(")");
            }

            type = direct_declarator(type, token);
            type = new_type_function(type, arg_types);
            current_args = arg_vars;
        }
    }

    return type;
}


/*
make a statement
```
stmt ::= ident ":" stmt
       | "case" num ":" stmt
       | "default" ":" stmt
       | declaration
       | "{" stmt* "}"
       | expr? ";"
       | "if" "(" expr ")" stmt ("else" stmt)?
       | "switch" "(" expr ")" stmt
       | "while" "(" expr ")" stmt
       | "do" stmt "while" "(" expr ")" ";"
       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
       | "goto" ident ";"
       | "continue" ";"
       | "break" ";"
       | "return" expr ";"
```
*/
static Node *stmt(void)
{
    Node *node;

    if(peek_reserved("{"))
    {
        node = new_node(ND_BLOCK);
        node->body = compound_stmt();
    }
    else if(consume_reserved("break"))
    {
        node = new_node(ND_BREAK);
        expect_reserved(";");
    }
    else if(consume_reserved("case"))
    {
        // parse label expression
        long val = expect_number();
        expect_reserved(":");

        // parse statement for the case label
        node = new_node(ND_CASE);
        node->lhs = stmt();

        // save the value of label expression and update node of currently parsing switch statement
        node->val = val;
        node->next_case = current_switch->next_case;
        current_switch->next_case = node;
    }
    else if(consume_reserved("continue"))
    {
        node = new_node(ND_CONTINUE);
        expect_reserved(";");
    }
    else if(consume_reserved("default"))
    {
        expect_reserved(":");

        // parse statement for the default label
        node = new_node(ND_CASE);
        node->lhs = stmt();

        // update node of currently parsing switch statement
        current_switch->default_case = node;
    }
    else if(consume_reserved("do"))
    {
        node = new_node(ND_DO);

        // parse loop body
        node->lhs = stmt();

        expect_reserved("while");
        expect_reserved("(");

        // parse loop condition
        node->cond = expr();

        expect_reserved(")");
        expect_reserved(";");
    }
    else if(consume_reserved("for"))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        node = new_node(ND_FOR);
        expect_reserved("(");

        // parse clause-1
        if(!consume_reserved(";"))
        {
            node->preexpr = expr();
            expect_reserved(";");
        }

        // parse expression-2
        if(!consume_reserved(";"))
        {
            node->cond = expr();
            expect_reserved(";");
        }

        // parse expression-3
        if(!consume_reserved(")"))
        {
            node->postexpr = expr();
            expect_reserved(")");
        }

        // parse loop body
        node->lhs = stmt();
    }
    else if(consume_reserved("goto"))
    {
        node = new_node(ND_GOTO);
        node->ident = make_ident(expect_ident());
        expect_reserved(";");
    }
    else if(consume_reserved("if"))
    {
        node = new_node(ND_IF);
        expect_reserved("(");

        // parse condition
        node->cond = expr();

        expect_reserved(")");

        // parse statement in case of condition being true
        node->lhs = stmt();

        // parse statement in case of condition being false
        if(consume_reserved("else"))
        {
            node->rhs = stmt();
        }
    }
    else if(consume_reserved("return"))
    {
        node = new_node(ND_RETURN);
        if(!consume_reserved(";"))
        {
            // return statement with an expression
            node->lhs = expr();
            expect_reserved(";");
        }
    }
    else if(consume_reserved("switch"))
    {
        // save node of previous switch statement
        Node *prev_switch = current_switch;

        // parse controlling expression
        node = new_node(ND_SWITCH);
        expect_reserved("(");
        node->cond = expr();
        expect_reserved(")");

        // update node of currently parsing switch statement and parse body
        current_switch = node;
        node->lhs = stmt();

        // restore node of previous switch statement
        current_switch = prev_switch;
    }
    else if(consume_reserved("while"))
    {
        node = new_node(ND_WHILE);
        expect_reserved("(");

        // parse loop condition
        node->cond = expr();

        expect_reserved(")");

        // parse loop body
        node->lhs = stmt();
    }
    else if(consume_reserved(";"))
    {
        // null statement
        node = new_node(ND_NULL);
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
                node = new_node(ND_LABEL);
                node->lhs = stmt();
                node->ident = make_ident(token);
                goto stmt_end;
            }
            else
            {
                // resume the token since it is not a label
                set_token(saved_token);
            }
        }

        // expression statement
        node = expr();
        expect_reserved(";");
    }

stmt_end:
    return node;
}


/*
make a compound statement
```
compound-stmt ::= "{" (declaration | stmt)* "}"
```
*/
static Node *compound_stmt(void)
{
    expect_reserved("{");

    // parse declaration and/or statement until reaching '}'
    Node head = {};
    Node *cursor = &head;
    while(!consume_reserved("}"))
    {
        if(peek_typename())
        {
            // declaration
            cursor->next = declaration();
        }
        else
        {
            // statement
            cursor->next = stmt();
        }
        cursor = cursor->next;
    }

    return head.next;
}


/*
make a declaration
```
declaration ::= declaration-spec init-declarator-list ";"
```
*/
static Node *declaration(void)
{
    // parse declaration specifier
    Type *type = declaration_spec();

    // parse init-declarator-list
    Node *node = new_node(ND_DECL);
    node->body = init_declarator_list(type);

    expect_reserved(";");

    return node;
}


/*
make a init-declarator-list
```
init-declarator-list ::= init-declarator ("," init-declarator)*
```
*/
static Node *init_declarator_list(Type *type)
{
    Node *node = init_declarator(type);
    Node *cursor = node;

    while(consume_reserved(","))
    {
        cursor->next = init_declarator(type);
        cursor = cursor->next;
    }

    return node;
}


/*
make a init-declarator-list
```
init-declarator ::= declarator ("=" initializer)?
```
*/
static Node *init_declarator(Type *type)
{
    // parse declarator
    Token *token;
    type = declarator(type, &token);
    if(get_lvar(token) != NULL)
    {
        report_error(token->str, "duplicated declaration of '%s'\n", make_ident(token));
    }

    Node *node = new_node(ND_LVAR);
    node->type = type;
    node->lvar = current_function->locals = new_lvar(token, current_function->locals, type);

    // parse initializer
    if(consume_reserved("="))
    {
        node->lvar->init = initializer();
    }

    return node;
}


/*
make an initializer
```
initializer ::= assign
```
*/
static Node *initializer(void)
{
    return assign();
}


/*
make an expression
```
expr ::= assign ("," assign)*
```
*/
static Node *expr(void)
{
    Node *node = assign();

    // parse tokens while finding an assignment expression
    while(true)
    {
        if(consume_reserved(","))
        {
            node = new_node_binary(ND_COMMA, node, assign());
        }
        else
        {
            return node;
        }
    }
}


/*
make an assignment expression
```
assign ::= conditional (assign-op assign)?
assign-op ::= "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|="
```
*/
static Node *assign(void)
{
    Node *node = conditional();

    // parse assignment
    if(consume_reserved("="))
    {
        node = new_node_binary(ND_ASSIGN, node, assign());
    }
    else if(consume_reserved("*="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment *=");
        }

        node = new_node_binary(ND_MUL_EQ, lhs, rhs);
    }
    else if(consume_reserved("/="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment /=");
        }

        node = new_node_binary(ND_DIV_EQ, lhs, rhs);
    }
    else if(consume_reserved("%="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment %=");
        }

        node = new_node_binary(ND_MOD_EQ, lhs, rhs);
    }
    else if(consume_reserved("+="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(ND_ADD_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
        {
            node = new_node_binary(ND_PTR_ADD_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }
    }
    else if(consume_reserved("-="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(ND_SUB_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
        {
            node = new_node_binary(ND_PTR_SUB_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }
    }
    else if(consume_reserved("<<="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment <<=");
        }

        node = new_node_binary(ND_LSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved(">>="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment >>=");
        }

        node = new_node_binary(ND_RSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved("&="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment &=");
        }

        node = new_node_binary(ND_AND_EQ, lhs, rhs);
    }
    else if(consume_reserved("^="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment ^=");
        }

        node = new_node_binary(ND_XOR_EQ, lhs, rhs);
    }
    else if(consume_reserved("|="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment |=");
        }

        node = new_node_binary(ND_OR_EQ, lhs, rhs);
    }

    return node;
}


/*
make a conditional expression
```
conditional ::= logical-or-expr ("?" expr ":" conditional)?
```
*/
static Node *conditional(void)
{
    Node *node = logical_or_expr();

    if(consume_reserved("?"))
    {
        Node *ternary = new_node(ND_COND);

        ternary->cond = node;
        ternary->lhs = expr();
        expect_reserved(":");
        ternary->rhs = conditional();
        ternary->type = node->type;

        return ternary;
    }
    else
    {
        return node;
    }
}


/*
make a logical OR expression
```
logical-or-expr ::= logical-and-expr (|| logical-and-expr)*
```
*/
static Node *logical_or_expr(void)
{
    Node *node = logical_and_expr();

    // parse tokens while finding a bitwise logical AND expression
    while(true)
    {
        if(consume_reserved("||"))
        {
            node = new_node_binary(ND_LOG_OR, node, logical_and_expr());
        }
        else
        {
            return node;
        }
    }
}


/*
make a logical AND expression
```
logical-and-expr ::= or-expr (&& or-expr)*
```
*/
static Node *logical_and_expr(void)
{
    Node *node = or_expr();

    // parse tokens while finding a bitwise inclusive OR expression
    while(true)
    {
        if(consume_reserved("&&"))
        {
            node = new_node_binary(ND_LOG_AND, node, or_expr());
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise inclusive OR expression
```
or-expr ::= xor-expr (| xor-expr)*
```
*/
static Node *or_expr(void)
{
    Node *node = xor_expr();

    // parse tokens while finding a bitwise exclusive OR expression
    while(true)
    {
        if(consume_reserved("|"))
        {
            node = new_node_binary(ND_OR, node, xor_expr());
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise exclusive OR expression
```
xor-expr ::= and-expr (^ and-expr)*
```
*/
static Node *xor_expr(void)
{
    Node *node = and_expr();

    // parse tokens while finding a bitwise AND expression
    while(true)
    {
        if(consume_reserved("^"))
        {
            node = new_node_binary(ND_XOR, node, and_expr());
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise AND expression
```
and-expr ::= equality (& equality)*
```
*/
static Node *and_expr(void)
{
    Node *node = equality();

    // parse tokens while finding a equality expression
    while(true)
    {
        if(consume_reserved("&"))
        {
            node = new_node_binary(ND_AND, node, equality());
        }
        else
        {
            return node;
        }
    }
}


/*
make an equality expression
```
equality ::= relational ("==" relational | "!=" relational)*
```
*/
static Node *equality(void)
{
    Node *node = relational();

    // parse tokens while finding a relational expression
    while(true)
    {
        if(consume_reserved("=="))
        {
            node = new_node_binary(ND_EQ, node, relational());
        }
        else if(consume_reserved("!="))
        {
            node = new_node_binary(ND_NEQ, node, relational());
        }
        else
        {
            return node;
        }
    }
}


/*
make a relational expression
```
relational ::= shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
```
*/
static Node *relational(void)
{
    Node *node = shift();

    // parse tokens while finding a shift expression
    while(true)
    {
        if(consume_reserved("<"))
        {
            node = new_node_binary(ND_L, node, shift());
        }
        else if(consume_reserved("<="))
        {
            node = new_node_binary(ND_LEQ, node, shift());
        }
        else if(consume_reserved(">"))
        {
            node = new_node_binary(ND_L, shift(), node);
        }
        else if(consume_reserved(">="))
        {
            node = new_node_binary(ND_LEQ, shift(), node);
        }
        else
        {
            return node;
        }
    }
}


/*
make a shift expression
```
shift ::=  additive ("<<" additive | ">>" additive)*
```
*/
static Node *shift(void)
{
    Node *node = additive();

    // parse tokens while finding an additive expression
    while(true)
    {
        if(consume_reserved("<<"))
        {
            Node *lhs = node;
            Node *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_LSHIFT, lhs, rhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator <<");
            }
        }
        else if(consume_reserved(">>"))
        {
            Node *lhs = node;
            Node *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_RSHIFT, lhs, rhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator >>");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make an additive expression
```
additive ::= multiplicative ("+" multiplicative | "-" multiplicative)*
```
*/
static Node *additive(void)
{
    Node *node = multiplicative();

    // parse tokens while finding a multiplicative expression
    while(true)
    {
        if(consume_reserved("+"))
        {
            Node *lhs = node;
            Node *rhs = multiplicative();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_ADD, lhs, rhs);
            }
            else if(is_pointer_or_array(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_PTR_ADD, lhs, rhs);
            }
            else if(is_integer(lhs->type) && is_pointer_or_array(rhs->type))
            {
                node = new_node_binary(ND_PTR_ADD, rhs, lhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator +");
            }
        }
        else if(consume_reserved("-"))
        {
            Node *lhs = node;
            Node *rhs = multiplicative();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_SUB, lhs, rhs);
            }
            else if(is_pointer_or_array(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_PTR_SUB, lhs, rhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator -");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a multiplicative expression
```
multiplicative ::= cast ("*" cast | "/" cast | "%" cast)*
```
*/
static Node *multiplicative(void)
{
    Node *node = cast();

    // parse tokens while finding a cast expression
    while(true)
    {
        if(consume_reserved("*"))
        {
            node = new_node_binary(ND_MUL, node, cast());
        }
        else if(consume_reserved("/"))
        {
            node = new_node_binary(ND_DIV, node, cast());
        }
        else if(consume_reserved("%"))
        {
            node = new_node_binary(ND_MOD, node, cast());
        }
        else
        {
            return node;
        }
    }
}


/*
make a cast expression
```
cast ::= ("(" type-name ")")? unary
```
*/
static Node *cast(void)
{
    Node *node;

    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_typename())
        {
            Type *type = type_name();
            expect_reserved(")");
            node = new_node(ND_CAST);
            node->lhs = unary();
            node->type = type;
            goto cast_end;
        }
        else
        {
            set_token(saved_token);
        }
    }

    node = unary();

cast_end:
    return node;
}


/*
make an unary expression
```
unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | "sizeof" unary
        | "sizeof" "(" type-name ")"
unary-op ::= "&" | "*" | "+" | "-" | "~" | "!"
```
*/
static Node *unary(void)
{
    Node *node;

    if(consume_reserved("sizeof"))
    {
        Token *saved_token = get_token();
        if(consume_reserved("("))
        {
            if(peek_typename())
            {
                Type *type = type_name();
                node = new_node_num(type->size);
                expect_reserved(")");
                goto unary_end;
            }
            else
            {
                set_token(saved_token);
            }
        }

        Node *operand = unary();
        node = new_node_num(operand->type->size);
    }
    else if(consume_reserved("++"))
    {
        Node *operand = unary();

        if(is_integer(operand->type))
        {
            node = new_node_binary(ND_ADD_EQ, operand, new_node_num(1));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(ND_PTR_ADD_EQ, operand, new_node_num(1));
        }
        else
        {
            report_error(NULL, "bad operand for prefix increment operator ++");
        }
    }
    else if(consume_reserved("--"))
    {
        Node *operand = unary();

        if(is_integer(operand->type))
        {
            node = new_node_binary(ND_SUB_EQ, operand, new_node_num(1));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(ND_PTR_SUB_EQ, operand, new_node_num(1));
        }
        else
        {
            report_error(NULL, "bad operand for prefix decrement operator --");
        }
    }
    else if(consume_reserved("&"))
    {
        node = new_node_unary(ND_ADDR, unary());
    }
    else if (consume_reserved("*"))
    {
        node = new_node_unary(ND_DEREF, unary());
    }
    else if(consume_reserved("+"))
    {
        node = unary();
    }
    else if(consume_reserved("-"))
    {
        node = new_node_binary(ND_SUB, new_node_num(0), unary());
    }
    else if (consume_reserved("~"))
    {
        node = new_node_unary(ND_COMPL, unary());
    }
    else if (consume_reserved("!"))
    {
        node = new_node_unary(ND_NEG, unary());
    }
    else
    {
        node = postfix();
    }

unary_end:
    return node;
}


/*
make a postfix expression
```
postfix ::= primary ("[" expr "]" | "(" arg-expr-list? ")" | "++" | "--" )*
```
*/
static Node *postfix(void)
{
    Node *node = primary();

    // parse tokens while finding a postfix operator
    while(true)
    {
        if(consume_reserved("["))
        {
            // array subscripting
            Node *lhs;
            Node *index = expr();

            if(is_pointer_or_array(node->type) && is_integer(index->type))
            {
                lhs = new_node_binary(ND_PTR_ADD, node, index);
            }
            else if(is_integer(node->type) && is_pointer_or_array(index->type))
            {
                lhs = new_node_binary(ND_PTR_ADD, index, node);
            }
            else
            {
                report_error(NULL, "bad operand for [] operator\n");
            }

            node = new_node_unary(ND_DEREF, lhs);
            expect_reserved("]");
        }
        else if(consume_reserved("("))
        {
            // function call
            if((node->type->base == NULL) || (node->type->base->kind != TY_FUNC))
            {
                report_error(NULL, "expected function");
            }

            Node *func_node = new_node(ND_FUNC);
            if(!consume_reserved(")"))
            {
                func_node->args = arg_expr_list();
                expect_reserved(")");
            }
            func_node->type = node->type->base->base; // dereference pointer and get type of return value
            func_node->ident = node->ident;
            node = func_node;
        }
        else if(consume_reserved("++"))
        {
            // postfix increment
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix increment operator ++\n");
            }
            node = new_node_unary(ND_POST_INC, node);
        }
        else if(consume_reserved("--"))
        {
            // postfix decrement
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix decrement operator --\n");
            }
            node = new_node_unary(ND_POST_DEC, node);
        }
        else
        {
            return node;
        }
    }
}


/*
make an argument expression list
```
arg-expr-list ::= assign ("," assign)*
```
*/
static Node *arg_expr_list(void)
{
    Node *arg;
    Node *cursor = NULL;

    arg = assign();
    arg->next = cursor;
    cursor = arg;

    // parse arguments
    while(consume_reserved(","))
    {
        // append the argument at the head in order to push arguments in reverse order when generating assembler code
        arg = assign();
        arg->next = cursor;
        cursor = arg;
    }

    return cursor;
}


/*
make a primary expression
```
primary ::= ident
          | num
          | str
          | "(" expr ")"
```
*/
static Node *primary(void)
{
    // expression in brackets
    if(consume_reserved("("))
    {
        Node *node = expr();
 
        expect_reserved(")");
 
        return node;
    }

    // identifier
    Token *token;
    if(consume_token(TK_IDENT, &token))
    {
        // search function
        Function *func = get_function(token);
        if(func != NULL)
        {
            Node *node = new_node(ND_FUNC);
            node->type = new_type_pointer(func->type);
            node->ident = make_ident(token);
            return node;
        }

        // search global variable
        GVar *gvar = get_gvar(token);
        if(gvar != NULL)
        {
            Node *node = new_node(ND_GVAR);
            node->type = gvar->type;
            node->gvar = gvar;
            return node;
        }

        // search local variable
        LVar *lvar = get_lvar(token);
        if(lvar != NULL)
        {
            Node *node = new_node(ND_LVAR);
            node->type = lvar->type;
            node->lvar = lvar;
            return node;
        }

        if(peek_reserved("("))
        {
            // implicitly assume that the token denotes a function which returns int
            Node *node = new_node(ND_FUNC);
            node->type = new_type_pointer(new_type_function(new_type(TY_INT), NULL));
            node->ident = make_ident(token);
#if(WARN_IMPLICIT_DECLARATION_OF_FUNCTION == ENABLED)
            report_warning(token->str, "implicit declaration of function '%s'\n", make_ident(token));
#endif /* WARN_IMPLICIT_DECLARATION_OF_FUNCTION */
            return node;
        }
        else
        {
            report_error(token->str, "undefined variable '%s'", make_ident(token));
        }
    }

    // string-literal
    if(consume_token(TK_STR, &token))
    {
        Node *node = new_node(ND_GVAR);
        node->gvar = new_str(token);
        node->type = node->gvar->type;
        return node;
    }

    // number
    return new_node_num(expect_number());
}


/*
make a new node
*/
static Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->next = NULL;
    node->kind = kind;
    node->lhs = NULL;
    node->rhs = NULL;
    node->type = NULL;
    node->val = 0;
    node->gvar = NULL;
    node->lvar = NULL;
    node->cond = NULL;
    node->preexpr = NULL;
    node->postexpr = NULL;
    node->body = NULL;
    node->ident = NULL;
    node->args = NULL;

    return node;
}


/*
make a new node for unary operations
*/
static Node *new_node_unary(NodeKind kind, Node *lhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;

    switch(kind)
    {
    case ND_ADDR:
        node->type = new_type_pointer(lhs->type);
        break;

    case ND_DEREF:
        node->type = lhs->type->base;
        break;

    case ND_POST_INC:
    case ND_POST_DEC:
        node->type = lhs->type;
        break;

    case ND_COMPL:
    case ND_NEG:
    default:
        node->type = new_type(TY_INT);
        break;
    }

    return node;
}


/*
make a new node for binary operations
*/
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;

    switch(kind)
    {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NEQ:
    case ND_L:
    case ND_LEQ:
    case ND_AND:
    case ND_XOR:
    case ND_OR:
        if((lhs->type->kind == TY_LONG) || (rhs->type->kind == TY_LONG))
        {
            node->type = new_type(TY_LONG);
        }
        else
        {
            node->type = new_type(TY_INT);
        }
        break;

    case ND_PTR_ADD:
    case ND_PTR_SUB:
        node->type = lhs->type;
        break;

    case ND_ASSIGN:
        if(is_array(rhs->type))
        {
            // convert from array to pointer
            node->type = new_type_pointer(rhs->type->base);
        }
        else
        {
            node->type = lhs->type;
        }
        break;

    case ND_ADD_EQ:
    case ND_PTR_ADD_EQ:
    case ND_SUB_EQ:
    case ND_PTR_SUB_EQ:
    case ND_MUL_EQ:
    case ND_DIV_EQ:
    case ND_MOD_EQ:
    case ND_LSHIFT_EQ:
    case ND_RSHIFT_EQ:
    case ND_AND_EQ:
    case ND_XOR_EQ:
    case ND_OR_EQ:
        node->type = lhs->type;
        break;

    case ND_COMMA:
        node->type = rhs->type;
        break;

    case ND_LOG_AND:
    case ND_LOG_OR:
    default:
        node->type = new_type(TY_INT);
        break;
    }

    return node;
}


/*
make a new node for number
*/
static Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->type = new_type(TY_INT);
    node->val = val;

    return node;
}


/*
make a new global variable
*/
static GVar *new_gvar(const Token *token, GVar *cur_gvar, Type *type)
{
    GVar *gvar = calloc(1, sizeof(GVar));
    gvar->next = NULL;
    gvar->name = make_ident(token);
    gvar->type = type;
    gvar->content = NULL;
    gvar->init = NULL;
    cur_gvar->next = gvar;

    return gvar;
}


/*
make a new string-literal
* String-literal is regarded as a global variable.
*/
static GVar *new_str(const Token *token)
{
    GVar *str = calloc(1, sizeof(GVar));
    str->next = NULL;
    str->name = new_strlabel();
    str->type = new_type_array(new_type(TY_CHAR), token->len + 1);
    str->init = NULL;

    str->content = calloc(token->len + 1, sizeof(char));
    strncpy(str->content, token->str, token->len);

    current_gvar->next = str;
    current_gvar = current_gvar->next;

    return str;
}


/*
get an existing global variable
* If there exists a global variable with a given token, this function returns the variable.
* Otherwise, it returns NULL.
*/
static GVar *get_gvar(const Token *token)
{
    // search list of gocal variables
    for(GVar *gvar = gvar_list->next; gvar != NULL; gvar = gvar->next)
    {
        if((strlen(gvar->name) == token->len) && (strncmp(token->str, gvar->name, token->len) == 0))
        {
            return gvar;
        }
    }

    return NULL;
}


/*
make a new local variable
*/
static LVar *new_lvar(const Token *token, LVar *cur_lvar, Type *type)
{
    current_function->stack_size += type->size;

    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = cur_lvar;
    lvar->str = token->str;
    lvar->len = token->len;
    lvar->offset = current_function->stack_size;
    lvar->type = type;
    lvar->init = NULL;

    return lvar;
}


/*
get a function argument or an existing local variable
* If there exists a function argument or a local variable with a given token, this function returns the variable.
* Otherwise, it returns NULL.
*/
static LVar *get_lvar(const Token *token)
{
    // search list of function arguments
    for(LVar *arg = current_args; arg != NULL; arg = arg->next)
    {
        if((arg->len == token->len) && (strncmp(token->str, arg->str, token->len) == 0))
        {
            return arg;
        }
    }

    // search list of local variables
    for(LVar *lvar = current_function->locals; lvar != NULL; lvar = lvar->next)
    {
        if((lvar->len == token->len) && (strncmp(token->str, lvar->str, token->len) == 0))
        {
            return lvar;
        }
    }

    return NULL;
}


/*
make a new function
*/
static Function *new_function(const Token *token, Function *cur_func, Type *type, LVar *args, size_t stack_size)
{
    Function *new_func = calloc(1, sizeof(Function));

    // initialize the name
    new_func->name = make_ident(token);

    // initialize arguments
    new_func->args = args;

    // initialize type
    new_func->type = new_type_function(type->base, type->args);

    // initialize function body
    new_func->body = NULL;

    // initialize list of local variables
    new_func->locals = NULL;

    // initialize stack size
    new_func->stack_size = (stack_size + (stack_alignment_size - 1)) & ~(stack_alignment_size - 1); // align stack size

    // update list of functions
    cur_func->next = new_func;

    return new_func;
}


/*
get an existing function
* If there exists a function with a given token, this function returns the function.
* Otherwise, it returns NULL.
*/
static Function *get_function(const Token *token)
{
    // search list of function
    for(Function *func = function_list->next; func != NULL; func = func->next)
    {
        if((strlen(func->name) == token->len) && (strncmp(token->str, func->name, token->len) == 0))
        {
            return func;
        }
    }

    return NULL;
}


/*
make a new parameter
*/
static LVar *new_param(const Token *token, Type *type)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->str = token->str;
    lvar->len = token->len;
    lvar->type = type;
    lvar->init = NULL;

    return lvar;
}


/*
make a new label for string-literal
*/
static char *new_strlabel(void)
{
    // A label for string-literal is of the form "LS<number>", so the length of buffer should be more than 2 + 10 + 1.
    char *label = calloc(15, sizeof(char));

    sprintf(label, "LS%d", str_label);
    str_label++;

    return label;
}


/*
peek a function
*/
static bool peek_func(void)
{
    // save the currently parsing token
    Token *saved_token = get_token();

    // parse declaration specifier and declarator
    Type *base = declaration_spec();
    Token *token;
    base = declarator(base, &token);

    // check if a compound statement follows
    bool is_func = consume_reserved("{");

    // resume the saved token
    set_token(saved_token);

    return is_func;
}


/*
make a constant expression
*/
static long const_expr(void)
{
    return evaluate(conditional());
}


/*
evaluate a node
*/
static long evaluate(const Node *node)
{
    switch(node->kind)
    {
    case ND_ADD:
        return evaluate(node->lhs) + evaluate(node->rhs);

    case ND_SUB:
        return evaluate(node->lhs) - evaluate(node->rhs);

    case ND_MUL:
        return evaluate(node->lhs) * evaluate(node->rhs);

    case ND_DIV:
        return evaluate(node->lhs) / evaluate(node->rhs);

    case ND_MOD:
        return evaluate(node->lhs) % evaluate(node->rhs);

    case ND_LSHIFT:
        return evaluate(node->lhs) << evaluate(node->rhs);;

    case ND_RSHIFT:
        return evaluate(node->lhs) >> evaluate(node->rhs);

    case ND_EQ:
        return evaluate(node->lhs) == evaluate(node->rhs);

    case ND_NEQ:
        return evaluate(node->lhs) != evaluate(node->rhs);

    case ND_L:
        return evaluate(node->lhs) < evaluate(node->rhs);

    case ND_LEQ:
        return evaluate(node->lhs) <= evaluate(node->rhs);;

    case ND_AND:
        return evaluate(node->lhs) & evaluate(node->rhs);

    case ND_XOR:
        return evaluate(node->lhs) ^ evaluate(node->rhs);

    case ND_OR:
        return evaluate(node->lhs) | evaluate(node->rhs);;

    case ND_LOG_AND:
        return evaluate(node->lhs) && evaluate(node->rhs);

    case ND_LOG_OR:
        return evaluate(node->lhs) || evaluate(node->rhs);

    case ND_NUM:
        return node->val;

    default:
        report_error(NULL, "cannot evaluate");
        return 0;
    }
}
