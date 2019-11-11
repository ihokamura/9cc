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
static Node *new_node_lvar(const Token *tok);
static int get_offset(const Token *tok);


// global variable
static Node *codes[100]; // root nodes of syntax tree for each statements
static LVar *locals; // list of local variables
static int locals_size; // number of local variables
static int label_number; // serial number of labels


/*
construct syntax tree
*/
void construct(void)
{
    // initialize list of local variables (make a dummy node)
    locals = calloc(1, sizeof(LVar));
    locals->next = NULL;
    locals->offset = 0;

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

    // prologue: allocate stack for local variables
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", 8 * locals_size);

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
* stmt = expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "for" "(" expr? ";" expr? ";" expr? ")" stmt
*/
static Node *stmt(void)
{
    Node *node;

    if(consume_keyword(TK_IF))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->cond = expr();
        expect(")");
        node->lhs = stmt();
        if(consume_keyword(TK_ELSE))
        {
            node->kind = ND_IFELSE;
            node->rhs = stmt();
        }
        else
        {
            node->kind = ND_IF;
        }

        return node;
    }
    else if(consume_keyword(TK_WHILE))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = expr();
        expect(")");
        node->lhs = stmt();

        return node;
    }
    else if(consume_keyword(TK_FOR))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;

        if(consume_operator(";"))
        {
            node->preexpr = NULL;
        }
        else
        {
            // parse clause-1
            node->preexpr = expr();
            expect(";");
        }

        if(consume_operator(";"))
        {
            node->cond = NULL;
        }
        else
        {
            // parse expression-2
            node->cond = expr();
            expect(";");
        }

        if(consume_operator(")"))
        {
            node->postexpr = NULL;
        }
        else
        {
            // parse expression-3
            node->postexpr = expr();
            expect(")");
        }

        // parse loop body
        node->lhs = stmt();

        return node;
    }
    else if(consume_keyword(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    }
    else
    {
        node = expr();
    }

    if(!consume_operator(";"))
    {
        report_error(NULL, "expected ';'\n");
    }

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
    if(consume_operator("="))
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
        if(consume_operator("=="))
        {
            node = new_node(ND_EQ, node, relational());
        }
        else if(consume_operator("!="))
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
        if(consume_operator("<"))
        {
            node = new_node(ND_L, node, add());
        }
        else if(consume_operator("<="))
        {
            node = new_node(ND_LEQ, node, add());
        }
        else if(consume_operator(">"))
        {
            node = new_node(ND_L, add(), node);
        }
        else if(consume_operator(">="))
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
        if(consume_operator("+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume_operator("-"))
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
        if(consume_operator("*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if(consume_operator("/"))
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
    if(consume_operator("+"))
    {
        return primary();
    }
    else if(consume_operator("-"))
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
    if(consume_operator("("))
    {
        Node *node = expr();
 
        expect(")");
 
        return node;
    }

    // identifier
    Token *tok = consume_ident();
    if(tok != NULL)
    {
        return new_node_lvar(tok);
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

        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;

        case ND_IF:
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", label_number);
            gen(node->lhs);
            printf(".Lend%d:\n", label_number);
            label_number++;
            return;

        case ND_IFELSE:
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lelse%d\n", label_number);
            gen(node->lhs);
            printf("  jmp .Lend%d\n", label_number);
            printf(".Lelse%d:\n", label_number);
            gen(node->rhs);
            printf(".Lend%d:\n", label_number);
            label_number++;
            return;

        case ND_WHILE:
            printf(".Lbegin%d:\n", label_number);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", label_number);
            gen(node->lhs);
            printf("  jmp .Lbegin%d\n", label_number);
            printf(".Lend%d:\n", label_number);
            label_number++;
            return;

        case ND_FOR:
            if(node->preexpr != NULL)
            {
                gen(node->preexpr);
            }
            printf(".Lbegin%d:\n", label_number);
            if(node->cond != NULL)
            {
                gen(node->cond);
            }
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", label_number);
            gen(node->lhs);
            if(node->postexpr != NULL)
            {
                gen(node->postexpr);
            }
            printf("  jmp .Lbegin%d\n", label_number);
            printf(".Lend%d:\n", label_number);
            label_number++;
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


/*
make a new node for local variable
*/
static Node *new_node_lvar(const Token *tok)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_LVAR;
    node->offset = get_offset(tok);

    return node;
}


/*
get offset of local variable from base pointer
*/
static int get_offset(const Token *tok)
{
    for(LVar *lvar = locals; lvar != NULL; lvar = lvar->next)
    {
        if(
            (lvar->len == tok->len) && 
            (strncmp(tok->str, lvar->name, lvar->len) == 0)
            )
        {
            // find local variable in the list
            return lvar->offset;
        }
    }

    // add local variable to the list
    LVar *lvar = calloc(1, sizeof(LVar));

    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->offset = locals->offset + 8;
    locals = lvar;
    locals_size++;

    return lvar->offset;
}