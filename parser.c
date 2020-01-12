/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser (syntax tree constructor)
*/

#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// function prototype
static void program(void);
static Function *func(void);
static Node *stmt(void);
static Node *declaration(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);
static Node *new_node(NodeKind kind);
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static Node *new_node_lvar(const Token *tok);
static Node *new_node_func(const Token *tok);
static LVar *new_lvar(const Token *tok, LVar *cur_lvar, int offset, Type *type);
static LVar *get_lvar(const Token *tok);
static Function *new_function(const Token *tok);


// global variable
static Function *function_list; // list of functions
static Function *current_function; // currently constructing function


/*
construct syntax tree
*/
void construct(Function **functions)
{
    program();
    *functions = function_list;
}


/*
make a program
* program ::= func*
*/
static void program(void)
{
    Function head;
    head.next = NULL;
    Function *cursor = &head;
    while(!at_eof())
    {
        cursor->next = func();
        cursor = cursor->next;
    }

    function_list = head.next;
}


/*
make a function
* func ::= "int" declarator "(" ("int" declarator ("," "int" declarator)*)? ")" "{" stmt* "}"
*/
static Function *func(void)
{
    // parse function name and type of return value
    if(!consume_reserved("int"))
    {
        report_error(NULL, "expected 'int'\n");
    }

    Type *type;
    Token *tok;

    expect_declarator(&type, &tok);
    current_function = new_function(tok);

    // parse arguments
    expect_operator("(");
    if(consume_reserved("int"))
    {
        Type *type;
        Token *arg;

        expect_declarator(&type, &arg);
        current_function->args[0] = new_lvar(arg, NULL, LVAR_SIZE, type);
        current_function->argc++;
        current_function->stack_size += LVAR_SIZE;

        for(size_t i = 1; (i < ARG_REGISTERS_SIZE) && consume_reserved(","); i++)
        {
            if(!consume_reserved("int"))
            {
                report_error(NULL, "expected 'int'\n");
            }

            expect_declarator(&type, &arg);
            current_function->args[i] = new_lvar(arg, NULL, (i + 1) * LVAR_SIZE, type);
            current_function->argc++;
            current_function->stack_size += LVAR_SIZE;
        }
    }
    expect_operator(")");

    // parse body
    expect_operator("{");

    Node head = {};
    Node *cursor = &head;
    while(!consume_reserved("}"))
    {
        cursor->next = stmt();
        cursor = cursor->next;
    }
    current_function->body = head.next;

    return current_function;
}


/*
make a statement
* stmt ::= declaration | expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "do" stmt "while" "(" expr ")" ";" | "for" "(" expr? ";" expr? ";" expr? ")" stmt | "{" stmt* "}"
*/
static Node *stmt(void)
{
    Node *node;

    if(consume_reserved("if"))
    {
        node = new_node(ND_IF);
        expect_operator("(");
        node->cond = expr();
        expect_operator(")");
        node->lhs = stmt();
        if(consume_reserved("else"))
        {
            node->rhs = stmt();
        }

        return node;
    }
    else if(consume_reserved("while"))
    {
        node = new_node(ND_WHILE);
        expect_operator("(");
        node->cond = expr();
        expect_operator(")");
        node->lhs = stmt();

        return node;
    }
    else if(consume_reserved("do"))
    {
        node = new_node(ND_DO);
        node->lhs = stmt();
        if(!consume_reserved("while"))
        {
            report_error(NULL, "expected `while`\n");
        }
        expect_operator("(");
        node->cond = expr();
        expect_operator(")");
        expect_operator(";");

        return node;
    }
    else if(consume_reserved("for"))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        node = new_node(ND_FOR);
        expect_operator("(");

        if(consume_reserved(";"))
        {
            node->preexpr = NULL;
        }
        else
        {
            // parse clause-1
            node->preexpr = expr();
            expect_operator(";");
        }

        if(consume_reserved(";"))
        {
            node->cond = NULL;
        }
        else
        {
            // parse expression-2
            node->cond = expr();
            expect_operator(";");
        }

        if(consume_reserved(")"))
        {
            node->postexpr = NULL;
        }
        else
        {
            // parse expression-3
            node->postexpr = expr();
            expect_operator(")");
        }

        // parse loop body
        node->lhs = stmt();

        return node;
    }
    else if(consume_reserved("{"))
    {
        Node head = {};
        Node *cursor = &head;

        // parse statements until reaching '}'
        while(!consume_reserved("}"))
        {
            cursor->next = stmt();
            cursor = cursor->next;
        }

        node = new_node(ND_BLOCK);
        node->body = head.next;

        return node;
    }
    else if(consume_reserved("return"))
    {
        node = new_node(ND_RETURN);
        node->lhs = expr();
    }
    else if(consume_reserved("int"))
    {
        node = declaration();
    }
    else
    {
        node = expr();
    }

    if(!consume_reserved(";"))
    {
        report_error(NULL, "expected ';'\n");
    }

    return node;
}


/*
make a declaration
* declaration ::= "int" declarator ";"
*/
static Node *declaration(void)
{
    // parse declarator
    Type *type;
    Token *tok;

    expect_declarator(&type, &tok);
    if(get_lvar(tok) != NULL)
    {
        report_error(NULL, "duplicated declaration\n");
    }

    current_function->stack_size += LVAR_SIZE;
    current_function->locals = new_lvar(tok, current_function->locals, current_function->stack_size, type);

    Node *node = new_node(ND_DECL);
    node->lvar = current_function->locals;

    return node;
}


/*
make an expression
* expr ::= assign
*/
static Node *expr(void)
{
    return assign();
}


/*
make an assignment expression
* assign ::= equality ("=" assign)?
*/
static Node *assign(void)
{
    Node *node = equality();

    // parse assignment
    if(consume_reserved("="))
    {
        node = new_node_binary(ND_ASSIGN, node, assign());
    }

    return node;
}


/*
make an equality
* equality ::= relational ("==" relational | "!=" relational)*
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
* relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
*/
static Node *relational(void)
{
    Node *node = add();

    // parse tokens while finding an addition term
    while(true)
    {
        if(consume_reserved("<"))
        {
            node = new_node_binary(ND_L, node, add());
        }
        else if(consume_reserved("<="))
        {
            node = new_node_binary(ND_LEQ, node, add());
        }
        else if(consume_reserved(">"))
        {
            node = new_node_binary(ND_L, add(), node);
        }
        else if(consume_reserved(">="))
        {
            node = new_node_binary(ND_LEQ, add(), node);
        }
        else
        {
            return node;
        }
    }
}


/*
make an addition term
* add ::= mul ("+" mul | "-" mul)*
*/
static Node *add(void)
{
    Node *node = mul();

    // parse tokens while finding a term
    while(true)
    {
        if(consume_reserved("+"))
        {
            node = new_node_binary(ND_ADD, node, mul());
        }
        else if(consume_reserved("-"))
        {
            node = new_node_binary(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}


/*
make a multiplication term
* mul ::= unary ("*" unary | "/" unary)*
*/
static Node *mul(void)
{
    Node *node = unary();

    // parse tokens while finding a primary
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
        else
        {
            return node;
        }
    }
}


/*
make an unary
* unary ::= ("+" | "-")? primary | "&" unary | "*" unary
*/
static Node *unary(void)
{
    Node *node;

    if(consume_reserved("&"))
    {
        node = new_node(ND_ADDR);
        node->lhs = unary();
    }
    else if (consume_reserved("*"))
    {
        node = new_node(ND_DEREF);
        node->lhs = unary();
    }
    else if(consume_reserved("+"))
    {
        node = primary();
    }
    else if(consume_reserved("-"))
    {
        node = new_node_binary(ND_SUB, new_node_num(0), primary());
    }
    else
    {
        node = primary();
    }

    return node;
}


/*
make a primary
* primary ::= num | ident ("(" (expr ("," expr)*)? ")")? | "(" expr ")"
*/
static Node *primary(void)
{
    // expression in brackets
    if(consume_reserved("("))
    {
        Node *node = expr();
 
        expect_operator(")");
 
        return node;
    }

    // identifier
    Token *tok = consume_ident();
    if(tok != NULL)
    {
        if(consume_reserved("("))
        {
            // function
            Node *node = new_node_func(tok);
            if(consume_reserved(")"))
            {
                for(size_t i = 0; i < sizeof(node->args) / sizeof(node->args[0]); i++)
                {
                    node->args[i] = NULL;
                }
            }
            else
            {
                // arguments
                node->args[0] = expr();
                for(size_t i = 1; (i < sizeof(node->args) / sizeof(node->args[0])) && consume_reserved(","); i++)
                {
                    node->args[i] = expr();
                }
                expect_operator(")");
            }

            return node;
        }
        else
        {
            // local variable
            return new_node_lvar(tok);
        }
    }

    // number
    return new_node_num(expect_number());
}


/*
make a new type
*/
Type *new_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(Type));
    type->ty = kind;
    type->ptr_to = NULL;

    return type;
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
    node->val = 0;
    node->lvar = NULL;
    node->cond = NULL;
    node->preexpr = NULL;
    node->postexpr = NULL;
    node->body = NULL;
    node->ident = NULL;
    for(size_t i = 0; i < sizeof(node->args) / sizeof(node->args[0]); i++)
    {
        node->args[i] = NULL;
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

    return node;
}


/*
make a new node for number
*/
static Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;

    return node;
}


/*
make a new node for local variable
*/
static Node *new_node_lvar(const Token *tok)
{
    LVar *lvar = get_lvar(tok);
    if(lvar == NULL)
    {
        report_error(NULL, "undefined variable");
    }

    Node *node = new_node(ND_LVAR);
    node->lvar = lvar;

    return node;
}


/*
make a new node for function call
*/
static Node *new_node_func(const Token *tok)
{
    Node *node = new_node(ND_FUNC);
    node->ident = calloc(tok->len, (sizeof(char) + 1));
    strncpy(node->ident, tok->str, tok->len);

    return node;
}


/*
make a new local variable
*/
static LVar *new_lvar(const Token *tok, LVar *cur_lvar, int offset, Type *type)
{
    LVar *lvar = calloc(1, sizeof(LVar));

    lvar->next = cur_lvar;
    lvar->str = tok->str;
    lvar->len = tok->len;
    lvar->offset = offset;
    lvar->type = type;

    return lvar;
}


/*
get a function argument or an existing local variable
* If there exists a function argument or a local variable with a given token, this function returns the variable.
* Otherwise, it returns NULL.
*/
static LVar *get_lvar(const Token *tok)
{
    // search list of function arguments
    for(size_t i = 0; (i < ARG_REGISTERS_SIZE) && (current_function->args[i] != NULL); i++)
    {
        LVar *lvar = current_function->args[i];
        if((lvar->len == tok->len) && (strncmp(tok->str, lvar->str, lvar->len) == 0))
        {
            return lvar;
        }
    }

    // search list of local variables
    for(LVar *lvar = current_function->locals; lvar != NULL; lvar = lvar->next)
    {
        if((lvar->len == tok->len) && (strncmp(tok->str, lvar->str, lvar->len) == 0))
        {
            return lvar;
        }
    }

    return NULL;
}


/*
make a new function
*/
static Function *new_function(const Token *tok)
{
    Function *new_func = calloc(1, sizeof(Function));

    // initialize the name
    new_func->name = calloc(tok->len + 1, sizeof(char));
    strncpy(new_func->name, tok->str, tok->len);
    new_func->name[tok->len] = '\0';

    // initialize arguments
    for(size_t i = 0; i < ARG_REGISTERS_SIZE; i++)
    {
        new_func->args[i] = NULL;
    }
    new_func->argc = 0;

    // initialize function body
    new_func->body = NULL;

    // initialize list of local variables
    new_func->locals = NULL;

    // initialize stack size
    new_func->stack_size = 0;

    return new_func;
}