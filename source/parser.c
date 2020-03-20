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


// type definition
typedef struct VarScope VarScope;
struct VarScope {
    VarScope *next; // next element
    Variable *var;  // visible variable
    int depth;      // depth of scope
};


// function prototype
static void prg(void);
static void gvar(void);
static void func(void);
static Type *declaration_spec(void);
static Type *declarator(Type *type, Token **token);
static Type *parameter_list(Variable **arg_vars);
static Type *parameter_declaration(Variable **arg_var);
static Type *type_name(void);
static Type *type_spec(void);
static Type *pointer(Type *base);
static Type *direct_declarator(Type *type, Token **token);
static Node *stmt(void);
static Node *compound_stmt(void);
static Node *declaration(bool is_local);
static Node *init_declarator_list(Type *type, bool is_local);
static Node *init_declarator(Type *type, bool is_local);
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
static Variable *new_var(char *name, Type *type, bool local);
static Variable *new_gvar(const Token *token, Type *type, bool entity);
static Variable *new_str(const Token *token);
static Variable *new_lvar(const Token *token, Type *type);
static Function *new_function(const Token *token, Type *type, Node *body);
static VarScope *enter_scope(void);
static void leave_scope(VarScope *scope);
static void push_scope(Variable *var);
static VarScope *find_scope(const Token *token);
static bool peek_typename(void);
static bool peek_func(void);
static char *new_strlabel(void);
static long const_expr(void);
static long evaluate(const Node *node);


// global variable
static int str_label = 0; // label number of string-literal
static Function *function_list; // list of functions
static Variable *gvar_list = NULL; // list of global variables
static Variable *args_list = NULL; // list of arguments of currently constructing function
static Variable *lvar_list = NULL; // list of local variables of currently constructing function
static const size_t stack_alignment_size = 8; // alignment size of function stack
static Node *current_switch = NULL; // currently parsing switch statement
static VarScope *var_scope = NULL; // list of variables visible in the current scope
static int scope_depth = 0; // depth of the current scope


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
    Function func_head = {};
    function_list = &func_head;
    Variable gvar_head = {};
    gvar_list = &gvar_head;

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

    function_list = func_head.next;
    gvar_list = gvar_head.next;
}


/*
make a global variable
```
gvar ::= declaration
```
*/
static void gvar(void)
{
    declaration(false);
}


/*
make a function
```
func ::= declaration-spec declarator compound-stmt
```
*/
static void func(void)
{
    // clear list of local variables
    lvar_list = NULL;

    // parse declaration specifier and declarator
    Type *type = declaration_spec();
    Token *token;
    type = declarator(type, &token);

    // make a function declarator
    new_gvar(token, new_type_function(type->base, type->args), false);

    // parse body
    Node *body = compound_stmt();

    // make a new function
    new_function(token, type, body);
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
static Type *parameter_list(Variable **arg_vars)
{
    Type *arg_types;
    Type arg_types_head = {};
    Type *arg_types_cursor = &arg_types_head;
    Variable arg_vars_head = {};
    Variable *arg_vars_cursor = &arg_vars_head;

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
static Type *parameter_declaration(Variable **arg_var)
{
    Type *arg_type = declaration_spec();
    Token *arg_token;

    arg_type = declarator(arg_type, &arg_token);
    *arg_var = new_var(make_ident(arg_token), arg_type, true);

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
            Variable *arg_vars = NULL;
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
            args_list = arg_vars;
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

    // save the current scope
    VarScope *scope = enter_scope();

    // parse declaration and/or statement until reaching '}'
    Node head = {};
    Node *cursor = &head;
    while(!consume_reserved("}"))
    {
        if(peek_typename())
        {
            // declaration
            cursor->next = declaration(true);
        }
        else
        {
            // statement
            cursor->next = stmt();
        }
        cursor = cursor->next;
    }

    // restore the scope
    leave_scope(scope);

    return head.next;
}


/*
make a declaration
```
declaration ::= declaration-spec init-declarator-list ";"
```
*/
static Node *declaration(bool is_local)
{
    // parse declaration specifier
    Type *type = declaration_spec();

    // parse init-declarator-list
    Node *node = new_node(ND_DECL);
    node->body = init_declarator_list(type, is_local);

    expect_reserved(";");

    return node;
}


/*
make a init-declarator-list
```
init-declarator-list ::= init-declarator ("," init-declarator)*
```
*/
static Node *init_declarator_list(Type *type, bool is_local)
{
    Node *node = init_declarator(type, is_local);
    Node *cursor = node;

    while(consume_reserved(","))
    {
        cursor->next = init_declarator(type, is_local);
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
static Node *init_declarator(Type *type, bool is_local)
{
    // parse declarator
    Token *token;
    type = declarator(type, &token);

    // check duplicated declaration
    VarScope *scope = find_scope(token);
    if((scope != NULL) && (scope->depth == scope_depth))
    {
        report_error(token->str, "duplicated declaration of '%s'\n", make_ident(token));
    }

    // make a new node for variable
    Node *node = new_node(ND_VAR);
    node->type = type;
    if(is_local)
    {
        node->var = new_lvar(token, type);

        // parse initializer
        if(consume_reserved("="))
        {
            node->var->init = initializer();
        }
    }
    else
    {
        bool emit = (type->kind != TY_FUNC);
        node->var = new_gvar(token, type, emit);

        // parse initializer
        if(consume_reserved("="))
        {
            node->var->init = initializer();
        }
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
        // search variable
        VarScope *scope = find_scope(token);
        if(scope != NULL)
        {
            Variable *var = scope->var;
            if(var->type->kind == TY_FUNC)
            {
                Node *node = new_node(ND_FUNC);
                node->type = new_type_pointer(var->type);
                node->ident = make_ident(token);
                return node;
            }
            else
            {
                Node *node = new_node(ND_VAR);
                node->type = var->type;
                node->var = var;
                return node;
            }
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

        report_error(token->str, "undefined variable '%s'", make_ident(token));
    }

    // string-literal
    if(consume_token(TK_STR, &token))
    {
        Node *node = new_node(ND_VAR);
        node->var = new_str(token);
        node->type = node->var->type;
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
    node->var = NULL;
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
make a new variable
*/
static Variable *new_var(char *name, Type *type, bool local)
{
    Variable *var = calloc(1, sizeof(Variable));
    var->next = NULL;
    var->name = name;
    var->type = type;
    var->init = NULL;
    var->local = local;
    var->offset = 0;
    var->content = NULL;
    var->entity = false;

    push_scope(var);

    return var;
}


/*
make a new global variable
*/
static Variable *new_gvar(const Token *token, Type *type, bool entity)
{
    Variable *gvar = new_var(make_ident(token), type, false);
    gvar->entity = entity;
    gvar_list->next = gvar;
    gvar_list = gvar;

    return gvar;
}


/*
make a new string-literal
* String-literal is regarded as a global variable.
*/
static Variable *new_str(const Token *token)
{
    Variable *gvar = new_var(new_strlabel(), new_type_array(new_type(TY_CHAR), token->len + 1), false);
    gvar->content = calloc(token->len + 1, sizeof(char));
    strncpy(gvar->content, token->str, token->len);
    gvar->entity = true;
    gvar_list->next = gvar;
    gvar_list = gvar;

    return gvar;
}


/*
make a new local variable
*/
static Variable *new_lvar(const Token *token, Type *type)
{
    Variable *lvar = new_var(make_ident(token), type, true);
    lvar->next = lvar_list;
    lvar_list = lvar;

    return lvar;
}


/*
make a new function
*/
static Function *new_function(const Token *token, Type *type, Node *body)
{
    Function *new_func = calloc(1, sizeof(Function));
    new_func->name = make_ident(token);
    new_func->type = new_type_function(type->base, type->args);
    new_func->body = body;

    // accumulate stack size and set offset of arguments and local variables
    size_t stack_size = 0;
    if(type->args->kind != TY_VOID)
    {
        for(Variable *arg = args_list; arg != NULL; arg = arg->next)
        {
            stack_size += arg->type->size;
            arg->offset = stack_size;
        }
    }
    for(Variable *arg = lvar_list; arg != NULL; arg = arg->next)
    {
        stack_size += arg->type->size;
        arg->offset = stack_size;
    }

    // align stack size
    new_func->stack_size = (stack_size + (stack_alignment_size - 1)) & ~(stack_alignment_size - 1);

    // save list of arguments and list of local variables
    new_func->args = args_list;
    new_func->locals = lvar_list;

    // update list of functions
    function_list->next = new_func;
    function_list = new_func;

    return new_func;
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
push a variable to the current scope
*/
static void push_scope(Variable *var)
{
    VarScope *scope = calloc(1, sizeof(VarScope));
    scope->next = var_scope;
    scope->var = var;
    scope->depth = scope_depth;
    var_scope = scope;
}


/*
enter a new scope
*/
static VarScope *enter_scope(void)
{
    scope_depth++;
    return var_scope;
}


/*
leave the current scope
*/
static void leave_scope(VarScope *scope)
{
    scope_depth--;
    var_scope = scope;
}


/*
find a variable in the current scope
* If there exists a visible variable with a given token, this function returns the scope.
* Otherwise, it returns NULL.
*/
static VarScope *find_scope(const Token *token)
{
    // search list of variables visible in the current scope
    for(VarScope *scope = var_scope; scope != NULL; scope = scope->next)
    {
        if((strlen(scope->var->name) == token->len) && (strncmp(token->str, scope->var->name, token->len) == 0))
        {
            return scope;
        }
    }

    return NULL;
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
