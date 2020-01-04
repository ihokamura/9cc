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
static Function *func(void);
static Node *stmt(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);
static void generate_lvalue(const Node *node);
static void generate_func(const Function *func);
static void generate_node(const Node *node);
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static Node *new_node_func(const Token *tok);
static Node *new_node_lvar(const Token *tok);
static LVar *new_lvar(const Token *tok, LVar *cur_lvar, int offset);
static Function *new_function(const Token *tok);


// global variable
static const size_t LVAR_SIZE = 8; // size of a local variable in bytes
static const char *arg_registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}; // name of registers for function arguments
static const size_t ARG_REGISTERS_SIZE = sizeof(arg_registers) / sizeof(arg_registers[0]); // number of registers for function arguments
static Function *function_list; // list of functions
static Function *current_function; // currently constructing function
static int label_number; // serial number of labels


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

    // generate functions
    for(Function *func = function_list; func != NULL; func = func->next)
    {
        generate_func(func);
    }
}


/*
make a program
* program = func*
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
* func = ident "(" (ident ("," ident)*)? ")" "{" stmt* "}"
*/
static Function *func(void)
{
    // parse function name
    Token *tok = consume_ident();
    if(tok != NULL)
    {
        // parse arguments
        expect_operator("(");

        // make a new function
        current_function = new_function(tok);

        Token *arg;
        arg = consume_ident();
        if(arg != NULL)
        {
            current_function->args[0] = new_lvar(arg, NULL, LVAR_SIZE);
            current_function->argc++;
            current_function->stack_size += LVAR_SIZE;

            for(size_t i = 1; (i < ARG_REGISTERS_SIZE) && consume_operator(","); i++)
            {
                arg = consume_ident();
                if(arg != NULL)
                {
                    current_function->args[i] = new_lvar(arg, NULL, (i + 1) * LVAR_SIZE);
                    current_function->argc++;
                    current_function->stack_size += LVAR_SIZE;
                }
                else
                {
                    report_error(NULL, "expected argument\n");
                }
            }
        }
        expect_operator(")");

        // parse body
        expect_operator("{");

        Node head;
        head.next = NULL;
        Node *cursor = &head;
        while(!consume_operator("}"))
        {
            cursor->next = stmt();
            cursor = cursor->next;
        }
        current_function->body = head.next;

        return current_function;
    }
    else
    {
        report_error(NULL, "expected function definition\n");
        return NULL;
    }
}


/*
make a statement
* stmt = expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "for" "(" expr? ";" expr? ";" expr? ")" stmt | "{" stmt* "}"
*/
static Node *stmt(void)
{
    Node *node;

    if(consume_keyword(TK_IF))
    {
        expect_operator("(");
        node = calloc(1, sizeof(Node));
        node->cond = expr();
        expect_operator(")");
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
        expect_operator("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = expr();
        expect_operator(")");
        node->lhs = stmt();

        return node;
    }
    else if(consume_keyword(TK_FOR))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        expect_operator("(");
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
            expect_operator(";");
        }

        if(consume_operator(";"))
        {
            node->cond = NULL;
        }
        else
        {
            // parse expression-2
            node->cond = expr();
            expect_operator(";");
        }

        if(consume_operator(")"))
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
    else if(consume_operator("{"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        Node head;
        head.next = NULL;
        Node *cursor = &head;

        // parse statements until reaching '}'
        while(!consume_operator("}"))
        {
            cursor->next = stmt();
            cursor = cursor->next;
        }
        node->body = head.next;

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
* `primary = num | ident ("(" (expr ("," expr)*)? ")")? | "(" expr ")"`
*/
static Node *primary(void)
{
    // expression in brackets
    if(consume_operator("("))
    {
        Node *node = expr();
 
        expect_operator(")");
 
        return node;
    }

    // identifier
    Token *tok = consume_ident();
    if(tok != NULL)
    {
        if(consume_operator("("))
        {
            // function
            Node *node = new_node_func(tok);
            if(consume_operator(")"))
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
                for(size_t i = 1; (i < sizeof(node->args) / sizeof(node->args[0])) && consume_operator(","); i++)
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
generate assembler code for lvalue
*/
static void generate_lvalue(const Node *node)
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
generate assembler code of a function
*/
static void generate_func(const Function *func)
{
    // declarations
    printf(".global %s\n", func->name);
    printf("%s:\n", func->name);

    // prologue: allocate stack for arguments and local variables and copy arguments
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %ld\n", func->stack_size);
    for(size_t i = 0; i < func->argc; i++)
    {
        printf("  mov rax, rbp\n");
        printf("  sub rax, %ld\n", LVAR_SIZE * (i + 1));
        printf("  mov [rax], %s\n", arg_registers[i]);
    }

    // body
    for(Node *node = func->body; node != NULL; node = node->next)
    {
        generate_node(node);
    }

    // epilogue: save return value and release stack
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}


/*
generate assembler code of a node, which emulates stack machine
*/
static void generate_node(const Node *node)
{
    // note that `label_number` is saved and incremented at the top of each case-statement because this function is recursively called
    switch(node->kind)
    {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;

        case ND_LVAR:
            generate_lvalue(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;

        case ND_ASSIGN:
            generate_lvalue(node->lhs);
            generate_node(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;

        case ND_RETURN:
            generate_node(node->lhs);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;

        case ND_IF:
        {
            int number = label_number;
            label_number++;

            generate_node(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", number);
            generate_node(node->lhs);
            printf(".Lend%d:\n", number);
            return;
        }

        case ND_IFELSE:
        {
            int number = label_number;
            label_number++;

            generate_node(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lelse%d\n", number);
            generate_node(node->lhs);
            printf("  jmp .Lend%d\n", number);
            printf(".Lelse%d:\n", number);
            generate_node(node->rhs);
            printf(".Lend%d:\n", number);
            return;
        }

        case ND_WHILE:
        {
            int number = label_number;
            label_number++;

            printf(".Lbegin%d:\n", number);
            generate_node(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", number);
            generate_node(node->lhs);
            printf("  jmp .Lbegin%d\n", number);
            printf(".Lend%d:\n", number);
            return;
        }

        case ND_FOR:
        {
            int number = label_number;
            label_number++;

            if(node->preexpr != NULL)
            {
                generate_node(node->preexpr);
            }
            printf(".Lbegin%d:\n", number);
            if(node->cond != NULL)
            {
                generate_node(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je  .Lend%d\n", number);
            }
            generate_node(node->lhs);
            if(node->postexpr != NULL)
            {
                generate_node(node->postexpr);
            }
            printf("  jmp .Lbegin%d\n", number);
            printf(".Lend%d:\n", number);
            return;
        }

        case ND_BLOCK:
            for(Node *n = node->body; n != NULL; n = n->next)
            {
                generate_node(n);
                // pop result of current statement
                printf("  pop rax\n");
            }
            return;

        case ND_FUNC:
        {
            // evaluate arguments
            for(size_t i = 0; (i < ARG_REGISTERS_SIZE) && (node->args[i] != NULL); i++)
            {
                generate_node(node->args[i]);
                printf("  pop %s\n", arg_registers[i]);
            }

            int number = label_number;
            label_number++;

            // note that x86-64 ABI requires rsp to be a multiple of 16 before function calls
            printf("  mov rax, rsp\n");
            printf("  and rax, 0xF\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lbegin%d\n", number);
            // case 1: rsp has not been aligned yet
            printf("  sub rsp, 8\n");
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->ident);
            printf("  add rsp, 8\n");
            printf("  jmp .Lend%d\n", number);
            printf(".Lbegin%d:\n", number);
            // case 2: rsp has already been aligned
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->ident);
            printf(".Lend%d:\n", number);
            printf("  push rax\n");
            return;
        }

        default:
            break;
    }

    // compile LHS and RHS
    generate_node(node->lhs);
    generate_node(node->rhs);

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
            printf("  movzb rax, al\n");
            break;

        case ND_NEQ:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;

        case ND_L:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;

        case ND_LEQ:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
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
make a new node for function call
*/
static Node *new_node_func(const Token *tok)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_FUNC;
    node->ident = calloc(tok->len, (sizeof(char) + 1));
    strncpy(node->ident, tok->str, tok->len);

    return node;
}


/*
make a new node for local variable
*/
static Node *new_node_lvar(const Token *tok)
{
    int offset;
    LVar *lvar = current_function->locals;
    while(lvar != NULL)
    {
        if((lvar->len == tok->len) && (strncmp(tok->str, lvar->str, lvar->len) == 0))
        {
            // find local variable in the list
            offset = lvar->offset;
            break;
        }
        else
        {
            lvar = lvar->next;
        }
    }

    if(lvar == NULL)
    {
        if(current_function->locals == NULL)
        {
            offset = LVAR_SIZE;
        }
        else
        {
            offset = current_function->locals->offset + LVAR_SIZE;
        }
        current_function->locals = new_lvar(tok, current_function->locals, offset);
        current_function->stack_size += LVAR_SIZE;
    }

    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_LVAR;
    node->offset = offset;

    return node;
}


/*
make a new local variable
*/
static LVar *new_lvar(const Token *tok, LVar *cur_lvar, int offset)
{
    LVar *lvar = calloc(1, sizeof(LVar));

    lvar->next = cur_lvar;
    lvar->str = tok->str;
    lvar->len = tok->len;
    lvar->offset = offset;

    return lvar;
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