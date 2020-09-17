/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser for expression
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declaration.h"
#include "expression.h"
#include "parser.h"
#include "tokenizer.h"
#include "type.h"

// definition of list operations
#include "list.h"
define_list_operations(Expression)
define_list_operations(GenericAssociation)

// function prototype
static Expression *new_node_unary(ExpressionKind kind, Expression *operand);
static Expression *new_node_cast(Type *type, Expression *operand);
static Expression *primary(void);
static Expression *generic_selection(void);
static List(GenericAssociation) *generic_assoc_list(void);
static GenericAssociation *generic_association(void);
static Expression *postfix(void);
static List(Expression) *arg_expr_list(void);
static Expression *unary(void);
static Expression *cast(void);
static Expression *multiplicative(void);
static Expression *additive(void);
static Expression *shift(void);
static Expression *relational(void);
static Expression *equality(void);
static Expression *bitwise_and(void);
static Expression *bitwise_xor(void);
static Expression *bitwise_or(void);
static Expression *logical_and(void);
static Expression *logical_or(void);
static Expression *conditional(void);
static Expression *search_generic_assoc_list(const Expression *control, const List(GenericAssociation) *assoc_list);
static Type *apply_integer_promotion(Type *type);
static Type *apply_arithmetic_conversion(Type *lhs_type, Type *rhs_type);
static Expression *apply_implicit_conversion(Expression *expr);
static bool check_constraint_binary(ExpressionKind kind, const Type *lhs_type, const Type *rhs_type);
static bool is_modifiable_lvalue(const Expression *expr);
static bool is_const_qualified(const Type *type);
static char *new_compound_literal_label(void);

// global variable
static int cl_number; // sequential number for compound-literal


/*
make a new expression
*/
Expression *new_expression(ExpressionKind kind, Type *type)
{
    Expression *node = calloc(1, sizeof(Expression));
    node->kind = kind;
    node->type = type;
    node->lhs = NULL;
    node->rhs = NULL;
    node->operand = NULL;
    node->value = 0;
    node->str = NULL;
    node->var = NULL;
    node->member = NULL;
    node->args = NULL;
    node->lvalue = false;

    return node;
}


/*
make a new node for constant
*/
Expression *new_node_constant(TypeKind kind, long value)
{
    Expression *node = new_expression(EXPR_CONST, new_type(kind, TQ_NONE));
    node->value = value;

    return node;
}


/*
make a new node for subscripting
*/
Expression *new_node_subscript(Expression *base, size_t index)
{
    Expression *addr = new_node_binary(EXPR_PTR_ADD, base, new_node_constant(TY_ULONG, index));
    Expression *dest = new_node_unary(EXPR_DEREF, addr);

    return dest;
}


/*
make a new node for members of structure or union
*/
Expression *new_node_member(Expression *operand, Member *member)
{
    Expression *node = new_expression(EXPR_MEMBER, copy_type(member->type, TQ_NONE));
    node->operand = operand;
    node->member = member;

    return node;
}


/*
make a new node for unary operations
*/
static Expression *new_node_unary(ExpressionKind kind, Expression *operand)
{
    // determine type of unary expression
    Type *type;
    switch(kind)
    {
    case EXPR_ADDR:
        type = new_type_pointer(operand->type);
        break;

    case EXPR_DEREF:
        // An array type is also converted to the element type.
        type = operand->type->base;
        type = copy_type(type, type->qual);
        break;

    case EXPR_POST_INC:
    case EXPR_POST_DEC:
    case EXPR_PLUS:
    case EXPR_MINUS:
    case EXPR_COMPL:
        type = operand->type;
        type = copy_type(type, type->qual);
        break;

    case EXPR_NEG:
    default:
        type = new_type(TY_INT, TQ_NONE);
        break;
    }

    Expression *node = new_expression(kind, type);
    node->operand = operand;
    if(kind == EXPR_DEREF)
    {
        node->lvalue = true;
    }

    return node;
}


/*
make a new node for cast operation
*/
static Expression *new_node_cast(Type *type, Expression *operand)
{
    if(type != operand->type)
    {
        Expression *node = new_expression(EXPR_CAST, type);
        node->operand = operand;

        return node;
    }
    else
    {
        return operand;
    }
}


/*
make a new node for binary operations
*/
Expression *new_node_binary(ExpressionKind kind, Expression *lhs, Expression *rhs)
{
    // determine type of binary expression
    Type *type;
    switch(kind)
    {
    case EXPR_ADD:
    case EXPR_PTR_ADD:
    case EXPR_SUB:
    case EXPR_PTR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    case EXPR_LSHIFT:
    case EXPR_RSHIFT:
    case EXPR_BIT_AND:
    case EXPR_BIT_XOR:
    case EXPR_BIT_OR:
    case EXPR_ASSIGN:
    case EXPR_ADD_EQ:
    case EXPR_PTR_ADD_EQ:
    case EXPR_SUB_EQ:
    case EXPR_PTR_SUB_EQ:
    case EXPR_MUL_EQ:
    case EXPR_DIV_EQ:
    case EXPR_MOD_EQ:
    case EXPR_LSHIFT_EQ:
    case EXPR_RSHIFT_EQ:
    case EXPR_AND_EQ:
    case EXPR_XOR_EQ:
    case EXPR_OR_EQ:
        type = copy_type(lhs->type, TQ_NONE);
        break;

    case EXPR_PTR_DIFF:
        // The type of the result of pointer difference is 'ptrdiff_t'.
        // This implementation regards 'ptrdiff_t' as 'long'.
        type = new_type(TY_LONG, TQ_NONE);
        break;

    case EXPR_COMMA:
        type = copy_type(rhs->type, TQ_NONE);
        break;

    case EXPR_L:
    case EXPR_LEQ:
    case EXPR_EQ:
    case EXPR_NEQ:
    case EXPR_LOG_AND:
    case EXPR_LOG_OR:
    default:
        type = new_type(TY_INT, TQ_NONE);
        break;
    }

    Expression *node = new_expression(kind, type);
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}


/*
make a primary expression
```
primary ::= identifier
          | constant
          | string-literal
          | "(" expression ")"
          | generic-selection
generic-selection ::= "_Generic" "(" assign "," generic-assoc-list ")"
generic-assoc-list ::= generic-association ("," generic-association)*
generic-association ::= (type-name | "default") ":" assign
```
*/
static Expression *primary(void)
{
    // generic selection
    if(peek_reserved("_Generic"))
    {
        return generic_selection();
    }

    // expression in brackets
    if(consume_reserved("("))
    {
        Expression *node = expression();
        expect_reserved(")");
        return node;
    }

    // identifier
    Token *token;
    if(consume_token(TK_IDENT, &token))
    {
        Identifier *ident = find_identifier(token);
        if(ident != NULL)
        {
            if(ident->var != NULL)
            {
                // variable
                Variable *var = ident->var;
                Type *type = var->type;
                Expression *node = new_expression(EXPR_VAR, type);
                node->var = var;
                node->lvalue = !is_function(type);
                return node;
            }
            else if(ident->en != NULL)
            {
                // enumeration
                return new_node_constant(TY_INT, ident->en->value);
            }
        }

        const char *name = make_identifier(token);
        if(strcmp(name, "__builtin_va_start") == 0)
        {
            Type *base = new_type(TY_VOID, TQ_NONE);
            List(Type) *args = new_list(Type)();
            add_list_entry_tail(Type)(args, new_type(TY_VOID, TQ_NONE));
            Type *type = new_type_function(base, args, false);
            Variable *var = new_gvar(name, type, SC_NONE, false);
            Expression *node = new_expression(EXPR_VAR, type);
            node->var = var;
            return node;
        }

        if(peek_reserved("("))
        {
            // implicitly assume that the token denotes a function which returns int
            Type *base = new_type(TY_INT, TQ_NONE);
            List(Type) *args = new_list(Type)();
            add_list_entry_tail(Type)(args, new_type(TY_VOID, TQ_NONE));
            Type *type = new_type_function(base, args, false);
            Variable *var = new_gvar(name, type, SC_NONE, false);
            Expression *node = new_expression(EXPR_VAR, type);
            node->var = var;
#if(WARN_IMPLICIT_DECLARATION_OF_FUNCTION == ENABLED)
            report_warning(token->str, "implicit declaration of function '%s'\n", name);
#endif /* WARN_IMPLICIT_DECLARATION_OF_FUNCTION */
            return node;
        }

        report_error(token->str, "undefined identifier '%s'", name);
    }

    // string-literal
    if(consume_token(TK_STR, &token))
    {
        StringLiteral *str = new_string(token);
        Variable *var = str->var;
        Type *type = var->type;
        Expression *node = new_expression(EXPR_VAR, type);
        node->str = str;
        node->var = var;
        node->lvalue = true;
        return node;
    }

    // constant
    token = expect_constant();
    return new_node_constant(token->type, token->value);
}


/*
make a generic selection
```
generic-selection ::= "_Generic" "(" assign "," generic-assoc-list ")"
```
*/
static Expression *generic_selection(void)
{
    Expression *node = new_expression(EXPR_GENERIC, NULL);
    expect_reserved("_Generic");
    expect_reserved("(");
    Expression *control = assign();
    expect_reserved(",");
    List(GenericAssociation) *assoc_list = generic_assoc_list();
    expect_reserved(")");
    node->operand = search_generic_assoc_list(control, assoc_list);

    return node;
}


/*
make a generic-assoc-list
```
generic-assoc-list ::= generic-association ("," generic-association)*
```
*/
static List(GenericAssociation) *generic_assoc_list(void)
{
    List(GenericAssociation) *list = new_list(GenericAssociation)();

    add_list_entry_tail(GenericAssociation)(list, generic_association());
    while(consume_reserved(","))
    {
        add_list_entry_tail(GenericAssociation)(list, generic_association());
    }

    return list;
}


/*
make a generic association
```
generic-assoc-list ::= generic-association ("," generic-association)*
```
*/
static GenericAssociation *generic_association(void)
{
    GenericAssociation *gen_assoc = calloc(1, sizeof(GenericAssociation));
    gen_assoc->type = consume_reserved("default") ? NULL : type_name();
    expect_reserved(":");
    gen_assoc->assign = assign();

    return gen_assoc;
}


/*
make a postfix expression
```
postfix ::= primary
          | postfix "[" expression "]"
          | postfix "(" arg-expr-list? ")"
          | postfix "." identifier
          | postfix "->" identifier
          | postfix "++"
          | postfix "--"
          | "(" type-name ")" "{" initializer-list ","? "}"
```
*/
static Expression *postfix(void)
{
    Expression *node;

    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_type_name())
        {
            // compound-literal
            Type *type = type_name();
            expect_reserved(")");
            node = new_expression(EXPR_COMPOUND, type);
            node->var = new_lvar(new_compound_literal_label(), type);
            node->var->inits = make_initializer_map(type, initializer());
            goto postfix_end;
        }
    }
    set_token(saved_token);

    // parse tokens while finding a postfix operator
    node = primary();
    while(true)
    {
        if(consume_reserved("["))
        {
            // array subscripting
            Expression *operand;
            Expression *index = expression();

            node = apply_implicit_conversion(node);
            index = apply_implicit_conversion(index);

            // check constraints
            if(is_pointer(node->type) && node->type->base->complete && is_integer(index->type))
            {
                operand = new_node_binary(EXPR_PTR_ADD, node, index);
            }
            else if(is_integer(node->type) && is_pointer(index->type) && index->type->base->complete)
            {
                operand = new_node_binary(EXPR_PTR_ADD, index, node);
            }
            else
            {
                report_error(NULL, "invalid operands to subscription operator []");
            }

            node = new_node_unary(EXPR_DEREF, operand);
            expect_reserved("]");
        }
        else if(consume_reserved("("))
        {
            // function call
            node = apply_implicit_conversion(node);

            if(is_function(node->type->base))
            {
                Expression *func_node = new_expression(EXPR_FUNC, node->type->base->base); // dereference pointer and get type of return value
                func_node->operand = node;
                // parse arguments
                if(consume_reserved(")"))
                {
                    func_node->args = new_list(Expression)(); // make a dummy list
                }
                else
                {
                    func_node->args = arg_expr_list();
                    expect_reserved(")");
                }
                node = func_node;
            }
            else
            {
                report_error(NULL, "expected function");
            }
        }
        else if(consume_reserved("."))
        {
            // access to member (by value)
            Token *token = expect_identifier();

            // check constraints
            if(is_struct_or_union(node->type))
            {
                Expression *struct_node = node;
                node = new_node_member(struct_node, find_member(token, struct_node->type));
                node->lvalue = struct_node->lvalue;
            }
            else
            {
                report_error(NULL, "expected structure or union");
            }
        }
        else if(consume_reserved("->"))
        {
            // access to member (by pointer)
            Token *token = expect_identifier();

            node = apply_implicit_conversion(node);
            if(is_pointer(node->type) && is_struct_or_union(node->type->base))
            {
                Expression *struct_node = new_node_unary(EXPR_DEREF, node);
                node = new_node_member(struct_node, find_member(token, struct_node->type));
                node->lvalue = true;
            }
            else
            {
                report_error(NULL, "expected pointer to structure or union");
            }
        }
        else if(consume_reserved("++"))
        {
            // postfix increment
            // check constraints
            if(!is_modifiable_lvalue(node))
            {
                report_error(NULL, "expected modifiable lvalue");
            }
            if(is_real(node->type) || is_pointer(node->type))
            {
                node = new_node_unary(EXPR_POST_INC, node);
            }
            else
            {
                report_error(NULL, "wrong type argument to increment");
            }
        }
        else if(consume_reserved("--"))
        {
            // postfix decrement
            // check constraints
            if(!is_modifiable_lvalue(node))
            {
                report_error(NULL, "expected modifiable lvalue");
            }
            if(is_real(node->type) || is_pointer(node->type))
            {
                node = new_node_unary(EXPR_POST_DEC, node);
            }
            else
            {
                report_error(NULL, "wrong type argument to decrement");
            }
        }
        else
        {
            break;
        }
    }

postfix_end:
    return node;
}


/*
make an argument expression list
```
arg-expr-list ::= assign ("," assign)*
```
*/
static List(Expression) *arg_expr_list(void)
{
    List(Expression) *args = new_list(Expression)();
    add_list_entry_tail(Expression)(args, apply_implicit_conversion(assign()));

    // parse arguments
    while(consume_reserved(","))
    {
        // append the argument at the head in order to push arguments in reverse order when generating assembler code
        add_list_entry_tail(Expression)(args, apply_implicit_conversion(assign()));
    }

    return args;
}


/*
make an unary expression
```
unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | "sizeof" unary
        | "sizeof" "(" type-name ")"
        | "_Alignof" "(" type-name ")"
unary-op ::= "&" | "*" | "+" | "-" | "~" | "!"
```
*/
static Expression *unary(void)
{
    Expression *node;

    if(consume_reserved("sizeof"))
    {
        // The type of the result of 'sizeof' operator is 'size_t'.
        // This implementation regards 'size_t' as 'unsigned long'.
        Token *saved_token = get_token();
        if(consume_reserved("("))
        {
            if(peek_type_name())
            {
                Type *type = type_name();
                node = new_node_constant(TY_ULONG, type->size);
                expect_reserved(")");
                goto unary_end;
            }
            else
            {
                set_token(saved_token);
            }
        }

        Expression *operand = unary();
        node = new_node_constant(TY_ULONG, operand->type->size);
    }
    else if(consume_reserved("_Alignof"))
    {
        expect_reserved("(");
        Type *type = type_name();
        node = new_node_constant(TY_ULONG, type->align);
        expect_reserved(")");
    }
    else if(consume_reserved("++"))
    {
        Expression *operand = unary();

        // check constraints
        if(!is_modifiable_lvalue(operand))
        {
            report_error(NULL, "expected modifiable lvalue");
        }
        if(is_real(operand->type))
        {
            node = new_node_binary(EXPR_ADD_EQ, operand, new_node_constant(TY_INT, 1));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(EXPR_PTR_ADD_EQ, operand, new_node_constant(TY_INT, 1));
        }
        else
        {
            report_error(NULL, "wrong type argument to increment");
        }
    }
    else if(consume_reserved("--"))
    {
        Expression *operand = unary();

        // check constraints
        if(!is_modifiable_lvalue(operand))
        {
            report_error(NULL, "expected modifiable lvalue");
        }
        if(is_integer(operand->type))
        {
            node = new_node_binary(EXPR_SUB_EQ, operand, new_node_constant(TY_INT, 1));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(EXPR_PTR_SUB_EQ, operand, new_node_constant(TY_INT, 1));
        }
        else
        {
            report_error(NULL, "wrong type argument to decrement");
        }
    }
    else if(consume_reserved("&"))
    {
        node = new_node_unary(EXPR_ADDR, unary());
    }
    else if (consume_reserved("*"))
    {
        Expression *operand = apply_implicit_conversion(unary());

        // check constraints
        if(is_pointer(operand->type))
        {
            node = new_node_unary(EXPR_DEREF, operand);
        }
        else
        {
            report_error(NULL, "wrong type argument to unary *");
        }
    }
    else if(consume_reserved("+"))
    {
        Expression *operand = unary();

        // check constraints
        if(is_arithmetic(operand->type))
        {
            operand = new_node_cast(apply_integer_promotion(operand->type), operand);
            node = new_node_unary(EXPR_PLUS, operand);
        }
        else
        {
            report_error(NULL, "wrong type argument to unary +");
        }
    }
    else if(consume_reserved("-"))
    {
        Expression *operand = unary();

        // check constraints
        if(is_arithmetic(operand->type))
        {
            operand = new_node_cast(apply_integer_promotion(operand->type), operand);
            node = new_node_unary(EXPR_MINUS, operand);
        }
        else
        {
            report_error(NULL, "wrong type argument to unary -");
        }
    }
    else if (consume_reserved("~"))
    {
        Expression *operand = unary();

        // check constraints
        if(is_integer(operand->type))
        {
            operand = new_node_cast(apply_integer_promotion(operand->type), operand);
            node = new_node_unary(EXPR_COMPL, operand);
        }
        else
        {
            report_error(NULL, "wrong type argument to unary ~");
        }
    }
    else if (consume_reserved("!"))
    {
        Expression *operand = unary();

        // check constraints
        if(is_scalar(operand->type))
        {
            node = new_node_unary(EXPR_NEG, operand);
        }
        else
        {
            report_error(NULL, "wrong type argument to unary !");
        }
    }
    else
    {
        node = postfix();
    }

unary_end:
    return node;
}


/*
make a cast expression
```
cast ::= ("(" type-name ")")? unary
```
*/
static Expression *cast(void)
{
    Expression *node;

    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_type_name())
        {
            Type *type = type_name();
            expect_reserved(")");
            if(!consume_reserved("{"))
            {
                // parsing cast operator, not compound-literal
                if(!(is_void(type) || is_scalar(type)))
                {
                    report_error(NULL, "expected void type or scalar type");
                }

                Expression *operand = apply_implicit_conversion(unary());
                if(!is_scalar(operand->type))
                {
                    report_error(NULL, "expected scalar type");
                }

                node = new_node_cast(type, operand);
                goto cast_end;
            }
        }
    }
    set_token(saved_token);

    node = unary();

cast_end:
    return node;
}


/*
make a multiplicative expression
```
multiplicative ::= cast ("*" cast | "/" cast | "%" cast)*
```
*/
static Expression *multiplicative(void)
{
    Expression *node = cast();

    // parse tokens while finding a cast expression
    while(true)
    {
        Expression *lhs, *rhs;
        ExpressionKind kind;
        char *operator = NULL;

        if(consume_reserved("*"))
        {
            lhs = node;
            rhs = cast();
            kind = EXPR_MUL;
            operator = "*";
        }
        else if(consume_reserved("/"))
        {
            lhs = node;
            rhs = cast();
            kind = EXPR_DIV;
            operator = "/";
        }
        else if(consume_reserved("%"))
        {
            lhs = node;
            rhs = cast();
            kind = EXPR_MOD;
            operator = "%";
        }
        else
        {
            return node;
        }

        // make a new node
        if(check_constraint_binary(kind, lhs->type, rhs->type))
        {
            // perform the usual arithmetic conversions
            Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
            lhs = new_node_cast(type, lhs);
            rhs = new_node_cast(type, rhs);
            node = new_node_binary(kind, lhs, rhs);
        }
        else
        {
            report_error(NULL, "invalid operands to binary %s", operator);
        }
    }
}


/*
make an additive expression
```
additive ::= multiplicative ("+" multiplicative | "-" multiplicative)*
```
*/
static Expression *additive(void)
{
    Expression *node = multiplicative();

    // parse tokens while finding a multiplicative expression
    while(true)
    {
        if(consume_reserved("+"))
        {
            Expression *lhs = apply_implicit_conversion(node);
            Expression *rhs = apply_implicit_conversion(multiplicative());

            if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
            {
                // perform the usual arithmetic conversions
                Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
                lhs = new_node_cast(type, lhs);
                rhs = new_node_cast(type, rhs);
                node = new_node_binary(EXPR_ADD, lhs, rhs);
            }
            else if(is_pointer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_PTR_ADD, lhs, rhs);
            }
            else if(is_integer(lhs->type) && is_pointer(rhs->type))
            {
                node = new_node_binary(EXPR_PTR_ADD, rhs, lhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary +");
            }
        }
        else if(consume_reserved("-"))
        {
            Expression *lhs = apply_implicit_conversion(node);
            Expression *rhs = apply_implicit_conversion(multiplicative());

            if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
            {
                // perform the usual arithmetic conversions
                Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
                lhs = new_node_cast(type, lhs);
                rhs = new_node_cast(type, rhs);
                node = new_node_binary(EXPR_SUB, lhs, rhs);
            }
            else if(is_pointer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_PTR_SUB, lhs, rhs);
            }
            else if(is_pointer(lhs->type) && is_pointer(rhs->type) &&
                    lhs->type->base->complete && rhs->type->base->complete &&
                    is_compatible(lhs->type->base, rhs->type->base))
            {
                node = new_node_binary(EXPR_PTR_DIFF, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary -");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a shift expression
```
shift ::=  additive ("<<" additive | ">>" additive)*
```
*/
static Expression *shift(void)
{
    Expression *node = additive();

    // parse tokens while finding an additive expression
    while(true)
    {
        Expression *lhs, *rhs;
        ExpressionKind kind;
        char *operator = NULL;

        if(consume_reserved("<<"))
        {
            lhs = node;
            rhs = additive();
            kind = EXPR_LSHIFT;
            operator = "<<";
        }
        else if(consume_reserved(">>"))
        {
            lhs = node;
            rhs = additive();
            kind = EXPR_RSHIFT;
            operator = ">>";
        }
        else
        {
            return node;
        }

        // make a new node
        if(check_constraint_binary(kind, lhs->type, rhs->type))
        {
            // perform the integer promotion on both operands
            lhs = new_node_cast(apply_integer_promotion(lhs->type), lhs);
            rhs = new_node_cast(apply_integer_promotion(rhs->type), rhs);
            node = new_node_binary(kind, lhs, rhs);
        }
        else
        {
            report_error(NULL, "invalid operands to binary %s", operator);
        }
    }
}


/*
make a relational expression
```
relational ::= shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
```
*/
static Expression *relational(void)
{
    Expression *node = shift();

    // parse tokens while finding a shift expression
    while(true)
    {
        Expression *lhs, *rhs;
        ExpressionKind kind;
        char *operator = NULL;

        if(consume_reserved("<"))
        {
            lhs = node;
            rhs = shift();
            kind = EXPR_L;
            operator = "<";
        }
        else if(consume_reserved("<="))
        {
            lhs = node;
            rhs = shift();
            kind = EXPR_LEQ;
            operator = "<=";
        }
        else if(consume_reserved(">"))
        {
            lhs = shift();
            rhs = node;
            kind = EXPR_L;
            operator = ">";
        }
        else if(consume_reserved(">="))
        {
            lhs = shift();
            rhs = node;
            kind = EXPR_LEQ;
            operator = ">=";
        }
        else
        {
            return node;
        }
        lhs = apply_implicit_conversion(lhs);
        rhs = apply_implicit_conversion(rhs);

        // check constraints
        if(is_real(lhs->type) && is_real(rhs->type))
        {
            // perform the usual arithmetic conversions
            Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
            lhs = new_node_cast(type, lhs);
            rhs = new_node_cast(type, rhs);
        }
        else if(is_pointer(lhs->type) && is_pointer(rhs->type) && is_compatible(lhs->type->base, rhs->type->base))
        {
            // do nothing
        }
        else
        {
            report_error(NULL, "invalid operands to binary %s", operator);
        }

        // make a new node
        node = new_node_binary(kind, lhs, rhs);
    }
}


/*
make an equality expression
```
equality ::= relational ("==" relational | "!=" relational)*
```
*/
static Expression *equality(void)
{
    Expression *node = relational();

    // parse tokens while finding a relational expression
    while(true)
    {
        Expression *lhs, *rhs;
        ExpressionKind kind;
        char *operator = NULL;

        if(consume_reserved("=="))
        {
            lhs = node;
            rhs = relational();
            kind = EXPR_EQ;
            operator = "==";
        }
        else if(consume_reserved("!="))
        {
            lhs = node;
            rhs = relational();
            kind = EXPR_NEQ;
            operator = "!=";
        }
        else
        {
            return node;
        }
        lhs = apply_implicit_conversion(lhs);
        rhs = apply_implicit_conversion(rhs);

        // check constraints
        bool valid = false;
        if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
        {
            // perform the usual arithmetic conversions
            Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
            lhs = new_node_cast(type, lhs);
            rhs = new_node_cast(type, rhs);
            valid = true;
        }
        else if((is_pointer(lhs->type) && is_pointer(rhs->type)))
        {
            if(is_compatible(lhs->type->base, rhs->type->base))
            {
                valid = true;
            }

            // If one operand is a pointer to an object type and the other is a pointer to a qualified or unqualified version of 'void', the former is converted to the type of the latter.
            else if(is_void(lhs->type->base))
            {
                rhs->type = new_type_pointer(new_type(TY_VOID, lhs->type->qual));
                valid = true;
            }
            else if(is_void(rhs->type->base))
            {
                lhs->type = new_type_pointer(new_type(TY_VOID, rhs->type->qual));
                valid = true;
            }
        }

        if(!valid)
        {
            report_error(NULL, "invalid operands to binary %s", operator);
        }

        // make a new node
        node = new_node_binary(kind, lhs, rhs);
    }
}


/*
make a bitwise AND expression
```
bitwise-and ::= equality (& equality)*
```
*/
static Expression *bitwise_and(void)
{
    Expression *node = equality();

    // parse tokens while finding a equality expression
    while(true)
    {
        if(consume_reserved("&"))
        {
            Expression *lhs = node;
            Expression *rhs = equality();

            // make a new node
            if(check_constraint_binary(EXPR_BIT_AND, lhs->type, rhs->type))
            {
                // perform the usual arithmetic conversions
                Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
                lhs = new_node_cast(type, lhs);
                rhs = new_node_cast(type, rhs);
                node = new_node_binary(EXPR_BIT_AND, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary &");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise exclusive OR expression
```
bitwise-xor ::= bitwise-and (^ bitwise-and)*
```
*/
static Expression *bitwise_xor(void)
{
    Expression *node = bitwise_and();

    // parse tokens while finding a bitwise AND expression
    while(true)
    {
        if(consume_reserved("^"))
        {
            Expression *lhs = node;
            Expression *rhs = bitwise_and();

            // make a new node
            if(check_constraint_binary(EXPR_BIT_XOR, lhs->type, rhs->type))
            {
                // perform the usual arithmetic conversions
                Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
                lhs = new_node_cast(type, lhs);
                rhs = new_node_cast(type, rhs);
                node = new_node_binary(EXPR_BIT_XOR, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary ^");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise inclusive OR expression
```
bitwise-or ::= bitwise-xor (| bitwise-xor)*
```
*/
static Expression *bitwise_or(void)
{
    Expression *node = bitwise_xor();

    // parse tokens while finding a bitwise exclusive OR expression
    while(true)
    {
        if(consume_reserved("|"))
        {
            Expression *lhs = node;
            Expression *rhs = bitwise_xor();

            // make a new node
            if(check_constraint_binary(EXPR_BIT_OR, lhs->type, rhs->type))
            {
                // perform the usual arithmetic conversions
                Type *type = apply_arithmetic_conversion(lhs->type, rhs->type);
                lhs = new_node_cast(type, lhs);
                rhs = new_node_cast(type, rhs);
                node = new_node_binary(EXPR_BIT_OR, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary |");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a logical AND expression
```
logical-and ::= bitwise-or (&& bitwise-or)*
```
*/
static Expression *logical_and(void)
{
    Expression *node = bitwise_or();

    // parse tokens while finding a bitwise inclusive OR expression
    while(true)
    {
        if(consume_reserved("&&"))
        {
            Expression *lhs = apply_implicit_conversion(node);
            Expression *rhs = apply_implicit_conversion(bitwise_or());

            // make a new node
            if(check_constraint_binary(EXPR_LOG_AND, lhs->type, rhs->type))
            {
                node = new_node_binary(EXPR_LOG_AND, lhs, rhs);
            }
            else
            {
                report_error(NULL, "expected scalar type");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a logical OR expression
```
logical-or ::= logical-and (|| logical-and)*
```
*/
static Expression *logical_or(void)
{
    Expression *node = logical_and();

    // parse tokens while finding a bitwise logical AND expression
    while(true)
    {
        if(consume_reserved("||"))
        {
            Expression *lhs = apply_implicit_conversion(node);
            Expression *rhs = apply_implicit_conversion(logical_and());

            // make a new node
            if(check_constraint_binary(EXPR_LOG_OR, lhs->type, rhs->type))
            {
                node = new_node_binary(EXPR_LOG_OR, lhs, rhs);
            }
            else
            {
                report_error(NULL, "expected scalar type");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a conditional expression
```
conditional ::= logical-or ("?" expression ":" conditional)?
```
*/
static Expression *conditional(void)
{
    Expression *node = logical_or();

    if(consume_reserved("?"))
    {
        // parse the first operand and check constraints on it
        Expression *operand = apply_implicit_conversion(node);
        if(!is_scalar(node->type))
        {
            report_error(NULL, "expected scalar type");
        }

        // parse the second and third operands, check constraints on them and determine the type of the conditional expression
        Expression *lhs = apply_implicit_conversion(expression());
        expect_reserved(":");
        Expression *rhs = apply_implicit_conversion(conditional());
        Type *type = NULL;
        bool valid = false;
        if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
        {
            // perform the usual arithmetic conversions
            type = apply_arithmetic_conversion(lhs->type, rhs->type);
            lhs = new_node_cast(type, lhs);
            rhs = new_node_cast(type, rhs);
            valid = true;
        }
        else if(((is_struct(lhs->type) && is_struct(rhs->type)) || (is_union(lhs->type) && is_union(rhs->type))) &&
                is_compatible(lhs->type, rhs->type))
        {
            type = lhs->type;
            valid = true;
        }
        else if(is_void(lhs->type) && is_void(rhs->type))
        {
            type = lhs->type;
            valid = true;
        }
        else if(is_pointer(lhs->type) && is_pointer(rhs->type))
        {
            if(   is_compatible(lhs->type, rhs->type)
               || (is_void(lhs->type->base) || is_void(rhs->type->base)))
            {
                type = copy_type(lhs->type, lhs->type->qual | rhs->type->qual);
                valid = true;
            }
        }

        if(!valid)
        {
            report_error(NULL, "type mismatch in conditional expression");
        }

        // make a new node
        Expression *ternary = new_expression(EXPR_COND, type);
        ternary->operand = operand;
        ternary->lhs = lhs;
        ternary->rhs = rhs;

        return ternary;
    }
    else
    {
        return node;
    }
}


/*
make an assignment expression
```
assign ::= conditional (assign-op assign)?
assign-op ::= "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|="
```
*/
Expression *assign(void)
{
    Expression *node = conditional();
    Expression *lhs, *rhs;
    ExpressionKind kind;
    char *operator = NULL;

    // parse assignment
    if(consume_reserved("="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_ASSIGN;
        operator = "=";
    }
    else if(consume_reserved("*="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_MUL_EQ;
        operator = "*=";
    }
    else if(consume_reserved("/="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_DIV_EQ;
        operator = "/=";
    }
    else if(consume_reserved("%="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_MOD_EQ;
        operator = "%=";
    }
    else if(consume_reserved("+="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = is_arithmetic(lhs->type) ? EXPR_ADD_EQ : EXPR_PTR_ADD_EQ;
        operator = "+=";
    }
    else if(consume_reserved("-="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = is_arithmetic(lhs->type) ? EXPR_SUB_EQ : EXPR_PTR_SUB_EQ;
        operator = "-=";
    }
    else if(consume_reserved("<<="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_LSHIFT_EQ;
        operator = "<<=";
    }
    else if(consume_reserved(">>="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_RSHIFT_EQ;
        operator = ">>=";
    }
    else if(consume_reserved("&="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_AND_EQ;
        operator = "&=";
    }
    else if(consume_reserved("^="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_XOR_EQ;
        operator = "^=";
    }
    else if(consume_reserved("|="))
    {
        lhs = node;
        rhs = apply_implicit_conversion(assign());
        kind = EXPR_OR_EQ;
        operator = "|=";
    }
    else
    {
        return node;
    }

    // make a new node
    if(is_modifiable_lvalue(lhs))
    {
        if(check_constraint_binary(kind, lhs->type, rhs->type))
        {
            node = new_node_binary(kind, lhs, rhs);
        }
        else
        {
            report_error(NULL, "invalid operands to %s", operator);
        }
    }
    else
    {
        report_error(NULL, "expected modifiable lvalue as left operand of assignment");
    }

    return node;
}


/*
make an expression
```
expression ::= assign ("," assign)*
```
*/
Expression *expression(void)
{
    Expression *node = assign();

    // parse tokens while finding an assignment expression
    while(true)
    {
        if(consume_reserved(","))
        {
            node = new_node_binary(EXPR_COMMA, node, assign());
        }
        else
        {
            return node;
        }
    }
}


/*
make a constant expression
```
const-expression ::= conditional
```
*/
long const_expression(void)
{
    return evaluate(conditional(), NULL);
}


/*
evaluate a node
*/
long evaluate(const Expression *expr, const Expression **base)
{
    switch(expr->kind)
    {
#define OPERAND (evaluate(expr->operand, base))
#define LHS     (evaluate(expr->lhs, base))
#define RHS     (evaluate(expr->rhs, base))
    case EXPR_CONST:
        return expr->value;

    case EXPR_VAR:
        *base = expr;
        return 0;

    case EXPR_GENERIC:
        return OPERAND;

    case EXPR_COMPOUND:
        return 0;

    case EXPR_ADDR:
        return OPERAND;

    case EXPR_DEREF:
        return OPERAND;

    case EXPR_MEMBER:
        return OPERAND + expr->member->offset;

    case EXPR_PLUS:
        return OPERAND;

    case EXPR_MINUS:
        return -OPERAND;

    case EXPR_COMPL:
        return ~OPERAND;

    case EXPR_NEG:
        return !OPERAND;

    case EXPR_CAST:
        return OPERAND;

    case EXPR_MUL:
        return LHS * RHS;
        
    case EXPR_DIV:
        return LHS / RHS;
        
    case EXPR_MOD:
        return LHS % RHS;
        
    case EXPR_ADD:
        return LHS + RHS;

    case EXPR_PTR_ADD:
        return LHS + expr->lhs->type->base->size * RHS;
        
    case EXPR_SUB:
        return LHS - RHS;

    case EXPR_PTR_SUB:
        return LHS - expr->lhs->type->base->size * RHS;
        
    case EXPR_LSHIFT:
        return LHS << RHS;
        
    case EXPR_RSHIFT:
        return LHS >> RHS;
        
    case EXPR_EQ:
        return (LHS == RHS);
        
    case EXPR_NEQ:
        return (LHS != RHS);
        
    case EXPR_L:
        return (LHS < RHS);
        
    case EXPR_LEQ:
        return (LHS <= RHS);
        
    case EXPR_BIT_AND:
        return (LHS & RHS);
        
    case EXPR_BIT_XOR:
        return (LHS ^ RHS);
        
    case EXPR_BIT_OR:
        return (LHS | RHS);
        
    case EXPR_LOG_AND:
        return (LHS && RHS);
        
    case EXPR_LOG_OR:
        return (LHS || RHS);

    case EXPR_COND:
        return OPERAND ? LHS : RHS;

    default:
        report_error(NULL, "cannot evaluate");
        break;
#undef OPERAND
#undef LHS
#undef RHS
    }

    return 0;
}


/*
search the result expression from generic association list
*/
static Expression *search_generic_assoc_list(const Expression *control, const List(GenericAssociation) *assoc_list)
{
    Expression *result = NULL;
    for_each_entry(GenericAssociation, cursor, assoc_list)
    {
        GenericAssociation *assoc = get_element(GenericAssociation)(cursor);
        if(assoc->type == NULL)
        {
            // default case
            if(result == NULL)
            {
                result = assoc->assign;
            }
        }
        else
        {
            if(is_compatible(control->type, assoc->type))
            {
                if(result == NULL)
                {
                    result = assoc->assign;
                }
                else
                {
                    report_error(NULL, "'_Generic' selector matches multiple associations");
                }
            }
        }
    }

    if(result == NULL)
    {
        report_error(NULL, "'_Generic' selector is not compatible with any association");
    }

    return result;
}


/*
apply integer promotion
* In this implementation, integer promotions convert '_Bool', 'char' and 'short' (regardless of sign) to 'int' because
    * The size of '_Bool' and 'char' are 1 byte.
    * The size of 'short' is 2 bytes.
    * The size of 'int' is 4 bytes.
    * Therefore, 'int' can represent '_Bool', 'char', 'signed char', 'unsigned char', 'signed short' and 'unsigned short'.
*/
static Type *apply_integer_promotion(Type *type)
{
    if((type->kind == TY_BOOL)
    || (type->kind == TY_CHAR)
    || (type->kind == TY_SCHAR)
    || (type->kind == TY_UCHAR)
    || (type->kind == TY_SHORT)
    || (type->kind == TY_USHORT))
    {
        return new_type(TY_INT, TQ_NONE);
    }
    else
    {
        return type;
    }
}


/*
apply usual arithmetic conversion
*/
static Type *apply_arithmetic_conversion(Type *lhs_type, Type *rhs_type)
{
    Type *type;

    // perform integer promotions on both operands at first
    lhs_type = apply_integer_promotion(lhs_type);
    rhs_type = apply_integer_promotion(rhs_type);

    if(lhs_type->kind == rhs_type->kind)
    {
        // If both operands have the same type, then no further conversion is needed.
        type = lhs_type;
    }
    else
    {
        if((is_signed(lhs_type) && is_signed(rhs_type)) || (is_unsigned(lhs_type) && is_unsigned(rhs_type)))
        {
            // Otherwise, if both operands have signed integer types or both have unsigned integer types, the operand with the type of lesser integer conversion rank is converted to the type of the operand with greater rank.
            if(lhs_type->kind < rhs_type->kind)
            {
                type = rhs_type;
            }
            else
            {
                type = lhs_type;
            }
        }
        else
        {
            // Otherwise, if the operand that has unsigned integer type has rank greater or equal to the rank of the type of the other operand, then the operand with signed integer type is converted to the type of the operand with unsigned integer type.
            if(is_unsigned(lhs_type) && is_signed(rhs_type) && (get_conversion_rank(lhs_type) >= get_conversion_rank(rhs_type)))
            {
                type = lhs_type;
            }
            else if(is_unsigned(rhs_type) && is_signed(lhs_type) && (get_conversion_rank(rhs_type) >= get_conversion_rank(lhs_type)))
            {
                type = rhs_type;
            }
            // Otherwise, if the type of the operand with signed integer type can represent all of the values of the type of the operand with unsigned integer type, then the operand with unsigned integer type is converted to the type of the operand with signed integer type.
            // In this implementation, this rule can be checked only by comparing integer conversion ranks of operands.
            else if(is_signed(lhs_type) && is_unsigned(rhs_type) && (get_conversion_rank(lhs_type) > get_conversion_rank(rhs_type)))
            {
                type = lhs_type;
            }
            else if(is_signed(rhs_type) && is_unsigned(lhs_type) && (get_conversion_rank(rhs_type) > get_conversion_rank(lhs_type)))
            {
                type = rhs_type;
            }
            // Otherwise, both operands are converted to the unsigned integer type corresponding to the type of the operand with signed integer type.
            else
            {
                if(is_signed(lhs_type))
                {
                    type = discard_sign(lhs_type);
                }
                else
                {
                    type = discard_sign(rhs_type);
                }
            }
        }
    }

    return type;
}


/*
apply implicit conversion on expression
* If the argument is an lvalue which does not have an array type, this function returns the expression after lvalue conversion.
* If the argument has an array type, this function returns the expression with an pointer type to the initial element of the array.
* If the argument has a function type, this function returns the expression with an pointer type to the function.
* Otherwise, this function returns the argument.
*/
static Expression *apply_implicit_conversion(Expression *expr)
{
    Expression *conv = expr;

    // implicitly convert lvalue
    if(expr->lvalue && (!is_array(expr->type)))
    {
        conv->type = copy_type(expr->type, TQ_NONE);
        conv->lvalue = false;
    }

    // implicitly convert array to pointer
    if(is_array(expr->type))
    {
        conv = new_node_unary(EXPR_ADDR, new_node_unary(EXPR_DEREF, expr));
    }
    // implicitly convert function to pointer
    else if(is_function(expr->type))
    {
        conv = new_node_unary(EXPR_ADDR, expr);
    }

    return conv;
}


/*
check constraints on binary operations
*/
static bool check_constraint_binary(ExpressionKind kind, const Type *lhs_type, const Type *rhs_type)
{
    bool valid = false;

    switch(kind)
    {
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MUL_EQ:
    case EXPR_DIV_EQ:
        valid = (is_arithmetic(lhs_type) && is_arithmetic(rhs_type));
        break;

    case EXPR_MOD:
    case EXPR_LSHIFT:
    case EXPR_RSHIFT:
    case EXPR_BIT_AND:
    case EXPR_BIT_XOR:
    case EXPR_BIT_OR:
    case EXPR_MOD_EQ:
    case EXPR_LSHIFT_EQ:
    case EXPR_RSHIFT_EQ:
    case EXPR_AND_EQ:
    case EXPR_XOR_EQ:
    case EXPR_OR_EQ:
        valid = (is_integer(lhs_type) && is_integer(rhs_type));
        break;

    case EXPR_LOG_AND:
    case EXPR_LOG_OR:
        valid = (is_scalar(lhs_type) && is_scalar(rhs_type));
        break;

    case EXPR_ASSIGN:
        valid = (
                    (is_arithmetic(lhs_type) && is_arithmetic(rhs_type))
                 || (is_struct_or_union(lhs_type) && is_compatible(lhs_type, rhs_type))
                 || (is_pointer(lhs_type) && is_pointer(rhs_type) && is_compatible(lhs_type->base, rhs_type->base) && ((lhs_type->base->qual & rhs_type->base->qual) == rhs_type->base->qual))
                 || (is_pointer(lhs_type) && is_pointer(rhs_type) && (!is_function(lhs_type)) && is_void(rhs_type->base) && ((lhs_type->base->qual & rhs_type->base->qual) == rhs_type->base->qual))
                 || (is_pointer(lhs_type) && is_pointer(rhs_type) && (!is_function(rhs_type)) && is_void(lhs_type->base) && ((lhs_type->base->qual & rhs_type->base->qual) == rhs_type->base->qual))
                 || (is_bool(lhs_type) && is_pointer(rhs_type))
                );
        break;

    case EXPR_ADD_EQ:
    case EXPR_SUB_EQ:
        valid = (is_arithmetic(lhs_type) && is_arithmetic(rhs_type));
        break;

    case EXPR_PTR_ADD_EQ:
    case EXPR_PTR_SUB_EQ:
        valid = (is_pointer(lhs_type) && lhs_type->base->complete && is_integer(rhs_type));
        break;

    default:
        break;
    }

    return valid;
}


/*
check if a given expression is a modifiable lvalue
*/
static bool is_modifiable_lvalue(const Expression *expr)
{
    if((!expr->lvalue) || is_array(expr->type) || (!expr->type->complete))
    {
        return false;
    }
    else
    {
        return !is_const_qualified(expr->type);
    }
}


/*
check if a given type is const-qualified
* For a structure or union type, this function recursively checks if its members are const-qualified.
*/
static bool is_const_qualified(const Type *type)
{
    if(is_struct_or_union(type))
    {
        for_each_entry(Member, cursor, type->members)
        {
            Member *member = get_element(Member)(cursor);
            if(is_const_qualified(member->type))
            {
                return true;
            }
        }
    }

    return ((type->qual & TQ_CONST) == TQ_CONST);
}


/*
make a new label for compound-literal
*/
static char *new_compound_literal_label(void)
{
    // A label for compound-literal is of the form "CL<number>", so the length of buffer should be more than 2 + 10.
    char *label = calloc(13, sizeof(char));

    sprintf(label, "CL%d", cl_number);
    cl_number++;

    return label;
}
