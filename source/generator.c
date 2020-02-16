/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* assembler code generator
*/

#include <stdarg.h>
#include <stdio.h>

#include "9cc.h"


// function prototype
static void generate_load(const Type *type);
static void generate_store(const Type *type);
static void generate_lvalue(const Node *node);
static void generate_gvar(const GVar *gvar);
static void generate_func(const Function *func);
static void generate_args(const Node *args);
static void generate_node(const Node *node);
static void put_instruction(const char *fmt, ...);


// global variable
const char *arg_registers8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"}; // name of 8-bit registers for function arguments
const char *arg_registers16[] = {"di", "si", "dx", "cx", "r8w", "r9w"}; // name of 16-bit registers for function arguments
const char *arg_registers32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"}; // name of 32-bit registers for function arguments
const char *arg_registers64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}; // name of 64-bit registers for function arguments
const size_t ARG_REGISTERS_SIZE = 6; // number of registers for function arguments
static int label_number; // serial number of labels


/*
generate assembler code
*/
void generate(const Program *program)
{
    // use Intel syntax
    put_instruction(".intel_syntax noprefix");

    // generate global variables
    put_instruction(".data");
    for(GVar *gvar = program->gvars; gvar != NULL; gvar = gvar->next)
    {
        generate_gvar(gvar);
    }

    // generate functions
    put_instruction(".text");
    for(Function *func = program->funcs; func != NULL; func = func->next)
    {
        generate_func(func);
    }
}


/*
generate assembler code to load value
*/
static void generate_load(const Type *type)
{
    put_instruction("  pop rax");
    if(type->size == 1)
    {
        put_instruction("  movsxb rax, byte ptr [rax]");
    }
    else if(type->size == 2)
    {
        put_instruction("  movsxw rax, word ptr [rax]");
    }
    else if(type->size == 4)
    {
        put_instruction("  movsxd rax, dword ptr [rax]");
    }
    else
    {
        put_instruction("  mov rax, [rax]");
    }
    put_instruction("  push rax");
}


/*
generate assembler code to store value
*/
static void generate_store(const Type *type)
{
    put_instruction("  pop rdi");
    put_instruction("  pop rax");
    if(type->size == 1)
    {
        put_instruction("  mov byte ptr [rax], dil");
    }
    else if(type->size == 2)
    {
        put_instruction("  mov word ptr [rax], di");
    }
    else if(type->size == 4)
    {
        put_instruction("  mov dword ptr [rax], edi");
    }
    else
    {
        put_instruction("  mov [rax], rdi");
    }
    put_instruction("  push rdi");
}


/*
generate assembler code for lvalue
*/
static void generate_lvalue(const Node *node)
{
    switch(node->kind)
    {
    case ND_GVAR:
        put_instruction("  lea rax, %s[rip]", node->gvar->name);
        put_instruction("  push rax");
        break;

    case ND_DECL:
    case ND_LVAR:
        put_instruction("  mov rax, rbp");
        put_instruction("  sub rax, %d", node->lvar->offset);
        put_instruction("  push rax");
        break;

    case ND_DEREF:
        generate_node(node->lhs);
        break;

    default:
        report_error(NULL, "expected lvalue");
        break;
    }
}


/*
generate assembler code of a global variable
*/
static void generate_gvar(const GVar *gvar)
{
    // put label
    put_instruction(".global %s", gvar->name);
    put_instruction("%s:", gvar->name);
    if(gvar->content != NULL)
    {
        // allocate memory with string-literal
        put_instruction("  .string \"%s\"\n", gvar->content);
    }
    else if(gvar->init != NULL)
    {
        Type *type = gvar->init->type;
        if(is_integer(type))
        {
            // allocate memory with an integer
            if(type->size == 1)
            {
                put_instruction("  .byte %d\n", gvar->init->val);
            }
            else if(type->size == 2)
            {
                put_instruction("  .value %d\n", gvar->init->val);
            }
            else if(type->size == 4)
            {
                put_instruction("  .long %d\n", gvar->init->val);
            }
            else
            {
                put_instruction("  .quad %d\n", gvar->init->val);
            }
        }
    }
    else
    {
        // allocate memory with zero
        put_instruction("  .zero %ld\n", gvar->type->size);
    }
}


/*
generate assembler code of a function
*/
static void generate_func(const Function *func)
{
    // declarations
    put_instruction(".global %s", func->name);
    put_instruction("%s:", func->name);

    // prologue: allocate stack for arguments and local variables and copy arguments
    put_instruction("  push rbp");
    put_instruction("  mov rbp, rsp");
    put_instruction("  sub rsp, %ld", func->stack_size);

    // arguments
    size_t argc = 0;
    for(LVar *arg = func->args; arg != NULL; arg = arg->next)
    {
        put_instruction("  mov rax, rbp");
        put_instruction("  sub rax, %d", arg->offset);

        if(argc < ARG_REGISTERS_SIZE)
        {
            // load argument from a register
            if(arg->type->size == 1)
            {
                put_instruction("  mov byte ptr [rax], %s", arg_registers8[argc]);
            }
            else if(arg->type->size == 2)
            {
                put_instruction("  mov word ptr [rax], %s", arg_registers16[argc]);
            }
            else if(arg->type->size == 4)
            {
                put_instruction("  mov dword ptr [rax], %s", arg_registers32[argc]);
            }
            else
            {
                put_instruction("  mov [rax], %s", arg_registers64[argc]);
            }
        }
        else
        {
            // load argument from the stack
            put_instruction("  push rax");
            put_instruction("  mov rax, rbp");
            put_instruction("  add rax, %d", (argc - ARG_REGISTERS_SIZE + 2) * 8);
            put_instruction("  push rax");
            generate_load(arg->type);
            generate_store(arg->type);
            put_instruction("  pop rdi");
        }

        // count number of arguments
        argc++;
    }

    // body
    for(Node *node = func->body; node != NULL; node = node->next)
    {
        generate_node(node);
    }

    // epilogue: save return value and release stack
    put_instruction("  pop rax");
    put_instruction("  mov rsp, rbp");
    put_instruction("  pop rbp");
    put_instruction("  ret");
}


/*
generate assembler code of function arguments
*/
static void generate_args(const Node *args)
{
    // push arguments
    size_t argc = 0;
    for(const Node *arg = args; arg != NULL; arg = arg->next)
    {
        generate_node(arg);
        argc++;
    }

    // pop up to ARG_REGISTERS_SIZE arguments to registers
    size_t argc_reg = (argc < ARG_REGISTERS_SIZE) ? argc : ARG_REGISTERS_SIZE;
    for(size_t i = 0; i < argc_reg; i++)
    {
        put_instruction("  pop %s", arg_registers64[i]);
    }
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
        put_instruction("  push %d", node->val);
        return;

    case ND_DECL:
        if(node->lvar->init != NULL)
        {
            generate_lvalue(node);
            generate_node(node->lvar->init);
            generate_store(node->type);
        }
        return;

    case ND_GVAR:
    case ND_LVAR:
        generate_lvalue(node);
        if(node->type->kind != TY_ARRAY)
        {
            generate_load(node->type);
        }
        return;

    case ND_ADDR:
        generate_lvalue(node->lhs);
        return;

    case ND_DEREF:
        generate_node(node->lhs);
        if(node->type->kind != TY_ARRAY)
        {
            generate_load(node->type);
        }
        return;

    case ND_ASSIGN:
        generate_lvalue(node->lhs);
        generate_node(node->rhs);
        generate_store(node->type);
        return;

    case ND_ADD_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        put_instruction("  pop rdi");
        put_instruction("  pop rax");
        put_instruction("  add rax, rdi");
        put_instruction("  push rax");
        generate_store(node->type);
        break;

    case ND_PTR_ADD_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        put_instruction("  pop rdi");
        put_instruction("  pop rax");
        put_instruction("  imul rdi, %ld", node->type->base->size);
        put_instruction("  add rax, rdi");
        put_instruction("  push rax");
        generate_store(node->type);
        break;

    case ND_SUB_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        put_instruction("  pop rdi");
        put_instruction("  pop rax");
        put_instruction("  sub rax, rdi");
        put_instruction("  push rax");
        generate_store(node->type);
        break;

    case ND_PTR_SUB_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        put_instruction("  pop rdi");
        put_instruction("  pop rax");
        put_instruction("  imul rdi, %ld", node->type->base->size);
        put_instruction("  sub rax, rdi");
        put_instruction("  push rax");
        generate_store(node->type);
        break;

    case ND_MUL_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        put_instruction("  pop rdi");
        put_instruction("  pop rax");
        put_instruction("  imul rax, rdi");
        put_instruction("  push rax");
        generate_store(node->type);
        break;

    case ND_DIV_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        put_instruction("  pop rdi");
        put_instruction("  pop rax");
        put_instruction("  cqo");
        put_instruction("  idiv rdi");
        put_instruction("  push rax");
        generate_store(node->type);
        break;

    case ND_RETURN:
        generate_node(node->lhs);
        put_instruction("  pop rax");
        put_instruction("  mov rsp, rbp");
        put_instruction("  pop rbp");
        put_instruction("  ret");
        return;

    case ND_IF:
    {
        int number = label_number;
        label_number++;

        generate_node(node->cond);
        put_instruction("  pop rax");
        put_instruction("  cmp rax, 0");
        if(node->rhs == NULL)
        {
            // if ( expression ) statement
            put_instruction("  je  .Lend%d", number);
            generate_node(node->lhs);
            put_instruction(".Lend%d:", number);
        }
        else
        {
            // if ( expression ) statement else statement
            put_instruction("  je  .Lelse%d", number);
            generate_node(node->lhs);
            put_instruction("  jmp .Lend%d", number);
            put_instruction(".Lelse%d:", number);
            generate_node(node->rhs);
            put_instruction(".Lend%d:", number);
        }
        return;
    }

    case ND_WHILE:
    {
        int number = label_number;
        label_number++;

        put_instruction(".Lbegin%d:", number);
        generate_node(node->cond);
        put_instruction("  pop rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  je  .Lend%d", number);
        generate_node(node->lhs);
        put_instruction("  jmp .Lbegin%d", number);
        put_instruction(".Lend%d:", number);
        return;
    }

    case ND_DO:
    {
        int number = label_number;
        label_number++;

        put_instruction(".Lbegin%d:", number);
        generate_node(node->lhs);
        generate_node(node->cond);
        put_instruction("  pop rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  jne .Lbegin%d", number);
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
        put_instruction(".Lbegin%d:", number);
        if(node->cond != NULL)
        {
            generate_node(node->cond);
            put_instruction("  pop rax");
            put_instruction("  cmp rax, 0");
            put_instruction("  je  .Lend%d", number);
        }
        generate_node(node->lhs);
        if(node->postexpr != NULL)
        {
            generate_node(node->postexpr);
        }
        put_instruction("  jmp .Lbegin%d", number);
        put_instruction(".Lend%d:", number);
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
        int number = label_number;
        label_number++;

        // System V ABI for x86-64 requires that
        // * rsp be a multiple of 16 before function call.
        // * Arguments be passed in rdi, rsi, rdx, rcx, r8, r9, and further ones be passed on the stack in reverse order.
        // * The number of floating point arguments be set to al if the callee is a variadic function.

        // check alignment of rsp before function call
        put_instruction("  mov rax, rsp");
        put_instruction("  and rax, 0xF");
        put_instruction("  cmp rax, 0");
        put_instruction("  je  .Lbegin%d", number);
        // case 1: rsp has not been aligned yet
        put_instruction("  sub rsp, 8");
        generate_args(node->args);
        put_instruction("  mov rax, 0");
        put_instruction("  call %s", node->ident);
        put_instruction("  add rsp, 8");
        put_instruction("  jmp .Lend%d", number);
        put_instruction(".Lbegin%d:", number);
        // case 2: rsp has already been aligned
        generate_args(node->args);
        put_instruction("  mov rax, 0");
        put_instruction("  call %s", node->ident);
        put_instruction(".Lend%d:", number);
        // push return value
        put_instruction("  push rax");
        return;
    }

    default:
        break;
    }

    // compile LHS and RHS
    generate_node(node->lhs);
    generate_node(node->rhs);

    // pop RHS to rdi and LHS to rax
    put_instruction("  pop rdi");
    put_instruction("  pop rax");

    // execute operation
    switch(node->kind)
    {
    case ND_ADD:
        put_instruction("  add rax, rdi");
        break;

    case ND_PTR_ADD:
        put_instruction("  imul rdi, %ld", node->type->base->size);
        put_instruction("  add rax, rdi");
        break;

    case ND_SUB:
        put_instruction("  sub rax, rdi");
        break;

    case ND_PTR_SUB:
        put_instruction("  imul rdi, %ld", node->type->base->size);
        put_instruction("  sub rax, rdi");
        break;

    case ND_MUL:
        put_instruction("  imul rax, rdi");
        break;

    case ND_DIV:
        put_instruction("  cqo");
        put_instruction("  idiv rdi");
        break;

    case ND_EQ:
        put_instruction("  cmp rax, rdi");
        put_instruction("  sete al");
        put_instruction("  movzb rax, al");
        break;

    case ND_NEQ:
        put_instruction("  cmp rax, rdi");
        put_instruction("  setne al");
        put_instruction("  movzb rax, al");
        break;

    case ND_L:
        put_instruction("  cmp rax, rdi");
        put_instruction("  setl al");
        put_instruction("  movzb rax, al");
        break;

    case ND_LEQ:
        put_instruction("  cmp rax, rdi");
        put_instruction("  setle al");
        put_instruction("  movzb rax, al");
        break;

    default:
        break;
    }

    // push the result of operation
    put_instruction("  push rax");
}


/*
output an instruction
*/
static void put_instruction(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
}