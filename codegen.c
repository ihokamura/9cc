/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* syntax tree constructor
* assembler code generator
*/

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// function prototype
static void program(void);
static Node *stmt(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);
static void gen(const Node *node);
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);


// global variable
static Node *codes[100]; // root nodes of syntax tree for each statements


/*
construct syntax tree
*/
void construct(void)
{
    program();
}


/*
generate assembler code
*/
void generate(void)
{
    // use Intel syntax
    printf(".intel_syntax noprefix\n");

    // start main function
    printf(".global _main\n");
    printf("_main:\n");

    // prologue: allocate stack for 26 local variables
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // body
    for(size_t i = 0; i < sizeof(codes) / sizeof(codes[0]) && codes[i] != NULL; i++)
    {
        // generate assembler code for each statements
        gen(codes[i]);

        // pop return value to avoid stack overflow
        printf("  pop rax\n");
    }
    
    // epilogue: release stack and exit main function
    // The return value is stored in rax.
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}


/*
make a program
* program = stmt*
*/
static void program(void)
{
    size_t loop_max = sizeof(codes) / sizeof(codes[0]) - 1;
    size_t i = 0;

    while(!at_eof() && (i < loop_max))
    {
        codes[i] = stmt();
        i++;
    }
    codes[i] = NULL;
}


/*
make a statement
* stmt = expr ";"
*/
static Node *stmt(void)
{
    Node *node = expr();

    expect(";");

    return node;
}


/*
make an expression
* expr = assign
*/
static Node *expr(void)
{
    return assign();
}


/*
make an assignment expression
* assign = equality ("=" assign)?
*/
static Node *assign(void)
{
    Node *node = equality();

    // parse assignment
    if(consume("="))
    {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}


/*
make an equality
* equality = relational ("==" relational | "!=" relational)*
*/
static Node *equality(void)
{
    Node *node = relational();

    // parse tokens while finding a relational expression
    while(true)
    {
        if(consume("=="))
        {
            node = new_node(ND_EQ, node, relational());
        }
        else if(consume("!="))
        {
            node = new_node(ND_NEQ, node, relational());
        }
        else
        {
            return node;
        }
    }
}


/*
make a relational expression
* relational = add ("<" add | "<=" add | ">" add | ">=" add)*
*/
static Node *relational(void)
{
    Node *node = add();

    // parse tokens while finding an addition term
    while(true)
    {
        if(consume("<"))
        {
            node = new_node(ND_L, node, add());
        }
        else if(consume("<="))
        {
            node = new_node(ND_LEQ, node, add());
        }
        else if(consume(">"))
        {
            node = new_node(ND_L, add(), node);
        }
        else if(consume(">="))
        {
            node = new_node(ND_LEQ, add(), node);
        }
        else
        {
            return node;
        }
    }
}


/*
make an addition term
* add = mul ("+" mul | "-" mul)*
*/
static Node *add(void)
{
    Node *node = mul();

    // parse tokens while finding a term
    while(true)
    {
        if(consume("+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume("-"))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}


/*
make a multiplication term
* mul = unary ("*" unary | "/" unary)*
*/
static Node *mul(void)
{
    Node *node = unary();

    // parse tokens while finding a primary
    while(true)
    {
        if(consume("*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if(consume("/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}


/*
make an unary
* unary = ("+" | "-")? primary
*/
static Node *unary(void)
{
    // parse sign
    if(consume("+"))
    {
        return primary();
    }
    else if(consume("-"))
    {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    else
    {
        return primary();
    }
}


/*
make a primary
* primary = num | ident | "(" expr ")"
*/
static Node *primary(void)
{
    // expression in brackets
    if(consume("("))
    {
        Node *node = expr();
 
        expect(")");
 
        return node;
    }

    // identifier
    // It is currently assumed that
    // * an identifier consists of one lower-case character
    // * character code of the environment is the ASCII code
    Token *tok = consume_ident();
    if(tok != NULL)
    {
        Node *node = calloc(1, sizeof(Node));

        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;

        return node;
    }

    // number
    return new_node_num(expect_number());
}


/*
generate assembler code for lvalue
*/
static void gen_lvalue(const Node *node)
{
    if(node->kind != ND_LVAR)
    {
        report_error(NULL, "lvalue of assignment is not a variable.");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

/*
generate assembler code which emulates stack machine
*/
static void gen(const Node *node)
{
    switch(node->kind)
    {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;

        case ND_LVAR:
            gen_lvalue(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;

        case ND_ASSIGN:
            gen_lvalue(node->lhs);
            gen(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;

        default:
            break;
    }

    // compile LHS and RHS
    gen(node->lhs);
    gen(node->rhs);

    // pop RHS to rdi and LHS to rax
    printf("  pop rdi\n");
    printf("  pop rax\n");

    // execute operation
    switch(node->kind)
    {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;

        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;

        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;

        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;

        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzx rax, al\n");
            break;

        case ND_NEQ:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzx rax, al\n");
            break;

        case ND_L:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzx rax, al\n");
            break;

        case ND_LEQ:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzx rax, al\n");
            break;

        default:
            break;
    }

    // push return value
    printf("  push rax\n");
}


/*
make a new node for non-number
*/
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}


/*
make a new node for number
*/
static Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->val = val;

    return node;
}