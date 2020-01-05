/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* assembler code generator
*/

#include <stdio.h>

#include "9cc.h"


// function prototype
static void generate_lvalue(const Node *node);
static void generate_func(const Function *func);
static void generate_node(const Node *node);


// global variable
const size_t LVAR_SIZE = 8; // size of a local variable in bytes
const char *arg_registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}; // name of registers for function arguments
const size_t ARG_REGISTERS_SIZE = sizeof(arg_registers) / sizeof(arg_registers[0]); // number of registers for function arguments
static int label_number; // serial number of labels


/*
generate assembler code
*/
void generate(Function *functions)
{
    // use Intel syntax
    printf(".intel_syntax noprefix\n");

    // generate functions
    for(Function *func = functions; func != NULL; func = func->next)
    {
        generate_func(func);
    }
}


/*
generate assembler code for lvalue
*/
static void generate_lvalue(const Node *node)
{
    if(node->kind != ND_LVAR)
    {
        report_error(NULL, "expected lvalue");
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

        case ND_ADDR:
            generate_lvalue(node->lhs);
            return;

        case ND_DEREF:
            generate_node(node->lhs);
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
            if(node->rhs == NULL)
            {
                // if ( expression ) statement
                printf("  je  .Lend%d\n", number);
                generate_node(node->lhs);
                printf(".Lend%d:\n", number);
            }
            else
            {
                // if ( expression ) statement else statement
                printf("  je  .Lelse%d\n", number);
                generate_node(node->lhs);
                printf("  jmp .Lend%d\n", number);
                printf(".Lelse%d:\n", number);
                generate_node(node->rhs);
                printf(".Lend%d:\n", number);
            }
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

        case ND_DO:
        {
            int number = label_number;
            label_number++;

            printf(".Lbegin%d:\n", number);
            generate_node(node->lhs);
            generate_node(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  jne .Lbegin%d\n", number);
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