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
static void generate_push_xmm(int number);
static void generate_push_struct(const Type *type);
static void generate_push_offset(const char *reg, size_t offset);
static void generate_pop(const char *reg_or_mem);
static void generate_pop_xmm(int number);
static void generate_load(const Type *type);
static void generate_store(const Type *type);
static void generate_lvalue(const Expression *expr);
static void generate_lvar_init(const Variable *lvar);
static void generate_gvar(const Variable *gvar);
static void generate_gvar_data(const Type *type, const Expression *expr);
static void generate_gvar_inits(const List(InitializerMap) *inits);
static void generate_func(const Function *func);
static void generate_args(bool pass_address, const List(Expression) *args_reg, const List(Expression) *args_xmm, const List(Expression) *args_stack);
static void generate_statement(const Statement *stmt);
static void generate_stmt_label(const Statement *stmt);
static void generate_stmt_case(const Statement *stmt);
static void generate_stmt_compound(const Statement *stmt);
static void generate_stmt_decl(const Statement *stmt);
static void generate_stmt_expr(const Statement *stmt);
static void generate_stmt_null(const Statement *stmt);
static void generate_stmt_if(const Statement *stmt);
static void generate_stmt_switch(const Statement *stmt);
static void generate_stmt_while(const Statement *stmt);
static void generate_stmt_do(const Statement *stmt);
static void generate_stmt_for(const Statement *stmt);
static void generate_stmt_goto(const Statement *stmt);
static void generate_stmt_continue(const Statement *stmt);
static void generate_stmt_break(const Statement *stmt);
static void generate_stmt_return(const Statement *stmt);
static void generate_binary_operation(const Expression *expr, BinaryOperationKind kind);
static void generate_binop_add(const Expression *expr);
static void generate_binop_ptr_add(const Expression *expr);
static void generate_binop_sub(const Expression *expr);
static void generate_binop_ptr_sub(const Expression *expr);
static void generate_binop_ptr_diff(const Expression *expr);
static void generate_binop_mul(const Expression *expr);
static void generate_binop_div(const Expression *expr);
static void generate_binop_mod(const Expression *expr);
static void generate_binop_lshift(const Expression *expr);
static void generate_binop_rshift(const Expression *expr);
static void generate_binop_eq(const Expression *expr);
static void generate_binop_neq(const Expression *expr);
static void generate_binop_l(const Expression *expr);
static void generate_binop_leq(const Expression *expr);
static void generate_binop_and(const Expression *expr);
static void generate_binop_xor(const Expression *expr);
static void generate_binop_or(const Expression *expr);
static void generate_expression(const Expression *expr);
static void generate_expr_const(const Expression *expr);
static void generate_expr_var(const Expression *expr);
static void generate_expr_generic(const Expression *expr);
static void generate_expr_func(const Expression *expr);
static void generate_expr_member(const Expression *expr);
static void generate_expr_post_inc(const Expression *expr);
static void generate_expr_post_dec(const Expression *expr);
static void generate_expr_compound(const Expression *expr);
static void generate_expr_addr(const Expression *expr);
static void generate_expr_deref(const Expression *expr);
static void generate_expr_plus(const Expression *expr);
static void generate_expr_minus(const Expression *expr);
static void generate_expr_compl(const Expression *expr);
static void generate_expr_neg(const Expression *expr);
static void generate_expr_cast(const Expression *expr);
static void generate_expr_mul(const Expression *expr);
static void generate_expr_div(const Expression *expr);
static void generate_expr_mod(const Expression *expr);
static void generate_expr_add(const Expression *expr);
static void generate_expr_ptr_add(const Expression *expr);
static void generate_expr_sub(const Expression *expr);
static void generate_expr_ptr_sub(const Expression *expr);
static void generate_expr_ptr_diff(const Expression *expr);
static void generate_expr_lshift(const Expression *expr);
static void generate_expr_rshift(const Expression *expr);
static void generate_expr_eq(const Expression *expr);
static void generate_expr_neq(const Expression *expr);
static void generate_expr_l(const Expression *expr);
static void generate_expr_leq(const Expression *expr);
static void generate_expr_bit_and(const Expression *expr);
static void generate_expr_bit_xor(const Expression *expr);
static void generate_expr_bit_or(const Expression *expr);
static void generate_expr_log_and(const Expression *expr);
static void generate_expr_log_or(const Expression *expr);
static void generate_expr_cond(const Expression *expr);
static void generate_expr_assign(const Expression *expr);
static void generate_expr_add_eq(const Expression *expr);
static void generate_expr_ptr_add_eq(const Expression *expr);
static void generate_expr_sub_eq(const Expression *expr);
static void generate_expr_ptr_sub_eq(const Expression *expr);
static void generate_expr_mul_eq(const Expression *expr);
static void generate_expr_div_eq(const Expression *expr);
static void generate_expr_mod_eq(const Expression *expr);
static void generate_expr_lshift_eq(const Expression *expr);
static void generate_expr_rshift_eq(const Expression *expr);
static void generate_expr_and_eq(const Expression *expr);
static void generate_expr_xor_eq(const Expression *expr);
static void generate_expr_or_eq(const Expression *expr);
static void generate_expr_comma(const Expression *expr);
static void generate_expr_binary(const Expression *expr, BinaryOperationKind kind);
static void generate_expr_compound_assignment(const Expression *expr, BinaryOperationKind kind);
static size_t classify_args(const List(Expression) *args, bool pass_address, List(Expression) *args_reg, List(Expression) *args_xmm, List(Expression) *args_stack);
static void put_line(const char *fmt, ...);
static void put_line_with_tab(const char *fmt, ...);
#if(CHECK_STACK_SIZE == ENABLED)
static void check_stack_pointer(void);
#endif /* CHECK_STACK_SIZE */


// global variable
static const int STACK_POINTER_ALIGNMENT = 16; // alignment of stack pointer
static const char *arg_registers8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"}; // name of 8-bit registers for function arguments
static const char *arg_registers16[] = {"di", "si", "dx", "cx", "r8w", "r9w"}; // name of 16-bit registers for function arguments
static const char *arg_registers32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"}; // name of 32-bit registers for function arguments
static const char *arg_registers64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}; // name of 64-bit registers for function arguments
static const size_t ARG_REGISTERS_SIZE = 6; // number of registers for function arguments
static int lab_number; // sequential number for labels (see [Notes] below)
static int brk_number; // sequential number for break statements (see [Notes] below)
static int cnt_number; // sequential number for continue statements (see [Notes] below)
static int stack_size; // size of the stack
static unsigned int gp_offset; // offset from reg_save_area to the place where the next available general purpose argument register is saved
/*
# [Notes]
* Because functions to generate statements and expressions are recursively called,
    * `lab_number` is saved and incremented at the top of each sub-functions.
    * `brk_number` and `cnt_number` are saved at the top of each sub-functions and restored at the end of each sub-functions.
*/


/*
generate assembler code
*/
void generate(const Program *program)
{
    // use Intel syntax
    put_line(".intel_syntax noprefix");

    // generate global variables
    put_line(".data");
    for_each_entry(Variable, cursor, program->gvars)
    {
        Variable *gvar = get_element(Variable)(cursor);
        if(gvar->entity)
        {
            generate_gvar(gvar);
        }
    }

    // generate functions
    put_line(".text");
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
    put_line_with_tab("push %d", value);
    stack_size += STACK_ALIGNMENT;
}


/*
generate assembler code to push a register or memory contents to the stack
*/
static void generate_push_reg_or_mem(const char *reg_or_mem)
{
    put_line_with_tab("push %s", reg_or_mem);
    stack_size += STACK_ALIGNMENT;
}


/*
generate assembler code to push a xmm register to the stack
*/
static void generate_push_xmm(int number)
{
    put_line_with_tab("sub rsp, 8");
    put_line_with_tab("movsd [rsp], xmm%d", number);
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
        put_line_with_tab("push [rax+%lu]", offset - STACK_ALIGNMENT);
        stack_size += STACK_ALIGNMENT;
    }
}


/*
generate assembler code to push address stored in a register
*/
static void generate_push_offset(const char *reg, size_t offset)
{
    put_line_with_tab("mov rax, %s", reg);
    put_line_with_tab("sub rax, %lu", offset);
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code to pop from the stack to a register or memory
*/
static void generate_pop(const char *reg_or_mem)
{
    put_line_with_tab("pop %s", reg_or_mem);
    stack_size -= STACK_ALIGNMENT;
#if(CHECK_STACK_SIZE == ENABLED)
    assert(stack_size >= 0);
#endif /* CHECK_STACK_SIZE */
}


/*
generate assembler code to pop from the stack to a xmm register
*/
static void generate_pop_xmm(int number)
{
    put_line_with_tab("movsd xmm%d, [rsp]", number);
    put_line_with_tab("add rsp, 8");
    stack_size -= STACK_ALIGNMENT;
#if(CHECK_STACK_SIZE == ENABLED)
    assert(stack_size >= 0);
#endif /* CHECK_STACK_SIZE */
}


/*
generate assembler code to load value
*/
static void generate_load(const Type *type)
{
    generate_pop("rax");

    if(is_floating(type))
    {
        if(type->size == 4)
        {
            put_line_with_tab("movss xmm0, [rax]");
        }
        else
        {
            put_line_with_tab("movsd xmm0, [rax]");
        }
        generate_push_xmm(0);
    }
    else
    {
        if(type->size == 1)
        {
            put_line_with_tab("movsxb rax, byte ptr [rax]");
        }
        else if(type->size == 2)
        {
            put_line_with_tab("movsxw rax, word ptr [rax]");
        }
        else if(type->size <= 4)
        {
            put_line_with_tab("movsxd rax, dword ptr [rax]");
        }
        else if(type->size <= 8)
        {
            put_line_with_tab("mov rax, [rax]");
        }
        else if(type->size <= 16)
        {
            put_line_with_tab("mov r11, [rax+8]");
            put_line_with_tab("mov rax, [rax]");
            generate_push_reg_or_mem("r11");
        }
        generate_push_reg_or_mem("rax");
    }
}


/*
generate assembler code to store value
*/
static void generate_store(const Type *type)
{
    size_t size = type->size;

    if(is_floating(type))
    {
        generate_pop_xmm(0);
        generate_pop("rax");
        if(size <= 4)
        {
            put_line_with_tab("movss [rax], xmm0");
        }
        else
        {
            put_line_with_tab("movsd [rax], xmm0");
        }
        generate_push_xmm(0);
    }
    else
    {
        if(size <= 8)
        {
            generate_pop("rdi");
            generate_pop("rax");
            if(size == 1)
            {
                put_line_with_tab("mov byte ptr [rax], dil");
            }
            else if(size <= 2)
            {
                put_line_with_tab("mov word ptr [rax], di");
            }
            else if(size <= 4)
            {
                put_line_with_tab("mov dword ptr [rax], edi");
            }
            else
            {
                put_line_with_tab("mov [rax], rdi");
            }
        }
        else if(size <= 16)
        {
            generate_pop("rdi");
            generate_pop("r11");
            generate_pop("rax");
            put_line_with_tab("mov [rax], rdi");
            put_line_with_tab("mov [rax+8], r11");
        }
        else
        {
            // pop source address (rdi) and destination address (rax)
            generate_pop("rdi");
            generate_pop("rax");

            // copy bytes from rdi to rax
            size_t offset = 0;
            for(; offset + 8 <= size; offset += 8)
            {
                put_line_with_tab("mov r11, [rdi+%lu]", offset);
                put_line_with_tab("mov [rax+%lu], r11", offset);
            }
            for(; offset + 4 <= size; offset += 4)
            {
                put_line_with_tab("mov r11, [rdi+%lu]", offset);
                put_line_with_tab("mov dword ptr [rax+%lu], r11d", offset);
            }
            for(; offset + 2 <= size; offset += 2)
            {
                put_line_with_tab("mov r11, [rdi+%lu]", offset);
                put_line_with_tab("mov word ptr [rax+%lu], r11w", offset);
            }
            for(; offset + 1 <= size; offset += 1)
            {
                put_line_with_tab("mov r11, [rdi+%lu]", offset);
                put_line_with_tab("mov byte ptr [rax+%lu], r11b", offset);
            }
        }
        generate_push_reg_or_mem("rdi");
    }
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
            put_line_with_tab("mov rax, rbp");
            put_line_with_tab("sub rax, %lu", expr->var->offset);
            generate_push_reg_or_mem("rax");
        }
        else
        {
            put_line_with_tab("lea rax, %s[rip]", expr->var->name);
            generate_push_reg_or_mem("rax");
        }
        break;

    case EXPR_COMPOUND:
        generate_lvar_init(expr->var);
        put_line_with_tab("mov rax, rbp");
        put_line_with_tab("sub rax, %lu", expr->var->offset);
        generate_push_reg_or_mem("rax");
        break;

    case EXPR_DEREF:
        generate_expression(expr->operand);
        break;

    case EXPR_MEMBER:
        generate_lvalue(expr->operand);
        generate_pop("rax");
        put_line_with_tab("add rax, %lu", expr->member->offset);
        generate_push_reg_or_mem("rax");
        break;

    default:
        report_error(NULL, "expected lvalue");
        break;
    }
}


/*
generate assembler code of a local variable
*/
static void generate_lvar_init(const Variable *lvar)
{
    if(lvar->str != NULL)
    {
        // allocate memory for string-literal
        const char *content = lvar->str->content;
        Type *char_type = new_type(TY_CHAR, TQ_NONE);
        for(size_t index = 0; index < strlen(content) + 1; index++)
        {
            generate_push_offset("rbp", lvar->offset - index);
            put_line_with_tab("mov rax, %d", content[index]);
            generate_push_reg_or_mem("rax");
            generate_store(char_type);
            generate_pop("rax");
        }
    }
    else
    {
        for_each_entry(InitializerMap, cursor, lvar->inits)
        {
            InitializerMap *map = get_element(InitializerMap)(cursor);
            if(map->initialized)
            {
                if(map->zero)
                {
                    size_t size = 0;
                    size_t offset = lvar->offset - map->offset;
                    for(; size + 8 <= map->size; size += 8)
                    {
                        put_line_with_tab("mov rax, rbp");
                        put_line_with_tab("sub rax, %lu", offset - size);
                        put_line_with_tab("mov qword ptr [rax], 0");
                    }
                    for(; size + 4 <= map->size; size += 4)
                    {
                        put_line_with_tab("mov rax, rbp");
                        put_line_with_tab("sub rax, %lu", offset - size);
                        put_line_with_tab("mov dword ptr [rax], 0");
                    }
                    for(; size + 2 <= map->size; size += 2)
                    {
                        put_line_with_tab("mov rax, rbp");
                        put_line_with_tab("sub rax, %lu", offset - size);
                        put_line_with_tab("mov word ptr [rax], 0");
                    }
                    for(; size + 1 <= map->size; size += 1)
                    {
                        put_line_with_tab("mov rax, rbp");
                        put_line_with_tab("sub rax, %lu", offset - size);
                        put_line_with_tab("mov byte ptr [rax], 0");

                    }
                }
                else
                {
                    generate_push_offset("rbp", lvar->offset - map->offset);
                    if(map->label != NULL)
                    {
                        // generate pointer to label
                        put_line_with_tab("lea rax, %s[rip]", map->label);
                        generate_push_reg_or_mem("rax");
                    }
                    else
                    {
                        generate_expression(map->assign);
                    }
                    generate_store(map->type);
                    generate_pop("rax");
                }
            }
        }
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
        put_line(".global %s", gvar->name);
    }
    // adjust alignment
    if(gvar->align > gvar->type->align)
    {
        put_line_with_tab(".align %lu", gvar->align);
    }
    put_line("%s:", gvar->name);

    if(gvar->str != NULL)
    {
        // allocate memory for string-literal
        put_line_with_tab(".string \"%s\"", gvar->str->content);
    }
    else
    {
        generate_gvar_inits(gvar->inits);
    }
}


/*
generate data of a given size
*/
static void generate_gvar_data(const Type *type, const Expression *expr)
{
    if(is_floating(type))
    {
        size_t size = type->size;
        if(size == 4)
        {
            float value = (float)expr->value->float_value;
            put_line_with_tab(".long %u", *(unsigned int *)&value);
        }
        else
        {
            double value = expr->value->float_value;
            put_line_with_tab(".quad %lu", *(unsigned long *)&value);
        }
    }
    else
    {
        const Expression *base = NULL;
        long value = evaluate(expr, &base);
        if(base != NULL)
        {
            put_line_with_tab(".quad %s%+ld", base->var->name, value);
        }
        else
        {
            // allocate memory with an integer
            size_t size = type->size;
            if(size == 1)
            {
                put_line_with_tab(".byte %ld", value);
            }
            else if(size == 2)
            {
                put_line_with_tab(".value %ld", value);
            }
            else if(size == 4)
            {
                put_line_with_tab(".long %ld", value);
            }
            else
            {
                put_line_with_tab(".quad %ld", value);
            }
        }
    }
}


/*
generate data from list of initializers
*/
static void generate_gvar_inits(const List(InitializerMap) *inits)
{
    for_each_entry(InitializerMap, cursor, inits)
    {
        InitializerMap *map = get_element(InitializerMap)(cursor);
        if(map->initialized && !map->zero)
        {
            if(map->label != NULL)
            {
                // allocate memory with label
                put_line_with_tab(".quad %s", map->label);
            }
            else
            {
                if(map->assign->kind == EXPR_COMPOUND)
                {
                    generate_gvar_inits(map->assign->var->inits);
                }
                else
                {
                    generate_gvar_data(map->type, map->assign);
                }
            }
        }
        else
        {
            // allocate memory with zero
            put_line_with_tab(".zero %lu", map->size);
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
        put_line(".global %s", func->name);
    }
    put_line("%s:", func->name);

    // prologue: allocate stack for arguments and local variables and copy arguments
    bool pass_address = (get_parameter_class(func->type->base) == PC_MEMORY);
    stack_size = func->stack_size + pass_address * STACK_ALIGNMENT; // initialize stack size
    generate_push_reg_or_mem("rbp");
    put_line_with_tab("mov rbp, rsp");
    put_line_with_tab("sub rsp, %lu", stack_size);
    if(pass_address)
    {
        // save the hidden argument
        put_line_with_tab("mov [rbp-%lu], rdi", stack_size);
    }

    // arguments
    // separate arguments which are passed by registers from those passed by the stack
    size_t argc_reg = (pass_address ? 1 : 0);
    List(Variable) *args_reg = new_list(Variable)();
    List(Variable) *args_xmm = new_list(Variable)();
    List(Variable) *args_stack = new_list(Variable)();
    for_each_entry(Variable, cursor, func->args)
    {
        Variable *arg = get_element(Variable)(cursor);

        if(is_floating(arg->type))
        {
            add_list_entry_tail(Variable)(args_xmm, arg);
            continue;
        }

        // pass up to ARG_REGISTERS_SIZE arguments by registers
        if(argc_reg >= ARG_REGISTERS_SIZE)
        {
            add_list_entry_tail(Variable)(args_stack, arg);
            continue;
        }

        // pass arguments classified as MEMORY by the stack
        if(get_parameter_class(arg->type) == PC_MEMORY)
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

        put_line_with_tab("mov rax, rbp");
        put_line_with_tab("sub rax, %lu", arg->offset);
        if(is_struct_or_union(arg->type))
        {
            size_t argc_struct = adjust_alignment(arg->type->size, STACK_ALIGNMENT) / STACK_ALIGNMENT;
            for(size_t i = 0; i < argc_struct; i++)
            {
                put_line_with_tab("mov [rax+%lu], %s", i * STACK_ALIGNMENT, arg_registers64[argc_reg + i]);
            }
            argc_reg += argc_struct;
        }
        else
        {
            if(arg->type->size == 1)
            {
                put_line_with_tab("mov byte ptr [rax], %s", arg_registers8[argc_reg]);
            }
            else if(arg->type->size == 2)
            {
                put_line_with_tab("mov word ptr [rax], %s", arg_registers16[argc_reg]);
            }
            else if(arg->type->size == 4)
            {
                put_line_with_tab("mov dword ptr [rax], %s", arg_registers32[argc_reg]);
            }
            else
            {
                put_line_with_tab("mov [rax], %s", arg_registers64[argc_reg]);
            }
            argc_reg++;
        }
    }

    size_t argc_xmm = 0;
    for_each_entry(Variable, cursor, args_xmm)
    {
        // load argument from a register
        Variable *arg = get_element(Variable)(cursor);

        put_line_with_tab("mov rax, rbp");
        put_line_with_tab("sub rax, %lu", arg->offset);
        if(arg->type->size == 4)
        {
            put_line_with_tab("movss dword ptr [rax], xmm%d", argc_xmm);
        }
        else
        {
            put_line_with_tab("movsd [rax], xmm%d", argc_xmm);
        }
        argc_xmm++;
    }

    size_t args_stack_size = 0;
    for_each_entry(Variable, cursor, args_stack)
    {
        // load argument from the stack
        Variable *arg = get_element(Variable)(cursor);

        put_line_with_tab("mov rax, rbp");
        put_line_with_tab("sub rax, %lu", arg->offset);
        generate_push_reg_or_mem("rax");

        put_line_with_tab("mov rax, rbp");
        put_line_with_tab("add rax, %lu", args_stack_size + 2 * STACK_ALIGNMENT);
        generate_push_reg_or_mem("rax");

        generate_load(arg->type);
        generate_store(arg->type);

        args_stack_size += adjust_alignment(arg->type->size, STACK_ALIGNMENT);
    }

    if(func->type->variadic)
    {
        gp_offset = argc_reg * STACK_ALIGNMENT;
        for(size_t i = 0; i < ARG_REGISTERS_SIZE; i++)
        {
            put_line_with_tab("mov [rbp-%lu], %s", (ARG_REGISTERS_SIZE - i) * STACK_ALIGNMENT, arg_registers64[i]);
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
        put_line_with_tab("mov rax, [rbp-%lu]", func->stack_size + STACK_ALIGNMENT);
    }
    put_line_with_tab("leave");
    put_line_with_tab("ret");
}


/*
generate assembler code of function arguments
*/
static void generate_args(bool pass_address, const List(Expression) *args_reg, const List(Expression) *args_xmm, const List(Expression) *args_stack)
{
    // count number of registers used for passing arguments
    size_t argc_reg = (pass_address ? 1 : 0);
    for_each_entry(Expression, cursor, args_reg)
    {
        Expression *arg = get_element(Expression)(cursor);
        argc_reg += adjust_alignment(arg->type->size, STACK_ALIGNMENT) / STACK_ALIGNMENT;
    }

    // evaluate arguments and pass them by registers
    for_each_entry(Expression, cursor, args_reg)
    {
        Expression *arg = get_element(Expression)(cursor);
        generate_expression(arg);
    }
    for_each_entry_reversed(Expression, cursor, args_reg)
    {
        Expression *arg = get_element(Expression)(cursor);
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

    // count number of xmm registers used for passing arguments
    size_t argc_xmm = get_length(Expression)(args_xmm);

    // evaluate arguments and pass them by xmm registers
    for_each_entry(Expression, cursor, args_xmm)
    {
        Expression *arg = get_element(Expression)(cursor);
        generate_expression(arg);
    }
    for_each_entry_reversed(Expression, cursor, args_xmm)
    {
        argc_xmm--;
        generate_pop_xmm(argc_xmm);
    }

    // evaluate arguments and push them to the stack in reverse order
    for_each_entry_reversed(Expression, cursor, args_stack)
    {
        Expression *arg = get_element(Expression)(cursor);

        generate_expression(arg);
        if(get_parameter_class(arg->type) == PC_MEMORY)
        {
            generate_push_struct(arg->type);
        }
    }
}


/*
generate assembler code of a statement
*/
static void generate_statement(const Statement *stmt)
{

    switch(stmt->kind)
    {
    case STMT_LABEL:
        generate_stmt_label(stmt);
        return;

    case STMT_CASE:
        generate_stmt_case(stmt);
        return;

    case STMT_COMPOUND:
        generate_stmt_compound(stmt);
        return;

    case STMT_DECL:
        generate_stmt_decl(stmt);
        return;

    case STMT_EXPR:
        generate_stmt_expr(stmt);
        return;

    case STMT_NULL:
        generate_stmt_null(stmt);
        return;

    case STMT_IF:
        generate_stmt_if(stmt);
        return;

    case STMT_SWITCH:
        generate_stmt_switch(stmt);
        return;

    case STMT_WHILE:
        generate_stmt_while(stmt);
        return;

    case STMT_DO:
        generate_stmt_do(stmt);
        return;

    case STMT_FOR:
        generate_stmt_for(stmt);
        return;

    case STMT_GOTO:
        generate_stmt_goto(stmt);
        return;

    case STMT_CONTINUE:
        generate_stmt_continue(stmt);
        return;

    case STMT_BREAK:
        generate_stmt_break(stmt);
        return;

    case STMT_RETURN:
        generate_stmt_return(stmt);
        return;

    default:
        break;
    }
}


/*
generate assembler code of statement of kind STMT_LABEL
*/
static void generate_stmt_label(const Statement *stmt)
{
    put_line(".L%s:", stmt->ident);
    generate_statement(stmt->body);
}


/*
generate assembler code of statement of kind STMT_CASE
*/
static void generate_stmt_case(const Statement *stmt)
{
    put_line(".Lcase%d:", stmt->case_label);
    generate_statement(stmt->body);
}


/*
generate assembler code of statement of kind STMT_COMPOUND
*/
static void generate_stmt_compound(const Statement *stmt)
{
    for_each_entry(Statement, cursor, stmt->compound)
    {
        Statement *s = get_element(Statement)(cursor);
        generate_statement(s);
    }
}


/*
generate assembler code of statement of kind STMT_DECL
*/
static void generate_stmt_decl(const Statement *stmt)
{
    for_each_entry(Declaration, cursor, stmt->decl)
    {
        Declaration *decl = get_element(Declaration)(cursor);
        if((decl->var != NULL) && decl->var->local)
        {
            generate_lvar_init(decl->var);
        }
    }
}


/*
generate assembler code of statement of kind STMT_EXPR
*/
static void generate_stmt_expr(const Statement *stmt)
{
    generate_expression(stmt->expr);
    generate_pop("rax");
}


/*
generate assembler code of statement of kind STMT_NULL
*/
static void generate_stmt_null(const Statement *stmt)
{
    // do nothing
}


/*
generate assembler code of statement of kind STMT_IF
*/
static void generate_stmt_if(const Statement *stmt)
{
    int lab = lab_number;
    lab_number++;

    generate_expression(stmt->cond);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    if(stmt->false_case == NULL)
    {
        // if ( expression ) statement
        put_line_with_tab("je  .Lend%d", lab);
        generate_statement(stmt->true_case);
        put_line(".Lend%d:", lab);
    }
    else
    {
        // if ( expression ) statement else statement
        put_line_with_tab("je  .Lelse%d", lab);
        generate_statement(stmt->true_case);
        put_line_with_tab("jmp .Lend%d", lab);
        put_line(".Lelse%d:", lab);
        generate_statement(stmt->false_case);
        put_line(".Lend%d:", lab);
    }
}


/*
generate assembler code of statement of kind STMT_SWITCH
*/
static void generate_stmt_switch(const Statement *stmt)
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
        put_line_with_tab("cmp rax, %d", s->value);
        put_line_with_tab("je .Lcase%d", case_label);
    }
    if(stmt->default_case != NULL)
    {
        int case_label = lab_number;
        lab_number++;

        stmt->default_case->case_label = case_label;
        put_line_with_tab("jmp .Lcase%d", case_label);
    }

    generate_statement(stmt->body);
    put_line(".Lbreak%d:", lab);

    brk_number = brk;
}


/*
generate assembler code of statement of kind STMT_SWITCH
*/
static void generate_stmt_while(const Statement *stmt)
{
    int lab = lab_number;
    int brk = brk_number;
    int cnt = cnt_number;
    brk_number = cnt_number = lab_number;
    lab_number++;

    put_line(".Lbegin%d:", lab);
    put_line(".Lcontinue%d:", lab);
    generate_expression(stmt->cond);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("je  .Lend%d", lab);
    generate_statement(stmt->body);
    put_line_with_tab("jmp .Lbegin%d", lab);
    put_line(".Lend%d:", lab);
    put_line(".Lbreak%d:", lab);

    brk_number = brk;
    cnt_number = cnt;
}


/*
generate assembler code of statement of kind STMT_DO
*/
static void generate_stmt_do(const Statement *stmt)
{
    int lab = lab_number;
    int brk = brk_number;
    int cnt = cnt_number;
    brk_number = cnt_number = lab_number;
    lab_number++;

    put_line(".Lbegin%d:", lab);
    generate_statement(stmt->body);
    put_line(".Lcontinue%d:", lab);
    generate_expression(stmt->cond);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("jne .Lbegin%d", lab);
    put_line(".Lend%d:", lab);
    put_line(".Lbreak%d:", lab);

    brk_number = brk;
    cnt_number = cnt;
    return;
}


/*
generate assembler code of statement of kind STMT_FOR
*/
static void generate_stmt_for(const Statement *stmt)
{
    int lab = lab_number;
    int brk = brk_number;
    int cnt = cnt_number;
    brk_number = cnt_number = lab_number;
    lab_number++;

    if(stmt->preexpr != NULL)
    {
        generate_expression(stmt->preexpr);
        generate_pop("rax");
    }
    if(stmt->predecl != NULL)
    {
        generate_statement(stmt->predecl);
    }
    put_line(".Lbegin%d:", lab);
    if(stmt->cond != NULL)
    {
        generate_expression(stmt->cond);
        generate_pop("rax");
        put_line_with_tab("cmp rax, 0");
        put_line_with_tab("je  .Lend%d", lab);
    }
    generate_statement(stmt->body);
    put_line(".Lcontinue%d:", lab);
    if(stmt->postexpr != NULL)
    {
        generate_expression(stmt->postexpr);
        generate_pop("rax");
    }
    put_line_with_tab("jmp .Lbegin%d", lab);
    put_line(".Lend%d:", lab);
    put_line(".Lbreak%d:", lab);

    brk_number = brk;
    cnt_number = cnt;
    return;
}


/*
generate assembler code of statement of kind STMT_GOTO
*/
static void generate_stmt_goto(const Statement *stmt)
{
    put_line_with_tab("jmp .L%s", stmt->ident);
}


/*
generate assembler code of statement of kind STMT_CONTINUE
*/
static void generate_stmt_continue(const Statement *stmt)
{
    put_line_with_tab("jmp .Lcontinue%d", cnt_number);
}


/*
generate assembler code of statement of kind STMT_BREAK
*/
static void generate_stmt_break(const Statement *stmt)
{
    put_line_with_tab("jmp .Lbreak%d", brk_number);
}


/*
generate assembler code of statement of kind STMT_RETURN
*/
static void generate_stmt_return(const Statement *stmt)
{
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
    put_line_with_tab("leave");
    put_line_with_tab("ret");
}


/*
generate assembler code of binary operation
*/
static void generate_binary_operation(const Expression *expr, BinaryOperationKind kind)
{
    if(is_floating(expr->lhs->type))
    {
        // pop RHS to xmm1 and LHS to xmm0
        generate_pop_xmm(1);
        generate_pop_xmm(0);
    }
    else
    {
        // pop RHS to rdi and LHS to rax
        generate_pop("rdi");
        generate_pop("rax");
    }

    // execute operation
    switch(kind)
    {
    case BINOP_ADD:
        generate_binop_add(expr);
        break;

    case BINOP_PTR_ADD:
        generate_binop_ptr_add(expr);
        break;

    case BINOP_SUB:
        generate_binop_sub(expr);
        break;

    case BINOP_PTR_SUB:
        generate_binop_ptr_sub(expr);
        break;

    case BINOP_PTR_DIFF:
        generate_binop_ptr_diff(expr);
        break;

    case BINOP_MUL:
        generate_binop_mul(expr);
        break;

    case BINOP_DIV:
        generate_binop_div(expr);
        break;

    case BINOP_MOD:
        generate_binop_mod(expr);
        break;

    case BINOP_LSHIFT:
        generate_binop_lshift(expr);
        break;

    case BINOP_RSHIFT:
        generate_binop_rshift(expr);
        break;

    case BINOP_EQ:
        generate_binop_eq(expr);
        break;

    case BINOP_NEQ:
        generate_binop_neq(expr);
        break;

    case BINOP_L:
        generate_binop_l(expr);
        break;

    case BINOP_LEQ:
        generate_binop_leq(expr);
        break;

    case BINOP_AND:
        generate_binop_and(expr);
        break;

    case BINOP_XOR:
        generate_binop_xor(expr);
        break;

    case BINOP_OR:
        generate_binop_or(expr);
        break;

    default:
        break;
    }

    // push the result of operation
    if(is_floating(expr->type))
    {
        generate_push_xmm(0);
    }
    else
    {
        generate_push_reg_or_mem("rax");
    }
}


/*
generate assembler code of binary operation of kind BINOP_ADD
*/
static void generate_binop_add(const Expression *expr)
{
    switch(expr->type->kind)
    {
    case TY_FLOAT:
        put_line_with_tab("addss xmm0, xmm1");
        break;

    case TY_DOUBLE:
        put_line_with_tab("addsd xmm0, xmm1");
        break;

    default:
        put_line_with_tab("add rax, rdi");
        break;
    }
}


/*
generate assembler code of binary operation of kind BINOP_PTR_ADD
*/
static void generate_binop_ptr_add(const Expression *expr)
{
    put_line_with_tab("imul rdi, %lu", expr->type->base->size);
    generate_binop_add(expr);
}


/*
generate assembler code of binary operation of kind BINOP_SUB
*/
static void generate_binop_sub(const Expression *expr)
{
    switch(expr->type->kind)
    {
    case TY_FLOAT:
        put_line_with_tab("subss xmm0, xmm1");
        break;

    case TY_DOUBLE:
        put_line_with_tab("subsd xmm0, xmm1");
        break;

    default:
        put_line_with_tab("sub rax, rdi");
        break;
    }
}


/*
generate assembler code of binary operation of kind BINOP_PTR_SUB
*/
static void generate_binop_ptr_sub(const Expression *expr)
{
    put_line_with_tab("imul rdi, %lu", expr->type->base->size);
    generate_binop_sub(expr);
}


/*
generate assembler code of binary operation of kind BINOP_PTR_DIFF
*/
static void generate_binop_ptr_diff(const Expression *expr)
{
    put_line_with_tab("sub rax, rdi");
    put_line_with_tab("cqo");
    put_line_with_tab("mov rdi, %lu", expr->lhs->type->base->size);
    put_line_with_tab("idiv rdi");
}


/*
generate assembler code of binary operation of kind BINOP_MUL
*/
static void generate_binop_mul(const Expression *expr)
{
    switch(expr->type->kind)
    {
    case TY_FLOAT:
        put_line_with_tab("mulss xmm0, xmm1");
        break;

    case TY_DOUBLE:
        put_line_with_tab("mulsd xmm0, xmm1");
        break;

    default:
        put_line_with_tab("imul rax, rdi");
        break;
    }
}


/*
generate assembler code of binary operation of kind BINOP_DIV
*/
static void generate_binop_div(const Expression *expr)
{
    switch(expr->type->kind)
    {
    case TY_FLOAT:
        put_line_with_tab("divss xmm0, xmm1");
        break;

    case TY_DOUBLE:
        put_line_with_tab("divsd xmm0, xmm1");
        break;

    default:
        put_line_with_tab("cqo");
        put_line_with_tab("idiv rdi");
        break;
    }
}


/*
generate assembler code of binary operation of kind BINOP_MOD
*/
static void generate_binop_mod(const Expression *expr)
{
    put_line_with_tab("cqo");
    put_line_with_tab("idiv rdi");
    put_line_with_tab("mov rax, rdx");
}


/*
generate assembler code of binary operation of kind BINOP_LSHIFT
*/
static void generate_binop_lshift(const Expression *expr)
{
    put_line_with_tab("mov rcx, rdi");
    put_line_with_tab("shl rax, cl");
}


/*
generate assembler code of binary operation of kind BINOP_RSHIFT
*/
static void generate_binop_rshift(const Expression *expr)
{
    put_line_with_tab("mov rcx, rdi");
    put_line_with_tab("sar rax, cl");
}


/*
generate assembler code of binary operation of kind BINOP_EQ
*/
static void generate_binop_eq(const Expression *expr)
{
    if(is_floating(expr->lhs->type))
    {
        put_line_with_tab("%s xmm0, xmm1", (expr->lhs->type->kind == TY_FLOAT ? "ucomiss" : "ucomisd"));;
        put_line_with_tab("setnp al");
        put_line_with_tab("movzx eax, al");
        put_line_with_tab("mov edx, 0");
        put_line_with_tab("cmovne eax, edx");
    }
    else
    {
        put_line_with_tab("cmp rax, rdi");
        put_line_with_tab("sete al");
        put_line_with_tab("movzb rax, al");
    }
}


/*
generate assembler code of binary operation of kind BINOP_NEQ
*/
static void generate_binop_neq(const Expression *expr)
{
    if(is_floating(expr->lhs->type))
    {
        put_line_with_tab("%s xmm0, xmm1", (expr->lhs->type->kind == TY_FLOAT ? "ucomiss" : "ucomisd"));;
        put_line_with_tab("setp al");
        put_line_with_tab("movzx eax, al");
        put_line_with_tab("mov edx, 1");
        put_line_with_tab("cmovne eax, edx");
    }
    else
    {
        put_line_with_tab("cmp rax, rdi");
        put_line_with_tab("setne al");
        put_line_with_tab("movzb rax, al");
    }
}


/*
generate assembler code of binary operation of kind BINOP_L
*/
static void generate_binop_l(const Expression *expr)
{
    if(is_floating(expr->lhs->type))
    {
        put_line_with_tab("%s xmm1, xmm0", (expr->lhs->type->kind == TY_FLOAT ? "comiss" : "comisd"));;
        put_line_with_tab("seta al");
        put_line_with_tab("movzx eax, al");
    }
    else
    {
        put_line_with_tab("cmp rax, rdi");
        put_line_with_tab("%s al", (expr->lhs->type->kind == TY_ULONG) || (expr->rhs->type->kind == TY_ULONG) ? "setb" : "setl");
        put_line_with_tab("movzb rax, al");
    }
}


/*
generate assembler code of binary operation of kind BINOP_LEQ
*/
static void generate_binop_leq(const Expression *expr)
{
    if(is_floating(expr->lhs->type))
    {
        put_line_with_tab("%s xmm1, xmm0", (expr->lhs->type->kind == TY_FLOAT ? "comiss" : "comisd"));;
        put_line_with_tab("setnb al");
        put_line_with_tab("movzx eax, al");
    }
    else
    {
        put_line_with_tab("cmp rax, rdi");
        put_line_with_tab("%s al", (expr->lhs->type->kind == TY_ULONG) || (expr->rhs->type->kind == TY_ULONG) ? "setbe" : "setle");
        put_line_with_tab("movzb rax, al");
    }
}


/*
generate assembler code of binary operation of kind BINOP_AND
*/
static void generate_binop_and(const Expression *expr)
{
    put_line_with_tab("and rax, rdi");
}


/*
generate assembler code of binary operation of kind BINOP_XOR
*/
static void generate_binop_xor(const Expression *expr)
{
    put_line_with_tab("xor rax, rdi");
}


/*
generate assembler code of binary operation of kind BINOP_OR
*/
static void generate_binop_or(const Expression *expr)
{
    put_line_with_tab("or rax, rdi");
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
        generate_expr_const(expr);
        return;

    case EXPR_VAR:
        generate_expr_var(expr);
        return;

    case EXPR_GENERIC:
        generate_expr_generic(expr);
        return;

    case EXPR_COMPOUND:
        generate_expr_compound(expr);
        return;

    case EXPR_ADDR:
        generate_expr_addr(expr);
        return;

    case EXPR_DEREF:
        generate_expr_deref(expr);
        return;

    case EXPR_MEMBER:
        generate_expr_member(expr);
        return;

    case EXPR_PLUS:
        generate_expr_plus(expr);
        return;

    case EXPR_MINUS:
        generate_expr_minus(expr);
        return;

    case EXPR_COMPL:
        generate_expr_compl(expr);
        return;

    case EXPR_NEG:
        generate_expr_neg(expr);
        return;

    case EXPR_POST_INC:
        generate_expr_post_inc(expr);
        return;

    case EXPR_POST_DEC:
        generate_expr_post_dec(expr);
        return;

    case EXPR_CAST:
        generate_expr_cast(expr);
        return;

    case EXPR_LOG_AND:
        generate_expr_log_and(expr);
        return;

    case EXPR_LOG_OR:
        generate_expr_log_or(expr);
        return;

    case EXPR_COND:
        generate_expr_cond(expr);
        return;

    case EXPR_ASSIGN:
        generate_expr_assign(expr);
        return;

    case EXPR_ADD_EQ:
        generate_expr_add_eq(expr);
        return;

    case EXPR_PTR_ADD_EQ:
        generate_expr_ptr_add_eq(expr);
        return;

    case EXPR_SUB_EQ:
        generate_expr_sub_eq(expr);
        return;

    case EXPR_PTR_SUB_EQ:
        generate_expr_ptr_sub_eq(expr);
        return;

    case EXPR_MUL_EQ:
        generate_expr_mul_eq(expr);
        return;

    case EXPR_DIV_EQ:
        generate_expr_div_eq(expr);
        return;

    case EXPR_MOD_EQ:
        generate_expr_mod_eq(expr);
        return;

    case EXPR_LSHIFT_EQ:
        generate_expr_lshift_eq(expr);
        return;

    case EXPR_RSHIFT_EQ:
        generate_expr_rshift_eq(expr);
        return;

    case EXPR_AND_EQ:
        generate_expr_and_eq(expr);
        return;

    case EXPR_XOR_EQ:
        generate_expr_xor_eq(expr);
        return;

    case EXPR_OR_EQ:
        generate_expr_or_eq(expr);
        return;

    case EXPR_COMMA:
        generate_expr_comma(expr);
        return;

    case EXPR_FUNC:
        generate_expr_func(expr);
        return;

    case EXPR_ADD:
        generate_expr_add(expr);
        return;

    case EXPR_SUB:
        generate_expr_sub(expr);
        return;

    case EXPR_MUL:
        generate_expr_mul(expr);
        return;

    case EXPR_DIV:
        generate_expr_div(expr);
        return;

    case EXPR_PTR_ADD:
        generate_expr_ptr_add(expr);
        return;

    case EXPR_PTR_SUB:
        generate_expr_ptr_sub(expr);
        return;

    case EXPR_PTR_DIFF:
        generate_expr_ptr_diff(expr);
        return;

    case EXPR_MOD:
        generate_expr_mod(expr);
        return;

    case EXPR_LSHIFT:
        generate_expr_lshift(expr);
        return;

    case EXPR_RSHIFT:
        generate_expr_rshift(expr);
        return;

    case EXPR_EQ:
        generate_expr_eq(expr);
        return;

    case EXPR_NEQ:
        generate_expr_neq(expr);
        return;

    case EXPR_L:
        generate_expr_l(expr);
        return;

    case EXPR_LEQ:
        generate_expr_leq(expr);
        return;

    case EXPR_BIT_AND:
        generate_expr_bit_and(expr);
        return;

    case EXPR_BIT_XOR:
        generate_expr_bit_xor(expr);
        return;

    case EXPR_BIT_OR:
        generate_expr_bit_or(expr);
        return;

    default:
        break;
    }
}


/*
generate assembler code of expression of kind EXPR_CONST
*/
static void generate_expr_const(const Expression *expr)
{
    if(is_floating(expr->value->type))
    {
        put_line_with_tab("movsd xmm0, %s[rip]", expr->value->float_label);
        generate_push_xmm(0);
    }
    else
    {
        // It is assumed that the size of 'int' is 4 bytes.
        if(expr->value->int_value == (int)expr->value->int_value)
        {
            generate_push_imm(expr->value->int_value);
        }
        else
        {
            put_line_with_tab("mov rax, %ld", expr->value->int_value);
            generate_push_reg_or_mem("rax");
        }
    }
}


/*
generate assembler code of expression of kind EXPR_VAR
*/
static void generate_expr_var(const Expression *expr)
{
    generate_lvalue(expr);
    if(!(is_array(expr->var->type) || is_function(expr->var->type)))
    {
        generate_load(expr->var->type);
    }
}


/*
generate assembler code of expression of kind EXPR_GENERIC
*/
static void generate_expr_generic(const Expression *expr)
{
    generate_expression(expr->operand);
}


/*
generate assembler code of expression of kind EXPR_FUNC
*/
static void generate_expr_func(const Expression *expr)
{
    // System V ABI for x86-64 requires that
    // * rsp be a multiple of 16 before function call.
    // * Arguments be passed in rdi, rsi, rdx, rcx, r8, r9, and further ones be passed on the stack in reverse order.
    // * The number of floating point arguments be set to al if the callee is a variadic function.

    Expression *body = expr->operand->operand;
    if((body != NULL) && (body->var != NULL) && (strcmp(body->var->name, "__builtin_va_start") == 0))
    {
        generate_expression(get_first_element(Expression)(expr->args));
        generate_pop("rax");
        generate_push_reg_or_mem("r11");
        put_line_with_tab("mov dword ptr [rax], %d", gp_offset); // gp_offset
        put_line_with_tab("mov dword ptr [rax+4], %lu", ARG_REGISTERS_SIZE); // fp_offset
        put_line_with_tab("lea r11, [rbp+%lu]", 2 * STACK_ALIGNMENT);
        put_line_with_tab("mov qword ptr [rax+8], r11"); // overflow_arg_area
        put_line_with_tab("lea r11, [rbp-%lu]", REGISTER_SAVE_AREA_SIZE);
        put_line_with_tab("mov qword ptr [rax+16], r11"); // reg_save_area
        generate_pop("r11");
        return;
    }

#if(CHECK_STACK_SIZE == ENABLED)
    int current_stack = stack_size;
#endif /* CHECK_STACK_SIZE */
    int stack_adjustment = 0;
    size_t space = 0;
    bool pass_address = (get_parameter_class(expr->type) == PC_MEMORY);
    if(pass_address)
    {
        // provide space for the return value and pass the address of the space as the hidden 1st argument
        space = adjust_alignment(expr->type->size, STACK_ALIGNMENT) + STACK_ALIGNMENT;
        stack_adjustment += space;
        put_line_with_tab("lea rdi, [rsp-%lu]", space);
    }

    // classify arguments
    List(Expression) *args_reg = new_list(Expression)();
    List(Expression) *args_xmm = new_list(Expression)();
    List(Expression) *args_stack = new_list(Expression)();
    size_t args_stack_size = classify_args(expr->args, pass_address, args_reg, args_xmm, args_stack);

    // adjust alignment of rsp before generating arguments since the callee expects that arguments on the stack be just above the return address
    if(((stack_size + space + args_stack_size) % STACK_POINTER_ALIGNMENT) != 0)
    {
        stack_adjustment += STACK_ALIGNMENT;
    }
    if(stack_adjustment > 0)
    {
        stack_size += stack_adjustment;
        put_line_with_tab("sub rsp, %d", stack_adjustment);
    }

    // generate arguments
    generate_args(pass_address, args_reg, args_xmm, args_stack);
    stack_adjustment += args_stack_size; // increment the variable stack_adjustment here because the variable stack_size can be changed in the above function call

    if((body != NULL) && (body->var != NULL))
    {
        // call function by name
#if(CHECK_STACK_SIZE == ENABLED)
        assert(stack_size % STACK_POINTER_ALIGNMENT == 0);
        check_stack_pointer();
#endif /* CHECK_STACK_SIZE */
        put_line_with_tab("mov rax, 0");
        put_line_with_tab("call %s", body->var->name);
    }
    else
    {
        // call function through pointer
        generate_expression(expr->operand);
        generate_pop("r11");
#if(CHECK_STACK_SIZE == ENABLED)
        assert(stack_size % STACK_POINTER_ALIGNMENT == 0);
        check_stack_pointer();
#endif /* CHECK_STACK_SIZE */
        put_line_with_tab("mov rax, 0");
        put_line_with_tab("call r11");
    }

    // restore the stack
    if(stack_adjustment > 0)
    {
        put_line_with_tab("add rsp, %d", stack_adjustment);
        stack_size -= stack_adjustment;
    }
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
        put_line_with_tab("movzb rax, al");
    }
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of expression of kind EXPR_MEMBER
*/
static void generate_expr_member(const Expression *expr)
{
    generate_lvalue(expr);
    if(!is_array(expr->member->type))
    {
        generate_load(expr->member->type);
    }
}


/*
generate assembler code of expression of kind EXPR_POST_INC
*/
static void generate_expr_post_inc(const Expression *expr)
{
    generate_lvalue(expr->operand);
    generate_push_reg_or_mem("[rsp]");
    generate_load(expr->operand->type);
    generate_pop("rax");
    put_line_with_tab("mov rdx, rax");
    generate_push_reg_or_mem("rax");
    generate_push_imm(1);
    generate_pop("rdi");
    generate_pop("rax");
    if(is_integer(expr->type))
    {
        generate_binop_add(expr);
    }
    else
    {
        generate_binop_ptr_add(expr);
    }
    generate_push_reg_or_mem("rax");
    generate_store(expr->type);
    generate_pop("rax");
    generate_push_reg_or_mem("rdx");
}


/*
generate assembler code of expression of kind EXPR_POST_DEC
*/
static void generate_expr_post_dec(const Expression *expr)
{
    generate_lvalue(expr->operand);
    generate_push_reg_or_mem("[rsp]");
    generate_load(expr->operand->type);
    generate_pop("rax");
    put_line_with_tab("mov rdx, rax");
    generate_push_reg_or_mem("rax");
    generate_push_imm(1);
    generate_pop("rdi");
    generate_pop("rax");
    if(is_integer(expr->type))
    {
        generate_binop_sub(expr);
    }
    else
    {
        generate_binop_ptr_sub(expr);
    }
    generate_push_reg_or_mem("rax");
    generate_store(expr->type);
    generate_pop("rax");
    generate_push_reg_or_mem("rdx");
}


/*
generate assembler code of expression of kind EXPR_COMPOUND
*/
static void generate_expr_compound(const Expression *expr)
{
    generate_lvalue(expr);
    if(!(is_array(expr->var->type) || is_function(expr->var->type)))
    {
        generate_load(expr->var->type);
    }
}


/*
generate assembler code of expression of kind EXPR_ADDR
*/
static void generate_expr_addr(const Expression *expr)
{
    generate_lvalue(expr->operand);
}


/*
generate assembler code of expression of kind EXPR_DEREF
*/
static void generate_expr_deref(const Expression *expr)
{
    generate_expression(expr->operand);
    if(!(is_array(expr->type) || is_function(expr->type)))
    {
        generate_load(expr->type);
    }
}


/*
generate assembler code of expression of kind EXPR_PLUS
*/
static void generate_expr_plus(const Expression *expr)
{
    generate_expression(expr->operand);
}


/*
generate assembler code of expression of kind EXPR_MINUS
*/
static void generate_expr_minus(const Expression *expr)
{
    generate_expression(expr->operand);
    generate_pop("rax");
    put_line_with_tab("neg rax");
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of expression of kind EXPR_COMPL
*/
static void generate_expr_compl(const Expression *expr)
{
    generate_expression(expr->operand);
    generate_pop("rax");
    put_line_with_tab("not rax");
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of expression of kind EXPR_NEG
*/
static void generate_expr_neg(const Expression *expr)
{
    generate_expression(expr->operand);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("sete al");
    put_line_with_tab("movzb rax, al");
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of expression of kind EXPR_CAST
*/
static void generate_expr_cast(const Expression *expr)
{
    TypeKind kind_from = expr->operand->type->kind;
    TypeKind kind_to = expr->type->kind;

    generate_expression(expr->operand);
    if(is_floating(expr->operand->type))
    {
        generate_pop_xmm(0);
        if(kind_from == TY_FLOAT)
        {
            if(kind_to == TY_DOUBLE)
            {
                put_line_with_tab("cvtss2sd xmm0, xmm0");
            }
            else
            {
                put_line_with_tab("cvtss2si rax, xmm0");
            }
        }
        else
        {
            if(kind_to == TY_FLOAT)
            {
                put_line_with_tab("cvtsd2ss xmm0, xmm0");
            }
            else
            {
                put_line_with_tab("cvtsd2si rax, xmm0");
            }
        }
    }
    else
    {
        generate_pop("rax");
        if(kind_to == TY_FLOAT)
        {
            put_line_with_tab("cvtsi2ss xmm0, rax");
        }
        else if(kind_to == TY_DOUBLE)
        {
            put_line_with_tab("cvtsi2sd xmm0, rax");
        }
        else
        {
            if(expr->type->size == 1)
            {
                put_line_with_tab("movsxb rax, al");
            }
            else if(expr->type->size == 2)
            {
                put_line_with_tab("movsxw rax, ax");
            }
            else if(expr->type->size == 4)
            {
                put_line_with_tab("movsxd rax, eax");
            }
        }
    }

    if(is_floating(expr->type))
    {
        generate_push_xmm(0);
    }
    else
    {
        generate_push_reg_or_mem("rax");
    }
}


/*
generate assembler code of expression of kind EXPR_MUL
*/
static void generate_expr_mul(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_MUL);
}


/*
generate assembler code of expression of kind EXPR_DIV
*/
static void generate_expr_div(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_DIV);
}


/*
generate assembler code of expression of kind EXPR_MOD
*/
static void generate_expr_mod(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_MOD);
}


/*
generate assembler code of expression of kind EXPR_ADD
*/
static void generate_expr_add(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_ADD);
}


/*
generate assembler code of expression of kind EXPR_PTR_ADD
*/
static void generate_expr_ptr_add(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_PTR_ADD);
}


/*
generate assembler code of expression of kind EXPR_SUB
*/
static void generate_expr_sub(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_SUB);
}


/*
generate assembler code of expression of kind EXPR_PTR_SUB
*/
static void generate_expr_ptr_sub(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_PTR_SUB);
}


/*
generate assembler code of expression of kind EXPR_PTR_DIFF
*/
static void generate_expr_ptr_diff(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_PTR_DIFF);
}


/*
generate assembler code of expression of kind EXPR_LSHIFT
*/
static void generate_expr_lshift(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_LSHIFT);
}


/*
generate assembler code of expression of kind EXPR_RSHIFT
*/
static void generate_expr_rshift(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_RSHIFT);
}


/*
generate assembler code of expression of kind EXPR_EQ
*/
static void generate_expr_eq(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_EQ);
}


/*
generate assembler code of expression of kind EXPR_NEQ
*/
static void generate_expr_neq(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_NEQ);
}


/*
generate assembler code of expression of kind EXPR_L
*/
static void generate_expr_l(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_L);
}


/*
generate assembler code of expression of kind EXPR_LEQ
*/
static void generate_expr_leq(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_LEQ);
}


/*
generate assembler code of expression of kind EXPR_BIT_AND
*/
static void generate_expr_bit_and(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_AND);
}


/*
generate assembler code of expression of kind EXPR_BIT_XOR
*/
static void generate_expr_bit_xor(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_XOR);
}


/*
generate assembler code of expression of kind EXPR_BIT_OR
*/
static void generate_expr_bit_or(const Expression *expr)
{
    generate_expr_binary(expr, BINOP_OR);
}


/*
generate assembler code of expression of kind EXPR_LOG_AND
*/
static void generate_expr_log_and(const Expression *expr)
{
    // note that RHS is not evaluated if LHS is equal to 0
    int lab = lab_number;
    lab_number++;

    generate_expression(expr->lhs);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("je .Lfalse%d", lab);
    generate_expression(expr->rhs);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("je .Lfalse%d", lab);
    put_line_with_tab("mov rax, 1");
    put_line_with_tab("jmp .Lend%d", lab);
    put_line(".Lfalse%d:", lab);
    put_line_with_tab("mov rax, 0");
    put_line(".Lend%d:", lab);
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of expression of kind EXPR_LOG_OR
*/
static void generate_expr_log_or(const Expression *expr)
{
    // note that RHS is not evaluated if LHS is not equal to 0
    int lab = lab_number;
    lab_number++;

    generate_expression(expr->lhs);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("jne .Ltrue%d", lab);
    generate_expression(expr->rhs);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("jne .Ltrue%d", lab);
    put_line_with_tab("mov rax, 0");
    put_line_with_tab("jmp .Lend%d", lab);
    put_line(".Ltrue%d:", lab);
    put_line_with_tab("mov rax, 1");
    put_line(".Lend%d:", lab);
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of expression of kind EXPR_COND
*/
static void generate_expr_cond(const Expression *expr)
{
    // note that either second operand or third operand is evaluated
    int lab = lab_number;
    lab_number++;

    generate_expression(expr->operand);
    generate_pop("rax");
    put_line_with_tab("cmp rax, 0");
    put_line_with_tab("je .Lelse%d", lab);
    generate_expression(expr->lhs);
    generate_pop("rax");
    put_line_with_tab("jmp .Lend%d", lab);
    put_line(".Lelse%d:", lab);
    generate_expression(expr->rhs);
    generate_pop("rax");
    put_line(".Lend%d:", lab);
    generate_push_reg_or_mem("rax");
}


/*
generate assembler code of expression of kind EXPR_ASSIGN
*/
static void generate_expr_assign(const Expression *expr)
{
    generate_lvalue(expr->lhs);
    generate_expression(expr->rhs);
    generate_store(expr->type);
}


/*
generate assembler code of expression of kind EXPR_ADD_EQ
*/
static void generate_expr_add_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_ADD);
}


/*
generate assembler code of expression of kind EXPR_PTR_ADD_EQ
*/
static void generate_expr_ptr_add_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_PTR_ADD);
}


/*
generate assembler code of expression of kind EXPR_SUB_EQ
*/
static void generate_expr_sub_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_SUB);
}


/*
generate assembler code of expression of kind EXPR_PTR_SUB_EQ
*/
static void generate_expr_ptr_sub_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_PTR_SUB);
}


/*
generate assembler code of expression of kind EXPR_MUL_EQ
*/
static void generate_expr_mul_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_MUL);
}


/*
generate assembler code of expression of kind EXPR_DIV_EQ
*/
static void generate_expr_div_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_DIV);
}


/*
generate assembler code of expression of kind EXPR_MOD_EQ
*/
static void generate_expr_mod_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_MOD);
}


/*
generate assembler code of expression of kind EXPR_LSHIFT_EQ
*/
static void generate_expr_lshift_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_LSHIFT);
}


/*
generate assembler code of expression of kind EXPR_RSHIFT_EQ
*/
static void generate_expr_rshift_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_RSHIFT);
}


/*
generate assembler code of expression of kind EXPR_AND_EQ
*/
static void generate_expr_and_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_AND);
}


/*
generate assembler code of expression of kind EXPR_XOR_EQ
*/
static void generate_expr_xor_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_XOR);
}


/*
generate assembler code of expression of kind EXPR_OR_EQ
*/
static void generate_expr_or_eq(const Expression *expr)
{
    generate_expr_compound_assignment(expr, BINOP_OR);
}


/*
generate assembler code of expression of kind EXPR_COMMA
*/
static void generate_expr_comma(const Expression *expr)
{
    generate_expression(expr->lhs);
    generate_pop("rax");
    generate_expression(expr->rhs);
}


/*
generate assembler code of an expression with a binary operator
*/
static void generate_expr_binary(const Expression *expr, BinaryOperationKind kind)
{
    generate_expression(expr->lhs);
    generate_expression(expr->rhs);
    generate_binary_operation(expr, kind);
}


/*
generate assembler code of a compound assignment expression
*/
static void generate_expr_compound_assignment(const Expression *expr, BinaryOperationKind kind)
{
    generate_lvalue(expr->lhs);
    generate_push_reg_or_mem("[rsp]");
    generate_load(expr->lhs->type);
    generate_expression(expr->rhs);
    generate_binary_operation(expr, kind);
    generate_store(expr->type);
}


/*
separate arguments which are passed by registers from those passed by the stack
* This function returns size of the stack allocated for arguments.
*/
static size_t classify_args(const List(Expression) *args, bool pass_address, List(Expression) *args_reg, List(Expression) *args_xmm, List(Expression) *args_stack)
{
    size_t arg_stack_size = 0;
    size_t argc_reg = (pass_address ? 1 : 0);

    for_each_entry(Expression, cursor, args)
    {
        Expression *arg = get_element(Expression)(cursor);

        if(is_floating(arg->type))
        {
            add_list_entry_tail(Expression)(args_xmm, arg);
            continue;
        }

        // pass up to ARG_REGISTERS_SIZE arguments by registers
        if(argc_reg >= ARG_REGISTERS_SIZE)
        {
            add_list_entry_tail(Expression)(args_stack, arg);
            arg_stack_size += adjust_alignment(arg->type->size, STACK_ALIGNMENT);
            continue;
        }

        // pass arguments classified as MEMORY by the stack
        ParameterClassKind param_class = get_parameter_class(arg->type);
        if(param_class == PC_MEMORY)
        {
            add_list_entry_tail(Expression)(args_stack, arg);
            arg_stack_size += adjust_alignment(arg->type->size, STACK_ALIGNMENT);
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
                arg_stack_size += adjust_alignment(arg->type->size, STACK_ALIGNMENT);
            }
            continue;
        }

        add_list_entry_tail(Expression)(args_reg, arg);
        argc_reg++;
    }

    return arg_stack_size;
}


/*
output a line of assembler code
*/
static void put_line(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fputc('\n', stdout);
}


/*
output a line of assembler code with a tab
*/
static void put_line_with_tab(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fputc('\t', stdout);
    vfprintf(stdout, fmt, ap);
    fputc('\n', stdout);
}


#if(CHECK_STACK_SIZE == ENABLED)
/*
generate instructions to check alignment of the stack pointer at function call
* This function generates assembler code which triggers segmentation fault if rsp is not properly aligned.
*/
static void check_stack_pointer(void)
{
    int lab = lab_number;
    lab_number++;

    put_line_with_tab("mov r10, rsp");
    put_line_with_tab("and r10, 0x0F");
    put_line_with_tab("cmp r10, 0");
    put_line_with_tab("je .Lcall%d", lab);
    put_line_with_tab("mov r10, [0]"); // trigger segmentation fault
    put_line(".Lcall%d:", lab);
}
#endif /* CHECK_STACK_SIZE */
