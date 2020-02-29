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
static GVar *gvar(const Token *token, GVar *cur_gvar, Type *base);
static Function *func(const Token *token, Function *cur_func, Type *base);
static bool peek_typename(void);
static Type *declarator(Type *type, Token **token);
static Type *type_spec(void);
static Type *type_suffix(Type *type);
static Node *stmt(void);
static Node *declaration(void);
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
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *postfix(void);
static Node *primary(void);
static Node *new_node(NodeKind kind);
static Node *new_node_unary(NodeKind kind, Node *lhs);
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static Node *new_node_func(const Token *token);
static GVar *new_gvar(const Token *token, GVar *cur_gvar, Type *type);
static GVar *new_str(const Token *token);
static GVar *get_gvar(const Token *token);
static LVar *new_lvar(const Token *token, LVar *cur_lvar, Type *type);
static LVar *get_lvar(const Token *token);
static Function *new_function(const Token *token, Function *cur_func, Type *base);
static Function *get_function(const Token *token);
static char *new_strlabel(void);


// global variable
static GVar *gvar_list; // list of global variables
static GVar *current_gvar; // currently parsing global variable
static int str_label = 0; // label number of string-literal
static Function *function_list; // list of functions
static Function *current_function; // currently constructing function
static size_t stack_alignment_size = 8; // alignment size of function stack
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
    Function *func_cursor = &func_head;
    function_list = &func_head;

    while(!at_eof())
    {
        // parse identifier and base type
        Type *base = type_spec();
        Token *token;

        base = declarator(base, &token);
        if(peek_reserved("("))
        {
            // parse function
            func_cursor = func(token, func_cursor, base);
        }
        else
        {
            // parse global variable
            current_gvar = gvar(token, current_gvar, base);
        }
    }

    gvar_list = gvar_head.next;
    function_list = func_head.next;
}


/*
make a global variable
```
gvar ::= type-spec declarator ("=" initializer) ";"
```
*/
static GVar *gvar(const Token *token, GVar *cur_gvar, Type *base)
{
    current_gvar = new_gvar(token, cur_gvar, base);

    // parse initializer
    if(consume_reserved("="))
    {
        current_gvar->init = initializer();
    }
    expect_reserved(";");

    return current_gvar;
}


/*
make a function
```
func ::= type-spec declarator "(" (type-spec declarator ("," type-spec declarator)*)? ")" "{" stmt* "}"
```
*/
static Function *func(const Token *token, Function *cur_func, Type *base)
{
    // make a new function
    current_function = new_function(token, cur_func, base);

    // parse arguments
    expect_reserved("(");
    Type *arg_type = type_spec();
    if(arg_type != NULL)
    {
        LVar arg_head = {};
        LVar *arg_cursor = &arg_head;
        Token *arg_token;

        arg_type = declarator(arg_type, &arg_token);
        if(arg_type != NULL)
        {
            arg_cursor->next = new_lvar(arg_token, NULL, arg_type);
            arg_cursor = arg_cursor->next;
        }

        while(consume_reserved(","))
        {
            arg_type = type_spec();
            if(arg_type == NULL)
            {
                report_error(NULL, "expected type specifier\n");
            }

            arg_type = declarator(arg_type, &arg_token);
            arg_cursor->next = new_lvar(arg_token, NULL, arg_type);
            arg_cursor = arg_cursor->next;
        }
        current_function->args = arg_head.next;
    }
    expect_reserved(")");

    // parse body
    Node body_head = {};
    Node *body_cursor = &body_head;

    expect_reserved("{");
    while(!consume_reserved("}"))
    {
        body_cursor->next = stmt();
        body_cursor = body_cursor->next;
    }
    current_function->body = body_head.next;

    // align stack size
    current_function->stack_size = (current_function->stack_size + (stack_alignment_size - 1)) & ~(stack_alignment_size - 1);

    return current_function;
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
make a declarator
```
declarator ::= "*"* ident type-suffix
```
*/
static Type *declarator(Type *type, Token **token)
{
    // consume pointers
    while(consume_reserved("*"))
    {
        type = new_type_pointer(type);
    }

    // consume identifier
    if(!consume_token(TK_IDENT, token))
    {
        return NULL;
    }

    // consume type-suffix
    type = type_suffix(type);

    return type;
}


/*
make a type-spec
```
type-spec ::= "char" | "short" | "int" | "long"
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
make a type-suffix
```
type-suffix ::= ("[" num "]" | type-suffix)?
```
*/
static Type *type_suffix(Type *type)
{
    if(!consume_reserved("["))
    {
        return type;
    }

    size_t size = expect_number();
    expect_reserved("]");
    type = type_suffix(type);
    type = new_type_array(type, size);

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

    if(consume_reserved("{"))
    {
        // parse statements until reaching '}'
        Node head = {};
        Node *cursor = &head;
        while(!consume_reserved("}"))
        {
            cursor->next = stmt();
            cursor = cursor->next;
        }

        node = new_node(ND_BLOCK);
        node->body = head.next;
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
    else if(peek_typename())
    {
        // declaration
        node = declaration();
    }
    else if(consume_reserved(";"))
    {
        // null statement
        node = new_node(ND_NULL);
    }
    else
    {
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
                resume_token();
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
make a declaration
```
declaration ::= type-spec declarator ("=" initializer)? ";"
```
*/
static Node *declaration(void)
{
    // parse type-specifier
    Type *type = type_spec();

    // parse declarator
    Token *token;
    type = declarator(type, &token);
    if(get_lvar(token) != NULL)
    {
        report_error(token->str, "duplicated declaration of '%s'\n", make_ident(token));
    }

    Node *node = new_node(ND_DECL);
    node->type = type;
    node->lvar = current_function->locals = new_lvar(token, current_function->locals, type);

    // parse initializer
    if(consume_reserved("="))
    {
        node->lvar->init = initializer();
    }
    expect_reserved(";");

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
shift ::=  add ("<<" add | ">>" add)*
```
*/
static Node *shift(void)
{
    Node *node = add();

    // parse tokens while finding an additive expression
    while(true)
    {
        if(consume_reserved("<<"))
        {
            Node *lhs = node;
            Node *rhs = add();

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
            Node *rhs = mul();

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
add ::= mul ("+" mul | "-" mul)*
```
*/
static Node *add(void)
{
    Node *node = mul();

    // parse tokens while finding a multiplicative expression
    while(true)
    {
        if(consume_reserved("+"))
        {
            Node *lhs = node;
            Node *rhs = mul();

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
            Node *rhs = mul();

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
mul ::= unary ("*" unary | "/" unary | "%" unary)*
```
*/
static Node *mul(void)
{
    Node *node = unary();

    // parse tokens while finding an unary expression
    while(true)
    {
        if(consume_reserved("*"))
        {
            node = new_node_binary(ND_MUL, node, unary());
        }
        else if(consume_reserved("/"))
        {
            node = new_node_binary(ND_DIV, node, unary());
        }
        else if(consume_reserved("%"))
        {
            node = new_node_binary(ND_MOD, node, unary());
        }
        else
        {
            return node;
        }
    }
}


/*
make an unary expression
```
unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | sizeof unary
unary-op ::= "&" | "*" | "+" | "-" | "~" | "!"
```
*/
static Node *unary(void)
{
    Node *node;

    if(consume_reserved("sizeof"))
    {
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

    return node;
}


/*
make a postfix expression
```
postfix ::= primary ("[" expr "]")* ("++" | "--" )?
```
*/
static Node *postfix(void)
{
    Node *node = primary();

    // parse array subscripting
    while(consume_reserved("["))
    {
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

    // parse postfix increment and decrement operators
    if(consume_reserved("++"))
    {
        if(!(is_integer(node->type) || is_pointer(node->type)))
        {
            report_error(NULL, "bad operand for postfix increment operator ++\n");
        }

        node = new_node_unary(ND_POST_INC, node);
    }
    else if(consume_reserved("--"))
    {
        if(!(is_integer(node->type) || is_pointer(node->type)))
        {
            report_error(NULL, "bad operand for postfix decrement operator --\n");
        }
        node = new_node_unary(ND_POST_DEC, node);
    }

    return node;
}


/*
make a primary expression
```
primary ::= ident ("(" (assign ("," assign)*)? ")")?
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
        if(consume_reserved("("))
        {
            // function call
            Node *node = new_node_func(token);

            // parse arguments
            while(!consume_reserved(")"))
            {
                Node *arg = assign();

                // append the argument at the head in order to push arguments in reverse order when generating assembler code
                arg->next = node->args;
                node->args = arg;
                if(!consume_reserved(","))
                {
                    expect_reserved(")");
                    break;
                }
            }

            return node;
        }
        else
        {
            // variable
            GVar *gvar = get_gvar(token);
            if(gvar != NULL)
            {
                Node *node = new_node(ND_GVAR);
                node->type = gvar->type;
                node->gvar = gvar;
                return node;
            }

            LVar *lvar = get_lvar(token);
            if(lvar != NULL)
            {
                Node *node = new_node(ND_LVAR);
                node->type = lvar->type;
                node->lvar = lvar;
                return node;
            }

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
make a new node for function call
*/
static Node *new_node_func(const Token *token)
{
    Type *type;
    Function *func = get_function(token);
    if(func == NULL)
    {
        // implicitly assume that the function returns int
        type = new_type(TY_INT);
#if(WARN_IMPLICIT_DECLARATION_OF_FUNCTION == ENABLED)
        report_warning(token->str, "implicit declaration of function '%s'\n", make_ident(token));
#endif /* WARN_IMPLICIT_DECLARATION_OF_FUNCTION */
    }
    else
    {
        type = func->type;
    }

    Node *node = new_node(ND_FUNC);
    node->type = type;
    node->ident = make_ident(token);

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
    for(LVar *lvar = current_function->args; lvar != NULL; lvar = lvar->next)
    {
        if((lvar->len == token->len) && (strncmp(token->str, lvar->str, token->len) == 0))
        {
            return lvar;
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
static Function *new_function(const Token *token, Function *cur_func, Type *base)
{
    Function *new_func = calloc(1, sizeof(Function));

    // initialize the name
    new_func->name = make_ident(token);

    // initialize arguments
    new_func->args = NULL;

    // initialize type of return value
    new_func->type = base;

    // initialize function body
    new_func->body = NULL;

    // initialize list of local variables
    new_func->locals = NULL;

    // initialize stack size
    new_func->stack_size = 0;

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