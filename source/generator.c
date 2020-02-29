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


// type definition
typedef enum {
    BINOP_ADD,     // addition
    BINOP_PTR_ADD, // pointer addition
    BINOP_SUB,     // subtraction
    BINOP_PTR_SUB, // pointer subtraction
    BINOP_MUL,     // multiplication
    BINOP_DIV,     // dividion
    BINOP_MOD,     // remainder
    BINOP_LSHIFT,  // left shift
    BINOP_RSHIFT,  // right shift
    BINOP_EQ,      // equality
    BINOP_NEQ,     // inequality
    BINOP_L,       // strict order
    BINOP_LEQ,     // order
} BinaryOperationKind;


// function prototype
static void generate_load(const Type *type);
static void generate_store(const Type *type);
static void generate_lvalue(const Node *node);
static void generate_gvar(const GVar *gvar);
static void generate_func(const Function *func);
static void generate_args(const Node *args);
static void generate_binary(const Node *node, BinaryOperationKind kind);
static void generate_node(const Node *node);
static void put_instruction(const char *fmt, ...);


// global variable
const char *arg_registers8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"}; // name of 8-bit registers for function arguments
const char *arg_registers16[] = {"di", "si", "dx", "cx", "r8w", "r9w"}; // name of 16-bit registers for function arguments
const char *arg_registers32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"}; // name of 32-bit registers for function arguments
const char *arg_registers64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}; // name of 64-bit registers for function arguments
const size_t ARG_REGISTERS_SIZE = 6; // number of registers for function arguments
static int lab_number; // sequential number for labels
static int brk_number; // sequential number for break statements
static int cnt_number; // sequential number for continue statements


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
    if(func->type->kind != TY_VOID)
    {
        put_instruction("  pop rax");
    }
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
generate assembler code of binary operation
*/
static void generate_binary(const Node *node, BinaryOperationKind kind)
{
    // pop RHS to rdi and LHS to rax
    put_instruction("  pop rdi");
    put_instruction("  pop rax");

    // execute operation
    switch(kind)
    {
    case BINOP_ADD:
        put_instruction("  add rax, rdi");
        break;

    case BINOP_PTR_ADD:
        put_instruction("  imul rdi, %ld", node->type->base->size);
        put_instruction("  add rax, rdi");
        break;

    case BINOP_SUB:
        put_instruction("  sub rax, rdi");
        break;

    case BINOP_PTR_SUB:
        put_instruction("  imul rdi, %ld", node->type->base->size);
        put_instruction("  sub rax, rdi");
        break;

    case BINOP_MUL:
        put_instruction("  imul rax, rdi");
        break;

    case BINOP_DIV:
        put_instruction("  cqo");
        put_instruction("  idiv rdi");
        break;

    case BINOP_MOD:
        put_instruction("  cqo");
        put_instruction("  idiv rdi");
        put_instruction("  mov rax, rdx");
        break;

    case BINOP_LSHIFT:
        put_instruction("  mov rcx, rdi");
        put_instruction("  shl rax, cl");
        break;

    case BINOP_RSHIFT:
        put_instruction("  mov rcx, rdi");
        put_instruction("  sar rax, cl");
        break;

    case BINOP_EQ:
        put_instruction("  cmp rax, rdi");
        put_instruction("  sete al");
        put_instruction("  movzb rax, al");
        break;

    case BINOP_NEQ:
        put_instruction("  cmp rax, rdi");
        put_instruction("  setne al");
        put_instruction("  movzb rax, al");
        break;

    case BINOP_L:
        put_instruction("  cmp rax, rdi");
        put_instruction("  setl al");
        put_instruction("  movzb rax, al");
        break;

    case BINOP_LEQ:
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
generate assembler code of a node, which emulates stack machine
*/
static void generate_node(const Node *node)
{
    // Because this function is recursively called, 
    // * `lab_number` is saved and incremented at the top of each case-statement.
    // * `brk_number` and `cnt_number` are saved at the top of each case-statement and restored at the end of each case-statement.
    switch(node->kind)
    {
    case ND_NULL:
        return;

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

    case ND_POST_INC:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        put_instruction("  pop rax");
        put_instruction("  mov rbx, rax");
        put_instruction("  push rax");
        put_instruction("  push 1");
        generate_binary(node, (is_integer(node->type) ? BINOP_ADD : BINOP_PTR_ADD));
        generate_store(node->type);
        put_instruction("  pop rax");
        put_instruction("  push rbx");
        return;

    case ND_POST_DEC:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        put_instruction("  pop rax");
        put_instruction("  mov rbx, rax");
        put_instruction("  push rax");
        put_instruction("  push 1");
        generate_binary(node, (is_integer(node->type) ? BINOP_SUB : BINOP_PTR_SUB));
        generate_store(node->type);
        put_instruction("  pop rax");
        put_instruction("  push rbx");
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
        generate_binary(node, BINOP_ADD);
        generate_store(node->type);
        return;

    case ND_PTR_ADD_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        generate_binary(node, BINOP_PTR_ADD);
        generate_store(node->type);
        return;

    case ND_SUB_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        generate_binary(node, BINOP_SUB);
        generate_store(node->type);
        return;

    case ND_PTR_SUB_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        generate_binary(node, BINOP_PTR_SUB);
        generate_store(node->type);
        return;

    case ND_MUL_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        generate_binary(node, BINOP_MUL);
        generate_store(node->type);
        return;

    case ND_DIV_EQ:
        generate_lvalue(node->lhs);
        put_instruction("  push [rsp]");
        generate_load(node->lhs->type);
        generate_node(node->rhs);
        generate_binary(node, BINOP_DIV);
        generate_store(node->type);
        return;

    case ND_RETURN:
        if(node->lhs != NULL)
        {
            // return value
            generate_node(node->lhs);
            put_instruction("  pop rax");
        }
        put_instruction("  mov rsp, rbp");
        put_instruction("  pop rbp");
        put_instruction("  ret");
        return;

    case ND_IF:
    {
        int lab = lab_number;
        lab_number++;

        generate_node(node->cond);
        put_instruction("  pop rax");
        put_instruction("  cmp rax, 0");
        if(node->rhs == NULL)
        {
            // if ( expression ) statement
            put_instruction("  je  .Lend%d", lab);
            generate_node(node->lhs);
            put_instruction(".Lend%d:", lab);
        }
        else
        {
            // if ( expression ) statement else statement
            put_instruction("  je  .Lelse%d", lab);
            generate_node(node->lhs);
            put_instruction("  jmp .Lend%d", lab);
            put_instruction(".Lelse%d:", lab);
            generate_node(node->rhs);
            put_instruction(".Lend%d:", lab);
        }
        return;
    }

    case ND_SWITCH:
    {
        int lab = lab_number;
        int brk = brk_number;
        brk_number = lab_number;
        lab_number++;

        generate_node(node->cond);
        put_instruction("  pop rax");

        for(Node *case_node = node->next_case; case_node != NULL; case_node = case_node->next_case)
        {
            int case_label = lab_number;
            lab_number++;

            case_node->case_label = case_label;
            put_instruction("  cmp rax, %d", case_node->val);
            put_instruction("  je .Lcase%d", case_label);
        }
        if(node->default_case != NULL)
        {
            int case_label = lab_number;
            lab_number++;

            node->default_case->case_label = case_label;
            put_instruction("  jmp .Lcase%d", case_label);
        }

        generate_node(node->lhs);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        return;
    }

    case ND_CASE:
        put_instruction(".Lcase%d:", node->case_label);
        generate_node(node->lhs);
        return;

    case ND_LABEL:
        put_instruction(".L%s:", node->ident);
        generate_node(node->lhs);
        return;

    case ND_WHILE:
    {
        int lab = lab_number;
        int brk = brk_number;
        int cnt = cnt_number;
        brk_number = cnt_number = lab_number;
        lab_number++;

        put_instruction(".Lbegin%d:", lab);
        put_instruction(".Lcontinue%d:", lab);
        generate_node(node->cond);
        put_instruction("  pop rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  je  .Lend%d", lab);
        generate_node(node->lhs);
        put_instruction("  jmp .Lbegin%d", lab);
        put_instruction(".Lend%d:", lab);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        cnt_number = cnt;
        return;
    }

    case ND_DO:
    {
        int lab = lab_number;
        int brk = brk_number;
        int cnt = cnt_number;
        brk_number = cnt_number = lab_number;
        lab_number++;

        put_instruction(".Lbegin%d:", lab);
        generate_node(node->lhs);
        put_instruction(".Lcontinue%d:", lab);
        generate_node(node->cond);
        put_instruction("  pop rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  jne .Lbegin%d", lab);
        put_instruction(".Lend%d:", lab);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        cnt_number = cnt;
        return;
    }

    case ND_FOR:
    {
        int lab = lab_number;
        int brk = brk_number;
        int cnt = cnt_number;
        brk_number = cnt_number = lab_number;
        lab_number++;

        if(node->preexpr != NULL)
        {
            generate_node(node->preexpr);
        }
        put_instruction(".Lbegin%d:", lab);
        if(node->cond != NULL)
        {
            generate_node(node->cond);
            put_instruction("  pop rax");
            put_instruction("  cmp rax, 0");
            put_instruction("  je  .Lend%d", lab);
        }
        generate_node(node->lhs);
        put_instruction(".Lcontinue%d:", lab);
        if(node->postexpr != NULL)
        {
            generate_node(node->postexpr);
        }
        put_instruction("  jmp .Lbegin%d", lab);
        put_instruction(".Lend%d:", lab);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        cnt_number = cnt;
        return;
    }

    case ND_GOTO:
        put_instruction("  jmp .L%s", node->ident);
        return;

    case ND_BREAK:
        put_instruction("  jmp .Lbreak%d", brk_number);
        return;

    case ND_CONTINUE:
        put_instruction("  jmp .Lcontinue%d", cnt_number);
        return;

    case ND_BLOCK:
        for(Node *n = node->body; n != NULL; n = n->next)
        {
            generate_node(n);
        }
        return;

    case ND_FUNC:
    {
        int lab = lab_number;
        lab_number++;

        // System V ABI for x86-64 requires that
        // * rsp be a multiple of 16 before function call.
        // * Arguments be passed in rdi, rsi, rdx, rcx, r8, r9, and further ones be passed on the stack in reverse order.
        // * The number of floating point arguments be set to al if the callee is a variadic function.

        // check alignment of rsp before function call
        put_instruction("  mov rax, rsp");
        put_instruction("  and rax, 0xF");
        put_instruction("  cmp rax, 0");
        put_instruction("  je  .Lbegin%d", lab);
        // case 1: rsp has not been aligned yet
        put_instruction("  sub rsp, 8");
        generate_args(node->args);
        put_instruction("  mov rax, 0");
        put_instruction("  call %s", node->ident);
        put_instruction("  add rsp, 8");
        put_instruction("  jmp .Lend%d", lab);
        put_instruction(".Lbegin%d:", lab);
        // case 2: rsp has already been aligned
        generate_args(node->args);
        put_instruction("  mov rax, 0");
        put_instruction("  call %s", node->ident);
        put_instruction(".Lend%d:", lab);
        // push return value
        put_instruction("  push rax");
        return;
    }

    default:
        // in case of binary operation
        break;
    }

    // compile LHS and RHS
    generate_node(node->lhs);
    generate_node(node->rhs);

    // execute operation
    switch(node->kind)
    {
    case ND_ADD:
        generate_binary(node, BINOP_ADD);
        return;

    case ND_PTR_ADD:
        generate_binary(node, BINOP_PTR_ADD);
        return;

    case ND_SUB:
        generate_binary(node, BINOP_SUB);
        return;

    case ND_PTR_SUB:
        generate_binary(node, BINOP_PTR_SUB);
        return;

    case ND_MUL:
        generate_binary(node, BINOP_MUL);
        return;

    case ND_DIV:
        generate_binary(node, BINOP_DIV);
        return;

    case ND_MOD:
        generate_binary(node, BINOP_MOD);
        return;

    case ND_LSHIFT:
        generate_binary(node, BINOP_LSHIFT);
        return;

    case ND_RSHIFT:
        generate_binary(node, BINOP_RSHIFT);
        return;

    case ND_EQ:
        generate_binary(node, BINOP_EQ);
        return;

    case ND_NEQ:
        generate_binary(node, BINOP_NEQ);
        return;

    case ND_L:
        generate_binary(node, BINOP_L);
        return;

    case ND_LEQ:
        generate_binary(node, BINOP_LEQ);
        return;

    default:
        break;
    }
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