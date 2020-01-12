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
static Node *new_node_lvar(const Token *token);
static Node *new_node_func(const Token *token);
static LVar *new_lvar(const Token *token, LVar *cur_lvar, int offset, Type *type);
static LVar *get_lvar(const Token *token);
static Function *new_function(const Token *token);


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
```
program ::= func*
```
*/
static void program(void)
{
    Function head = {};
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
```
func ::= "int" declarator "(" ("int" declarator ("," "int" declarator)*)? ")" "{" stmt* "}"
```
*/
static Function *func(void)
{
    // parse function name and type of return value
    Type *ret_type;
    Token *func_token;

    expect_reserved("int");
    expect_declarator(&ret_type, &func_token);
    current_function = new_function(func_token);

    // parse arguments
    expect_reserved("(");
    if(consume_reserved("int"))
    {
        Type *arg_type;
        Token *arg_token;

        expect_declarator(&arg_type, &arg_token);
        current_function->args[0] = new_lvar(arg_token, NULL, LVAR_SIZE, arg_type);
        current_function->argc++;
        current_function->stack_size += LVAR_SIZE;

        for(size_t i = 1; (i < ARG_REGISTERS_SIZE) && consume_reserved(","); i++)
        {
            expect_reserved("int");
            expect_declarator(&arg_type, &arg_token);
            current_function->args[i] = new_lvar(arg_token, NULL, (i + 1) * LVAR_SIZE, arg_type);
            current_function->argc++;
            current_function->stack_size += LVAR_SIZE;
        }
    }
    expect_reserved(")");

    // parse body
    Node head = {};
    Node *cursor = &head;

    expect_reserved("{");
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
```
stmt ::= declaration |
         expr ";" |
         "{" stmt* "}" |
         "do" stmt "while" "(" expr ")" ";" |
         "for" "(" expr? ";" expr? ";" expr? ")" stmt |
         "if" "(" expr ")" stmt ("else" stmt)? |
         "return" expr ";" |
         "while" "(" expr ")" stmt
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

        return node;
    }
    if(consume_reserved("do"))
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

        return node;
    }
    else if(consume_reserved("for"))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        Node *node = new_node(ND_FOR);
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

        return node;
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

        return node;
    }
    else if(consume_reserved("return"))
    {
        node = new_node(ND_RETURN);
        node->lhs = expr();
        expect_reserved(";");

        return node;
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

        return node;
    }
    else
    {
        if(consume_reserved("int"))
        {
            // declaration
            node = declaration();
            expect_reserved(";");

            return node;
        }
        else
        {
            // expression statement
            node = expr();
            expect_reserved(";");

            return node;
        }
    }
}


/*
make a declaration
```
declaration ::= "int" declarator ";"
```
*/
static Node *declaration(void)
{
    // parse declarator
    Type *type;
    Token *token;

    expect_declarator(&type, &token);
    if(get_lvar(token) != NULL)
    {
        report_error(NULL, "duplicated declaration\n");
    }

    current_function->stack_size += LVAR_SIZE;
    current_function->locals = new_lvar(token, current_function->locals, current_function->stack_size, type);

    Node *node = new_node(ND_DECL);
    node->lvar = current_function->locals;

    return node;
}


/*
make an expression
```
expr ::= assign
```
*/
static Node *expr(void)
{
    return assign();
}


/*
make an assignment expression
```
assign ::= equality ("=" assign)?
```
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
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
```
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
```
add ::= mul ("+" mul | "-" mul)*
```
*/
static Node *add(void)
{
    Node *node = mul();

    // parse tokens while finding a term
    while(true)
    {
        if(consume_reserved("+"))
        {
            Node *lhs = node;
            Node *rhs = mul();

            if(is_pointer(lhs) && !is_pointer(rhs))
            {
                node = new_node_binary(ND_PTR_ADD, lhs, rhs);
            }
            else if(!is_pointer(lhs) && is_pointer(rhs))
            {
                node = new_node_binary(ND_PTR_ADD, rhs, lhs);
            }
            else
            {
                node = new_node_binary(ND_ADD, lhs, rhs);
            }
        }
        else if(consume_reserved("-"))
        {
            Node *lhs = node;
            Node *rhs = mul();

            if(is_pointer(lhs) && !is_pointer(rhs))
            {
                node = new_node_binary(ND_PTR_SUB, lhs, rhs);
            }
            else
            {
                node = new_node_binary(ND_SUB, lhs, rhs);
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a multiplication term
```
mul ::= unary ("*" unary | "/" unary)*
```
*/
static Node *mul(void)
{
    Node *node = unary();

    // parse tokens while finding an unary
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
```
unary ::= unary ::= sizeof unary | ("+" | "-")? primary | "&" unary | "*" unary
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
    else if(consume_reserved("&"))
    {
        node = new_node(ND_ADDR);
        node->lhs = unary();
        node->type = new_type(TY_PTR);
        node->type->ptr_to = node->lhs->type;
    }
    else if (consume_reserved("*"))
    {
        node = new_node(ND_DEREF);
        node->lhs = unary();
        node->type = node->lhs->type->ptr_to;
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
```
primary ::= num | ident ("(" (expr ("," expr)*)? ")")? | "(" expr ")"
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
    Token *ident = consume_ident();
    if(ident != NULL)
    {
        if(consume_reserved("("))
        {
            // function
            Node *node = new_node_func(ident);
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
                expect_reserved(")");
            }

            return node;
        }
        else
        {
            // local variable
            return new_node_lvar(ident);
        }
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
    case ND_ASSIGN:
        node->type = new_type(TY_INT);
        break;

    case ND_PTR_ADD:
    case ND_PTR_SUB:
        node->type = new_type(TY_PTR);
        break;
    
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
make a new node for local variable
*/
static Node *new_node_lvar(const Token *token)
{
    LVar *lvar = get_lvar(token);
    if(lvar == NULL)
    {
        report_error(NULL, "undefined variable");
    }

    Node *node = new_node(ND_LVAR);
    node->type = lvar->type;
    node->lvar = lvar;

    return node;
}


/*
make a new node for function call
*/
static Node *new_node_func(const Token *token)
{
    Node *node = new_node(ND_FUNC);
    node->ident = calloc(token->len, (sizeof(char) + 1));
    strncpy(node->ident, token->str, token->len);

    return node;
}


/*
make a new local variable
*/
static LVar *new_lvar(const Token *token, LVar *cur_lvar, int offset, Type *type)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = cur_lvar;
    lvar->str = token->str;
    lvar->len = token->len;
    lvar->offset = offset;
    lvar->type = type;

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
    for(size_t i = 0; (i < ARG_REGISTERS_SIZE) && (current_function->args[i] != NULL); i++)
    {
        LVar *lvar = current_function->args[i];
        if((lvar->len == token->len) && (strncmp(token->str, lvar->str, lvar->len) == 0))
        {
            return lvar;
        }
    }

    // search list of local variables
    for(LVar *lvar = current_function->locals; lvar != NULL; lvar = lvar->next)
    {
        if((lvar->len == token->len) && (strncmp(token->str, lvar->str, lvar->len) == 0))
        {
            return lvar;
        }
    }

    return NULL;
}


/*
make a new function
*/
static Function *new_function(const Token *token)
{
    Function *new_func = calloc(1, sizeof(Function));

    // initialize the name
    new_func->name = calloc(token->len + 1, sizeof(char));
    strncpy(new_func->name, token->str, token->len);
    new_func->name[token->len] = '\0';

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