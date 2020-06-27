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
#include <string.h>

#include "declaration.h"
#include "expression.h"
#include "generator.h"
#include "parser.h"
#include "statement.h"
#include "tokenizer.h"
#include "type.h"

#define CHECK_STACK_SIZE (ENABLED) // check stack size
#if(CHECK_STACK_SIZE == ENABLED)
#include <assert.h>
#endif /* CHECK_STACK_SIZE */


// type definition
typedef enum BinaryOperationKind BinaryOperationKind;
enum BinaryOperationKind
{
    BINOP_ADD,      // addition
    BINOP_PTR_ADD,  // pointer addition
    BINOP_SUB,      // subtraction
    BINOP_PTR_SUB,  // pointer subtraction
    BINOP_PTR_DIFF, // pointer difference
    BINOP_MUL,      // multiplication
    BINOP_DIV,      // dividion
    BINOP_MOD,      // remainder
    BINOP_LSHIFT,   // left shift
    BINOP_RSHIFT,   // right shift
    BINOP_EQ,       // equality
    BINOP_NEQ,      // inequality
    BINOP_L,        // strict order
    BINOP_LEQ,      // order
    BINOP_AND,      // bitwise AND
    BINOP_XOR,      // exclusive OR
    BINOP_OR,       // inclusive OR
};

// function prototype
static void generate_push_imm(int value);
static void generate_push_reg_or_mem(const char *reg_or_mem);
static void generate_push_struct(const Type *type);
static void generate_pop(const char *reg_or_mem);
static void generate_load(const Type *type);
static void generate_store(const Type *type);
static void generate_lvalue(const Expression *expr);
static void generate_gvar(const Variable *gvar);
static void generate_func(const Function *func);
static size_t generate_args(const List(Expression) *args, bool pass_address);
static void generate_binary(const Expression *expr, BinaryOperationKind kind);
static void generate_statement(const Statement *stmt);
static void generate_expression(const Expression *expr);
static void put_instruction(const char *fmt, ...);


// global variable
static const char *arg_registers8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"}; // name of 8-bit registers for function arguments
static const char *arg_registers16[] = {"di", "si", "dx", "cx", "r8w", "r9w"}; // name of 16-bit registers for function arguments
static const char *arg_registers32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"}; // name of 32-bit registers for function arguments
static const char *arg_registers64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}; // name of 64-bit registers for function arguments
static const size_t ARG_REGISTERS_SIZE = 6; // number of registers for function arguments
static int lab_number; // sequential number for labels
static int brk_number; // sequential number for break statements
static int cnt_number; // sequential number for continue statements
static int stack_size; // size of the stack
static unsigned int gp_offset; // offset from reg_save_area to the place where the next available general purpose argument register is saved


/*
generate assembler code
*/
void generate(const Program *program)
{
    // use Intel syntax
    put_instruction(".intel_syntax noprefix");

    // generate global variables
    put_instruction(".data");
    for_each_entry(Variable, cursor, program->gvars)
    {
        Variable *gvar = get_element(Variable)(cursor);
        if(gvar->entity)
        {
            generate_gvar(gvar);
        }
    }

    // generate functions
    put_instruction(".text");
    for_each_entry(Function, cursor, program->funcs)
    {
        Function *func = get_element(Function)(cursor);
        generate_func(func);
    }
}


/*
generate assembler code to push an immediate value to the stack
* 'push' instruction cannot take a 64-bit immediate value.
*/
static void generate_push_imm(int value)
{
    put_instruction("  push %d", value);
    stack_size += STACK_ALIGNMENT;
}


/*
generate assembler code to push a register or memory contents to the stack
*/
static void generate_push_reg_or_mem(const char *reg_or_mem)
{
    put_instruction("  push %s", reg_or_mem);
    stack_size += STACK_ALIGNMENT;
}


/*
generate assembler code to push contents of a structure to the stack
*/
static void generate_push_struct(const Type *type)
{
    generate_pop("rax");
    for(size_t offset = adjust_alignment(type->size, STACK_ALIGNMENT); offset > 0; offset -= STACK_ALIGNMENT)
    {
        put_instruction("  push [rax+%lu]", offset - STACK_ALIGNMENT);
        stack_size += STACK_ALIGNMENT;
    }
}


/*
generate assembler code to pop from the stack to a register or memory
*/
static void generate_pop(const char *reg_or_mem)
{
    put_instruction("  pop %s", reg_or_mem);
    stack_size -= STACK_ALIGNMENT;
}


/*
generate assembler code to load value
*/
static void generate_load(const Type *type)
{
    generate_pop("rax");
    if(type->size == 1)
    {
        put_instruction("  movsxb rax, byte ptr [rax]");
    }
    else if(type->size == 2)
    {
        put_instruction("  movsxw rax, word ptr [rax]");
    }
    else if(type->size <= 4)
    {
        put_instruction("  movsxd rax, dword ptr [rax]");
    }
    else if(type->size <= 8)
    {
        put_instruction("  mov rax, [rax]");
    }
    else if(type->size <= 16)
    {
        put_instruction("  mov r11, [rax+8]");
        put_instruction("  mov rax, [rax]");
        generate_push_reg_or_mem("r11");
    }
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code to store value
*/
static void generate_store(const Type *type)
{
    if(type->size <= 8)
    {
        generate_pop("rdi");
        generate_pop("rax");
        if(type->size == 1)
        {
            put_instruction("  mov byte ptr [rax], dil");
        }
        else if(type->size <= 2)
        {
            put_instruction("  mov word ptr [rax], di");
        }
        else if(type->size <= 4)
        {
            put_instruction("  mov dword ptr [rax], edi");
        }
        else
        {
            put_instruction("  mov [rax], rdi");
        }
    }
    else if(type->size <= 16)
    {
        generate_pop("rdi");
        generate_pop("r11");
        generate_pop("rax");
        put_instruction("  mov [rax], rdi");
        put_instruction("  mov [rax+8], r11");
    }
    else
    {
        // pop source address (rdi) and destination address (rax)
        generate_pop("rdi");
        generate_pop("rax");

        // copy bytes from rdi to rax
        size_t offset = 0;
        for(; offset + 8 <= type->size; offset += 8)
        {
            put_instruction("  mov r11, [rdi+%lu]", offset);
            put_instruction("  mov [rax+%lu], r11", offset);
        }
        for(; offset + 4 <= type->size; offset += 4)
        {
            put_instruction("  mov r11, [rdi+%lu]", offset);
            put_instruction("  mov dword ptr [rax+%lu], r11d", offset);
        }
        for(; offset + 2 <= type->size; offset += 2)
        {
            put_instruction("  mov r11, [rdi+%lu]", offset);
            put_instruction("  mov word ptr [rax+%lu], r11w", offset);
        }
        for(; offset + 1 <= type->size; offset += 1)
        {
            put_instruction("  mov r11, [rdi+%lu]", offset);
            put_instruction("  mov byte ptr [rax+%lu], r11b", offset);
        }
    }
    generate_push_reg_or_mem("rdi");
}


/*
generate assembler code for lvalue
*/
static void generate_lvalue(const Expression *expr)
{
    switch(expr->kind)
    {
    case EXPR_VAR:
        if(expr->var->local)
        {
            put_instruction("  mov rax, rbp");
            put_instruction("  sub rax, %lu", expr->var->offset);
            generate_push_reg_or_mem("rax");
        }
        else
        {
            put_instruction("  lea rax, %s[rip]", expr->var->name);
            generate_push_reg_or_mem("rax");
        }
        break;

    case EXPR_DEREF:
        generate_expression(expr->operand);
        break;

    case EXPR_MEMBER:
        generate_lvalue(expr->operand);
        generate_pop("rax");
        put_instruction("  add rax, %lu", expr->member->offset);
        generate_push_reg_or_mem("rax");
        break;

    default:
        report_error(NULL, "expected lvalue");
        break;
    }
}


/*
generate assembler code of a global variable
*/
static void generate_gvar(const Variable *gvar)
{
    // put label
    if(gvar->sclass != SC_STATIC)
    {
        put_instruction(".global %s", gvar->name);
    }
    put_instruction("%s:", gvar->name);

    if(gvar->str != NULL)
    {
        // allocate memory for string-literal
        put_instruction("  .string \"%s\"", gvar->str->content);
    }
    else
    {
        for_each_entry(DataSegment, cursor, gvar->data)
        {
            DataSegment *data = get_element(DataSegment)(cursor);
            if(data->label != NULL)
            {
                // allocate memory with label
                put_instruction("  .quad %s", data->label);
            }
            else if(data->zero)
            {
                // allocate memory with zero
                put_instruction("  .zero %lu", data->size);
            }
            else
            {
                // allocate memory with an integer
                if(data->size == 1)
                {
                    put_instruction("  .byte %ld", data->value);
                }
                else if(data->size == 2)
                {
                    put_instruction("  .value %ld", data->value);
                }
                else if(data->size == 4)
                {
                    put_instruction("  .long %ld", data->value);
                }
                else
                {
                    put_instruction("  .quad %ld", data->value);
                }
            }
        }
    }
}


/*
generate assembler code of a function
*/
static void generate_func(const Function *func)
{
    // declarations
    if(func->sclass != SC_STATIC)
    {
        put_instruction(".global %s", func->name);
    }
    put_instruction("%s:", func->name);

    // prologue: allocate stack for arguments and local variables and copy arguments
    bool pass_address = (get_parameter_class(func->type->base) == PC_MEMORY);
    stack_size = func->stack_size + pass_address * STACK_ALIGNMENT; // initialize stack size
    generate_push_reg_or_mem("rbp");
    put_instruction("  mov rbp, rsp");
    put_instruction("  sub rsp, %lu", stack_size);
    if(pass_address)
    {
        // save the hidden argument
        put_instruction("  mov [rbp-%lu], rdi", stack_size);
    }

    // arguments
    // separate arguments which are passed by registers from those passed by the stack
    size_t argc_reg = (pass_address ? 1 : 0);
    List(Variable) *args_reg = new_list(Variable)();
    List(Variable) *args_stack = new_list(Variable)();
    for_each_entry(Variable, cursor, func->args)
    {
        Variable *arg = get_element(Variable)(cursor);

        // pass up to ARG_REGISTERS_SIZE arguments by registers
        if(argc_reg >= ARG_REGISTERS_SIZE)
        {
            add_list_entry_tail(Variable)(args_stack, arg);
            continue;
        }

        // pass arguments classified as MEMORY by the stack
        ParameterClassKind param_class = get_parameter_class(arg->type);
        if(param_class == PC_MEMORY)
        {
            add_list_entry_tail(Variable)(args_stack, arg);
            continue;
        }

        if(is_struct_or_union(arg->type))
        {
            // check if there is enough registers to pass the whole of structure or union
            size_t argc_struct = adjust_alignment(arg->type->size, STACK_ALIGNMENT) / STACK_ALIGNMENT;
            if(argc_reg + argc_struct < ARG_REGISTERS_SIZE)
            {
                add_list_entry_tail(Variable)(args_reg, arg);
                argc_reg += argc_struct;
            }
            else
            {
                add_list_entry_tail(Variable)(args_stack, arg);
            }
            continue;
        }

        add_list_entry_tail(Variable)(args_reg, arg);
        argc_reg++;
    }

    argc_reg = (pass_address ? 1 : 0);
    for_each_entry(Variable, cursor, args_reg)
    {
        // load argument from a register
        Variable *arg = get_element(Variable)(cursor);

        put_instruction("  mov rax, rbp");
        put_instruction("  sub rax, %lu", arg->offset);
        if(is_struct_or_union(arg->type))
        {
            size_t argc_struct = adjust_alignment(arg->type->size, STACK_ALIGNMENT) / STACK_ALIGNMENT;
            for(size_t i = 0; i < argc_struct; i++)
            {
                put_instruction("  mov [rax+%lu], %s", i * STACK_ALIGNMENT, arg_registers64[argc_reg + i]);
            }
            argc_reg += argc_struct;
        }
        else
        {
            if(arg->type->size == 1)
            {
                put_instruction("  mov byte ptr [rax], %s", arg_registers8[argc_reg]);
            }
            else if(arg->type->size == 2)
            {
                put_instruction("  mov word ptr [rax], %s", arg_registers16[argc_reg]);
            }
            else if(arg->type->size == 4)
            {
                put_instruction("  mov dword ptr [rax], %s", arg_registers32[argc_reg]);
            }
            else
            {
                put_instruction("  mov [rax], %s", arg_registers64[argc_reg]);
            }
            argc_reg++;
        }
    }

    size_t stack_args = 0;
    for_each_entry(Variable, cursor, args_stack)
    {
        // load argument from the stack
        Variable *arg = get_element(Variable)(cursor);

        put_instruction("  mov rax, rbp");
        put_instruction("  sub rax, %lu", arg->offset);
        generate_push_reg_or_mem("rax");

        put_instruction("  mov rax, rbp");
        put_instruction("  add rax, %lu", stack_args + 2 * STACK_ALIGNMENT);
        generate_push_reg_or_mem("rax");

        generate_load(arg->type);
        generate_store(arg->type);

        stack_args += adjust_alignment(arg->type->size, STACK_ALIGNMENT);
    }

    if(func->type->variadic)
    {
        gp_offset = argc_reg * STACK_ALIGNMENT;
        for(size_t i = 0; i < ARG_REGISTERS_SIZE; i++)
        {
            put_instruction("  mov [rbp-%lu], %s", (ARG_REGISTERS_SIZE - i) * STACK_ALIGNMENT, arg_registers64[i]);
        }
    }

    // body
    for_each_entry(Statement, cursor, func->body)
    {
        Statement *stmt = get_element(Statement)(cursor);
        generate_statement(stmt);
    }

    // epilogue: release stack
    if(pass_address)
    {
        // return the address passed by the hidden argument
        put_instruction("  mov rax, [rbp-%lu]", func->stack_size + STACK_ALIGNMENT);
    }
    put_instruction("  mov rsp, rbp");
    generate_pop("rbp");
    put_instruction("  ret");
}


/*
generate assembler code of function arguments
* This function returns size of the stack allocated for arguments.
*/
static size_t generate_args(const List(Expression) *args, bool pass_address)
{
    // separate arguments which are passed by registers from those passed by the stack
    size_t argc_reg = (pass_address ? 1 : 0);
    List(Expression) *args_reg = new_list(Expression)();
    List(Expression) *args_stack = new_list(Expression)();
    for_each_entry(Expression, cursor, args)
    {
        Expression *arg = get_element(Expression)(cursor);

        // pass up to ARG_REGISTERS_SIZE arguments by registers
        if(argc_reg >= ARG_REGISTERS_SIZE)
        {
            add_list_entry_tail(Expression)(args_stack, arg);
            continue;
        }

        // pass arguments classified as MEMORY by the stack
        ParameterClassKind param_class = get_parameter_class(arg->type);
        if(param_class == PC_MEMORY)
        {
            add_list_entry_tail(Expression)(args_stack, arg);
            continue;
        }

        if(is_struct_or_union(arg->type))
        {
            // check if there is enough registers to pass the whole of structure or union
            size_t argc_struct = adjust_alignment(arg->type->size, STACK_ALIGNMENT) / STACK_ALIGNMENT;
            if(argc_reg + argc_struct < ARG_REGISTERS_SIZE)
            {
                add_list_entry_tail(Expression)(args_reg, arg);
                argc_reg += argc_struct;
            }
            else
            {
                add_list_entry_tail(Expression)(args_stack, arg);
            }
            continue;
        }

        add_list_entry_tail(Expression)(args_reg, arg);
        argc_reg++;
    }

    // evaluate and pass arguments in reverse order
    // because rdi (register for the 1st argument) may be modified when generating assembler code of arguments
    for_each_entry_reversed(Expression, cursor, args_reg)
    {
        Expression *arg = get_element(Expression)(cursor);

        generate_expression(arg);
        if(is_struct_or_union(arg->type))
        {
            size_t argc_struct = adjust_alignment(arg->type->size, STACK_ALIGNMENT) / STACK_ALIGNMENT;
            argc_reg -= argc_struct;

            for(size_t i = 0; i < argc_struct; i++)
            {
                generate_pop(arg_registers64[argc_reg + i]);
            }
        }
        else
        {
            argc_reg--;
            generate_pop(arg_registers64[argc_reg]);
        }
    }

    // push arguments to the stack in reverse order
    size_t stack_args = 0;
    for_each_entry_reversed(Expression, cursor, args_stack)
    {
        Expression *arg = get_element(Expression)(cursor);

        generate_expression(arg);
        if(get_parameter_class(arg->type) == PC_MEMORY)
        {
            generate_push_struct(arg->type);
        }
        stack_args += adjust_alignment(arg->type->size, STACK_ALIGNMENT);
    }

    return stack_args;
}


/*
generate assembler code of binary operation
*/
static void generate_binary(const Expression *expr, BinaryOperationKind kind)
{
    // pop RHS to rdi and LHS to rax
    generate_pop("rdi");
    generate_pop("rax");

    // execute operation
    switch(kind)
    {
    case BINOP_ADD:
        put_instruction("  add rax, rdi");
        break;

    case BINOP_PTR_ADD:
        put_instruction("  imul rdi, %lu", expr->type->base->size);
        put_instruction("  add rax, rdi");
        break;

    case BINOP_SUB:
        put_instruction("  sub rax, rdi");
        break;

    case BINOP_PTR_SUB:
        put_instruction("  imul rdi, %lu", expr->type->base->size);
        put_instruction("  sub rax, rdi");
        break;

    case BINOP_PTR_DIFF:
        put_instruction("  sub rax, rdi");
        put_instruction("  cqo");
        put_instruction("  mov rdi, %lu", expr->lhs->type->base->size);
        put_instruction("  idiv rdi");
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

    case BINOP_AND:
        put_instruction("  and rax, rdi");
        break;

    case BINOP_XOR:
        put_instruction("  xor rax, rdi");
        break;

    case BINOP_OR:
        put_instruction("  or rax, rdi");
        break;

    default:
        break;
    }

    // push the result of operation
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of a statement
*/
static void generate_statement(const Statement *stmt)
{
    // Because this function is recursively called, 
    // * `lab_number` is saved and incremented at the top of each case-statement.
    // * `brk_number` and `cnt_number` are saved at the top of each case-statement and restored at the end of each case-statement.
    switch(stmt->kind)
    {
    case STMT_LABEL:
        put_instruction(".L%s:", stmt->ident);
        generate_statement(stmt->body);
        return;

    case STMT_CASE:
        put_instruction(".Lcase%d:", stmt->case_label);
        generate_statement(stmt->body);
        return;

    case STMT_COMPOUND:
        for_each_entry(Statement, cursor, stmt->compound)
        {
            Statement *s = get_element(Statement)(cursor);
            generate_statement(s);
        }
        return;

    case STMT_DECL:
        for_each_entry(Declaration, cursor, stmt->decl)
        {
            Declaration *decl = get_element(Declaration)(cursor);
            if((decl->var != NULL) && (decl->var->init != NULL))
            {
                generate_statement(decl->var->init);
            }
        }
        return;

    case STMT_EXPR:
        generate_expression(stmt->expr);
        generate_pop("rax");
        return;

    case STMT_NULL:
        return;

    case STMT_IF:
    {
        int lab = lab_number;
        lab_number++;

        generate_expression(stmt->cond);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        if(stmt->false_case == NULL)
        {
            // if ( expression ) statement
            put_instruction("  je  .Lend%d", lab);
            generate_statement(stmt->true_case);
            put_instruction(".Lend%d:", lab);
        }
        else
        {
            // if ( expression ) statement else statement
            put_instruction("  je  .Lelse%d", lab);
            generate_statement(stmt->true_case);
            put_instruction("  jmp .Lend%d", lab);
            put_instruction(".Lelse%d:", lab);
            generate_statement(stmt->false_case);
            put_instruction(".Lend%d:", lab);
        }
        return;
    }

    case STMT_SWITCH:
    {
        int lab = lab_number;
        int brk = brk_number;
        brk_number = lab_number;
        lab_number++;

        generate_expression(stmt->cond);
        generate_pop("rax");

        for_each_entry(Statement, cursor, stmt->case_list)
        {
            Statement *s = get_element(Statement)(cursor);
            int case_label = lab_number;
            lab_number++;

            s->case_label = case_label;
            put_instruction("  cmp rax, %d", s->value);
            put_instruction("  je .Lcase%d", case_label);
        }
        if(stmt->default_case != NULL)
        {
            int case_label = lab_number;
            lab_number++;

            stmt->default_case->case_label = case_label;
            put_instruction("  jmp .Lcase%d", case_label);
        }

        generate_statement(stmt->body);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        return;
    }

    case STMT_WHILE:
    {
        int lab = lab_number;
        int brk = brk_number;
        int cnt = cnt_number;
        brk_number = cnt_number = lab_number;
        lab_number++;

        put_instruction(".Lbegin%d:", lab);
        put_instruction(".Lcontinue%d:", lab);
        generate_expression(stmt->cond);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  je  .Lend%d", lab);
        generate_statement(stmt->body);
        put_instruction("  jmp .Lbegin%d", lab);
        put_instruction(".Lend%d:", lab);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        cnt_number = cnt;
        return;
    }

    case STMT_DO:
    {
        int lab = lab_number;
        int brk = brk_number;
        int cnt = cnt_number;
        brk_number = cnt_number = lab_number;
        lab_number++;

        put_instruction(".Lbegin%d:", lab);
        generate_statement(stmt->body);
        put_instruction(".Lcontinue%d:", lab);
        generate_expression(stmt->cond);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  jne .Lbegin%d", lab);
        put_instruction(".Lend%d:", lab);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        cnt_number = cnt;
        return;
    }

    case STMT_FOR:
    {
        int lab = lab_number;
        int brk = brk_number;
        int cnt = cnt_number;
        brk_number = cnt_number = lab_number;
        lab_number++;

        if(stmt->preexpr != NULL)
        {
            generate_expression(stmt->preexpr);
        }
        if(stmt->predecl != NULL)
        {
            generate_statement(stmt->predecl);
        }
        put_instruction(".Lbegin%d:", lab);
        if(stmt->cond != NULL)
        {
            generate_expression(stmt->cond);
            generate_pop("rax");
            put_instruction("  cmp rax, 0");
            put_instruction("  je  .Lend%d", lab);
        }
        generate_statement(stmt->body);
        put_instruction(".Lcontinue%d:", lab);
        if(stmt->postexpr != NULL)
        {
            generate_expression(stmt->postexpr);
        }
        put_instruction("  jmp .Lbegin%d", lab);
        put_instruction(".Lend%d:", lab);
        put_instruction(".Lbreak%d:", lab);

        brk_number = brk;
        cnt_number = cnt;
        return;
    }

    case STMT_GOTO:
        put_instruction("  jmp .L%s", stmt->ident);
        return;

    case STMT_CONTINUE:
        put_instruction("  jmp .Lcontinue%d", cnt_number);
        return;

    case STMT_BREAK:
        put_instruction("  jmp .Lbreak%d", brk_number);
        return;

    case STMT_RETURN:
        if(stmt->expr != NULL)
        {
            // return value
            generate_expression(stmt->expr);
            if(is_struct_or_union(stmt->expr->type) && (adjust_alignment(stmt->expr->type->size, STACK_ALIGNMENT) == 16))
            {
                generate_pop("rax");
                generate_pop("rdx");
            }
            else
            {
                generate_pop("rax");
            }
        }
        put_instruction("  mov rsp, rbp");
        generate_pop("rbp");
        put_instruction("  ret");
        return;

    default:
        break;
    }
}

/*
generate assembler code of an expression, which emulates stack machine
*/
static void generate_expression(const Expression *expr)
{
    // Because this function is recursively called, 
    // * `lab_number` is saved and incremented at the top of each case-statement.
    // * `brk_number` and `cnt_number` are saved at the top of each case-statement and restored at the end of each case-statement.
    switch(expr->kind)
    {
    case EXPR_CONST:
        // It is assumed that the size of 'int' is 4 bytes.
        if(expr->value == (int)expr->value)
        {
            generate_push_imm(expr->value);
        }
        else
        {
            put_instruction("  mov rax, %ld", expr->value);
            generate_push_reg_or_mem("rax");
        }
        return;

    case EXPR_VAR:
        generate_lvalue(expr);
        if(!(is_array(expr->var->type) || is_function(expr->var->type)))
        {
            generate_load(expr->var->type);
        }
        return;

    case EXPR_ADDR:
        generate_lvalue(expr->operand);
        return;

    case EXPR_DEREF:
        generate_expression(expr->operand);
        if(!(is_array(expr->type) || is_function(expr->type)))
        {
            generate_load(expr->type);
        }
        return;

    case EXPR_MEMBER:
        generate_lvalue(expr);
        if(!is_array(expr->member->type))
        {
            generate_load(expr->member->type);
        }
        return;

    case EXPR_PLUS:
        generate_expression(expr->operand);
        return;

    case EXPR_MINUS:
        generate_expression(expr->operand);
        generate_pop("rax");
        put_instruction("  neg rax");
        generate_push_reg_or_mem("rax");
        return;

    case EXPR_COMPL:
        generate_expression(expr->operand);
        generate_pop("rax");
        put_instruction("  not rax");
        generate_push_reg_or_mem("rax");
        return;

    case EXPR_NEG:
        generate_expression(expr->operand);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  sete al");
        put_instruction("  movzb rax, al");
        generate_push_reg_or_mem("rax");
        return;

    case EXPR_POST_INC:
        generate_lvalue(expr->operand);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->operand->type);
        generate_pop("rax");
        put_instruction("  mov rdx, rax");
        generate_push_reg_or_mem("rax");
        generate_push_imm(1);
        generate_binary(expr, (is_integer(expr->type) ? BINOP_ADD : BINOP_PTR_ADD));
        generate_store(expr->type);
        generate_pop("rax");
        generate_push_reg_or_mem("rdx");
        return;

    case EXPR_POST_DEC:
        generate_lvalue(expr->operand);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->operand->type);
        generate_pop("rax");
        put_instruction("  mov rdx, rax");
        generate_push_reg_or_mem("rax");
        generate_push_imm(1);
        generate_binary(expr, (is_integer(expr->type) ? BINOP_SUB : BINOP_PTR_SUB));
        generate_store(expr->type);
        generate_pop("rax");
        generate_push_reg_or_mem("rdx");
        return;

    case EXPR_CAST:
        generate_expression(expr->operand);
        generate_pop("rax");
        if(expr->type->size == 1)
        {
            put_instruction("  movsxb rax, al");
        }
        else if(expr->type->size == 2)
        {
            put_instruction("  movsxw rax, ax");
        }
        else if(expr->type->size == 4)
        {
            put_instruction("  movsxd rax, eax");
        }
        generate_push_reg_or_mem("rax");
        return;

    case EXPR_LOG_AND:
    {
        // note that RHS is not evaluated if LHS is equal to 0
        int lab = lab_number;
        lab_number++;

        generate_expression(expr->lhs);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  je .Lfalse%d", lab);
        generate_expression(expr->rhs);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  je .Lfalse%d", lab);
        put_instruction("  mov rax, 1");
        put_instruction("  jmp .Lend%d", lab);
        put_instruction(".Lfalse%d:", lab);
        put_instruction("  mov rax, 0");
        put_instruction(".Lend%d:", lab);
        generate_push_reg_or_mem("rax");
        return;
    }

    case EXPR_LOG_OR:
    {
        // note that RHS is not evaluated if LHS is not equal to 0
        int lab = lab_number;
        lab_number++;

        generate_expression(expr->lhs);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  jne .Ltrue%d", lab);
        generate_expression(expr->rhs);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  jne .Ltrue%d", lab);
        put_instruction("  mov rax, 0");
        put_instruction("  jmp .Lend%d", lab);
        put_instruction(".Ltrue%d:", lab);
        put_instruction("  mov rax, 1");
        put_instruction(".Lend%d:", lab);
        generate_push_reg_or_mem("rax");
        return;
    }

    case EXPR_COND:
    {
        // note that either second operand or third operand is evaluated
        int lab = lab_number;
        lab_number++;

        generate_expression(expr->operand);
        generate_pop("rax");
        put_instruction("  cmp rax, 0");
        put_instruction("  je .Lelse%d", lab);
        generate_expression(expr->lhs);
        generate_pop("rax");
        put_instruction("  jmp .Lend%d", lab);
        put_instruction(".Lelse%d:", lab);
        generate_expression(expr->rhs);
        generate_pop("rax");
        put_instruction(".Lend%d:", lab);
        generate_push_reg_or_mem("rax");
        return;
    }

    case EXPR_ASSIGN:
        generate_lvalue(expr->lhs);
        generate_expression(expr->rhs);
        generate_store(expr->type);
        return;

    case EXPR_ADD_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_ADD);
        generate_store(expr->type);
        return;

    case EXPR_PTR_ADD_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_PTR_ADD);
        generate_store(expr->type);
        return;

    case EXPR_SUB_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_SUB);
        generate_store(expr->type);
        return;

    case EXPR_PTR_SUB_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_PTR_SUB);
        generate_store(expr->type);
        return;

    case EXPR_MUL_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_MUL);
        generate_store(expr->type);
        return;

    case EXPR_DIV_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_DIV);
        generate_store(expr->type);
        return;

    case EXPR_MOD_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_MOD);
        generate_store(expr->type);
        return;

    case EXPR_LSHIFT_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_LSHIFT);
        generate_store(expr->type);
        return;

    case EXPR_RSHIFT_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_RSHIFT);
        generate_store(expr->type);
        return;

    case EXPR_AND_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_AND);
        generate_store(expr->type);
        return;

    case EXPR_XOR_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_XOR);
        generate_store(expr->type);
        return;

    case EXPR_OR_EQ:
        generate_lvalue(expr->lhs);
        generate_push_reg_or_mem("[rsp]");
        generate_load(expr->lhs->type);
        generate_expression(expr->rhs);
        generate_binary(expr, BINOP_OR);
        generate_store(expr->type);
        return;

    case EXPR_COMMA:
        generate_expression(expr->lhs);
        generate_pop("rax");
        generate_expression(expr->rhs);
        return;

    case EXPR_FUNC:
    {
        // System V ABI for x86-64 requires that
        // * rsp be a multiple of 16 before function call.
        // * Arguments be passed in rdi, rsi, rdx, rcx, r8, r9, and further ones be passed on the stack in reverse order.
        // * The number of floating point arguments be set to al if the callee is a variadic function.

        Expression *body = expr->operand->operand;
        if((body != NULL) && (body->var != NULL) && (strcmp(body->var->name, "__builtin_va_start") == 0))
        {
            generate_expression(get_first_element(Expression)(expr->args));
            put_instruction("  pop rax");
            put_instruction("  mov dword ptr [rax], %d", gp_offset); // gp_offset
            put_instruction("  mov dword ptr [rax+4], %lu", ARG_REGISTERS_SIZE); // fp_offset
            put_instruction("  lea r11, [rbp+%lu]", 2 * STACK_ALIGNMENT);
            put_instruction("  mov qword ptr [rax+8], r11"); // overflow_arg_area
            put_instruction("  lea r11, [rbp-%lu]", REGISTER_SAVE_AREA_SIZE);
            put_instruction("  mov qword ptr [rax+16], r11"); // reg_save_area
            return;
        }

#if(CHECK_STACK_SIZE == ENABLED)
        int current_stack = stack_size;
#endif /* CHECK_STACK_SIZE */
        int stack_adjustment = 0;
        size_t space = adjust_alignment(expr->type->size, STACK_ALIGNMENT) + STACK_ALIGNMENT;
        bool pass_address = (get_parameter_class(expr->type) == PC_MEMORY);
        if(pass_address)
        {
            // provide space for the return value and pass the address of the space as the hidden 1st argument
            stack_size += space;
            stack_adjustment += space;
            put_instruction("  lea rdi, [rsp-%lu]", space);
        }

        // adjust alignment of rsp before generating arguments since the callee expects that arguments on the stack be just above the return address
        bool aligned = ((stack_size % 16) == 0);
        if(!aligned)
        {
            stack_size += STACK_ALIGNMENT;
            stack_adjustment += STACK_ALIGNMENT;
        }
        put_instruction("  sub rsp, %d", stack_adjustment);

        // generate arguments
        stack_adjustment += generate_args(expr->args, pass_address);
        put_instruction("  mov rax, 0");

        if((body != NULL) && (body->var != NULL))
        {
            // call function by name
            put_instruction("  call %s", body->var->name);
        }
        else
        {
            // call function through pointer
            generate_expression(expr->operand);
            generate_pop("rax");
            put_instruction("  call rax");
        }

        // restore the stack
        put_instruction("  add rsp, %d", stack_adjustment);
        stack_size -= stack_adjustment;
#if(CHECK_STACK_SIZE == ENABLED)
        assert(stack_size == current_stack);
#endif /* CHECK_STACK_SIZE */

        // push return value
        if(expr->type->size == 16)
        {
            generate_push_reg_or_mem("rdx");
        }
        if(is_bool(expr->type))
        {
            put_instruction("  movzb rax, al");
        }
        generate_push_reg_or_mem("rax");

        return;
    }

    default:
        // in case of binary operation
        break;
    }

    // compile LHS and RHS
    generate_expression(expr->lhs);
    generate_expression(expr->rhs);

    // execute operation
    switch(expr->kind)
    {
    case EXPR_ADD:
        generate_binary(expr, BINOP_ADD);
        return;

    case EXPR_PTR_ADD:
        generate_binary(expr, BINOP_PTR_ADD);
        return;

    case EXPR_SUB:
        generate_binary(expr, BINOP_SUB);
        return;

    case EXPR_PTR_SUB:
        generate_binary(expr, BINOP_PTR_SUB);
        return;

    case EXPR_PTR_DIFF:
        generate_binary(expr, BINOP_PTR_DIFF);
        return;

    case EXPR_MUL:
        generate_binary(expr, BINOP_MUL);
        return;

    case EXPR_DIV:
        generate_binary(expr, BINOP_DIV);
        return;

    case EXPR_MOD:
        generate_binary(expr, BINOP_MOD);
        return;

    case EXPR_LSHIFT:
        generate_binary(expr, BINOP_LSHIFT);
        return;

    case EXPR_RSHIFT:
        generate_binary(expr, BINOP_RSHIFT);
        return;

    case EXPR_EQ:
        generate_binary(expr, BINOP_EQ);
        return;

    case EXPR_NEQ:
        generate_binary(expr, BINOP_NEQ);
        return;

    case EXPR_L:
        generate_binary(expr, BINOP_L);
        return;

    case EXPR_LEQ:
        generate_binary(expr, BINOP_LEQ);
        return;

    case EXPR_BIT_AND:
        generate_binary(expr, BINOP_AND);
        return;

    case EXPR_BIT_XOR:
        generate_binary(expr, BINOP_XOR);
        return;

    case EXPR_BIT_OR:
        generate_binary(expr, BINOP_OR);
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
