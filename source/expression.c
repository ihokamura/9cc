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


// function prototype
static Expression *new_node_unary(ExpressionKind kind, Expression *operand);
static Expression *primary(void);
static Expression *postfix(void);
static Expression *arg_expr_list(void);
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
static Expression *apply_integer_promotion(Expression *expr);
static void apply_arithmetic_conversion(Expression *lhs, Expression *rhs);
static Expression *apply_implicit_conversion(Expression *expr);
static bool check_constraint_binary(ExpressionKind kind, const Type *lhs_type, const Type *rhs_type);


/*
make a new expression
*/
Expression *new_expression(ExpressionKind kind)
{
    Expression *node = calloc(1, sizeof(Expression));
    node->next = NULL;
    node->kind = kind;
    node->lhs = NULL;
    node->rhs = NULL;
    node->type = NULL;
    node->value = 0;
    node->var = NULL;
    node->operand = NULL;
    node->args = NULL;

    return node;
}


/*
make a new node for constant
*/
Expression *new_node_constant(TypeKind kind, long value)
{
    Expression *node = new_expression(EXPR_CONST);
    node->type = new_type(kind, TQ_NONE);
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
    Expression *node = new_expression(EXPR_MEMBER);
    node->member = member;
    node->type = copy_type(member->type);
    node->type->qual = operand->type->qual;
    node->operand = operand;

    return node;
}


/*
make a new node for unary operations
*/
static Expression *new_node_unary(ExpressionKind kind, Expression *operand)
{
    Expression *node = new_expression(kind);
    node->operand = operand;

    switch(kind)
    {
    case EXPR_ADDR:
        node->type = new_type_pointer(operand->type);
        break;

    case EXPR_DEREF:
        // An array type is also converted to the element type.
        node->type = operand->type->base;
        break;

    case EXPR_POST_INC:
    case EXPR_POST_DEC:
    case EXPR_PLUS:
    case EXPR_MINUS:
    case EXPR_COMPL:
        node->type = operand->type;
        break;

    case EXPR_NEG:
    default:
        node->type = new_type(TY_INT, TQ_NONE);
        break;
    }

    return node;
}


/*
make a new node for binary operations
*/
Expression *new_node_binary(ExpressionKind kind, Expression *lhs, Expression *rhs)
{
    Expression *node = new_expression(kind);
    node->lhs = lhs;
    node->rhs = rhs;

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
        node->type = lhs->type;
        break;

    case EXPR_PTR_DIFF:
        // The type of the result of pointer difference is 'ptrdiff_t'.
        // This implementation regards 'ptrdiff_t' as 'long'.
        node->type = new_type(TY_LONG, TQ_NONE);
        break;

    case EXPR_COMMA:
        node->type = rhs->type;
        break;

    case EXPR_L:
    case EXPR_LEQ:
    case EXPR_EQ:
    case EXPR_NEQ:
    case EXPR_LOG_AND:
    case EXPR_LOG_OR:
    default:
        node->type = new_type(TY_INT, TQ_NONE);
        break;
    }

    return node;
}


/*
make a primary expression
```
primary ::= identifier
          | constant
          | string-literal
          | "(" expression ")"
```
*/
static Expression *primary(void)
{
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
                Expression *node = new_expression(EXPR_VAR);
                node->type = ident->var->type;
                node->var = ident->var;
                return node;
            }
            else if(ident->en != NULL)
            {
                // enumeration
                return new_node_constant(TY_INT, ident->en->value);
            }
        }

        if(peek_reserved("("))
        {
            // implicitly assume that the token denotes a function which returns int
            Expression *node = new_expression(EXPR_VAR);
            Type *type = new_type_function(new_type(TY_INT, TQ_NONE), new_type(TY_VOID, TQ_NONE));
            Variable *var = new_gvar(token, type, false);
            node->type = type;
            node->var = var;
#if(WARN_IMPLICIT_DECLARATION_OF_FUNCTION == ENABLED)
            report_warning(token->str, "implicit declaration of function '%s'\n", make_identifier(token));
#endif /* WARN_IMPLICIT_DECLARATION_OF_FUNCTION */
            return node;
        }

        report_error(token->str, "undefined identifier '%s'", make_identifier(token));
    }

    // string-literal
    if(consume_token(TK_STR, &token))
    {
        Expression *node = new_expression(EXPR_VAR);
        node->str = new_string(token);
        node->var = node->str->var;
        node->type = node->var->type;
        return node;
    }

    // constant
    token = expect_constant();
    return new_node_constant(token->type, token->value);
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
```
*/
static Expression *postfix(void)
{
    Expression *node = primary();

    // parse tokens while finding a postfix operator
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
                Expression *func_node = new_expression(EXPR_FUNC);
                func_node->operand = node;
                func_node->type = node->type->base->base; // dereference pointer and get type of return value
                // parse arguments
                if(!consume_reserved(")"))
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
            if(is_struct(node->type) || is_union(node->type))
            {
                Expression *struct_node = node;
                node = new_node_member(struct_node, find_member(token, struct_node->type));
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

            if(is_pointer(node->type) && (is_struct(node->type->base) || is_union(node->type->base)))
            {
                Expression *struct_node = new_node_unary(EXPR_DEREF, node);
                node = new_node_member(struct_node, find_member(token, struct_node->type));
            }
            else
            {
                report_error(NULL, "expected pointer to structure or union");
            }
        }
        else if(consume_reserved("++"))
        {
            // postfix increment
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
            return node;
        }
    }
}


/*
make an argument expression list
```
arg-expr-list ::= assign ("," assign)*
```
*/
static Expression *arg_expr_list(void)
{
    Expression *arg;
    Expression *cursor = NULL;

    arg = assign();
    arg->next = cursor;
    cursor = arg;

    // parse arguments
    while(consume_reserved(","))
    {
        // append the argument at the head in order to push arguments in reverse order when generating assembler code
        arg = assign();
        arg->next = cursor;
        cursor = arg;
    }

    return cursor;
}


/*
make an unary expression
```
unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | "sizeof" unary
        | "sizeof" "(" type-name ")"
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
    else if(consume_reserved("++"))
    {
        Expression *operand = unary();

        // check constraints
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
            operand = apply_integer_promotion(operand);
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
            operand = apply_integer_promotion(operand);
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
            operand = apply_integer_promotion(operand);
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
            if(!(is_void(type) || is_scalar(type)))
            {
                report_error(NULL, "expected void type or scalar type");
            }

            Expression *operand = apply_implicit_conversion(unary());
            if(!is_scalar(operand->type))
            {
                report_error(NULL, "expected scalar type");
            }

            node = new_node_unary(EXPR_CAST, operand);
            node->type = type;
            goto cast_end;
        }
        else
        {
            set_token(saved_token);
        }
    }

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
            apply_arithmetic_conversion(lhs, rhs);
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
                apply_arithmetic_conversion(lhs, rhs);
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
                apply_arithmetic_conversion(lhs, rhs);
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
            lhs = apply_integer_promotion(lhs);
            rhs = apply_integer_promotion(rhs);
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
            apply_arithmetic_conversion(lhs, rhs);
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
            apply_arithmetic_conversion(lhs, rhs);
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
                apply_arithmetic_conversion(lhs, rhs);
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
                apply_arithmetic_conversion(lhs, rhs);
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
                apply_arithmetic_conversion(lhs, rhs);
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
            apply_arithmetic_conversion(lhs, rhs);
            type = lhs->type;
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
            if(is_compatible(lhs->type, rhs->type))
            {
                type = copy_type(lhs->type);
                type->qual = (lhs->type->qual | rhs->type->qual);
                valid = true;
            }
            else if(is_void(lhs->type->base) || is_void(rhs->type->base))
            {
                type = copy_type(lhs->type);
                type->qual = (lhs->type->qual | rhs->type->qual);
                valid = true;
            }
        }

        if(!valid)
        {
            report_error(NULL, "type mismatch in conditional expression");
        }

        // make a new node
        Expression *ternary = new_expression(EXPR_COND);
        ternary->operand = operand;
        ternary->lhs = lhs;
        ternary->rhs = rhs;
        ternary->type = type;

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

    // parse assignment
    if(consume_reserved("="))
    {
        node = new_node_binary(EXPR_ASSIGN, node, assign());
    }
    else if(consume_reserved("*="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_MUL_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_MUL_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment *=");
        }
    }
    else if(consume_reserved("/="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_DIV_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_DIV_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment /=");
        }
    }
    else if(consume_reserved("%="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_MOD_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_MOD_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment %=");
        }
    }
    else if(consume_reserved("+="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
        {
            node = new_node_binary(EXPR_ADD_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type) && lhs->type->base->complete && is_integer(rhs->type))
        {
            node = new_node_binary(EXPR_PTR_ADD_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }
    }
    else if(consume_reserved("-="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
        {
            node = new_node_binary(EXPR_SUB_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type) && lhs->type->base->complete && is_integer(rhs->type))
        {
            node = new_node_binary(EXPR_PTR_SUB_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }
    }
    else if(consume_reserved("<<="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_LSHIFT_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_LSHIFT_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment <<=");
        }
    }
    else if(consume_reserved(">>="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_RSHIFT_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_RSHIFT_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment >>=");
        }
    }
    else if(consume_reserved("&="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_AND_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_AND_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment &=");
        }
    }
    else if(consume_reserved("^="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_XOR_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_XOR_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment ^=");
        }
    }
    else if(consume_reserved("|="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        // make a new node
        if(check_constraint_binary(EXPR_OR_EQ, lhs->type, rhs->type))
        {
            node = new_node_binary(EXPR_OR_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment |=");
        }
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
    return evaluate(conditional());
}


/*
evaluate a node
*/
long evaluate(Expression *expr)
{
    long result = 0;

    switch(expr->kind)
    {
    case EXPR_CONST:
        result = expr->value;
        break;

    case EXPR_PLUS:
        result = evaluate(expr->operand);
        break;

    case EXPR_MINUS:
        result = -evaluate(expr->operand);
        break;

    case EXPR_COMPL:
        result = ~evaluate(expr->operand);
        break;

    case EXPR_NEG:
        result = !evaluate(expr->operand);
        break;

    case EXPR_ADD:
        result = evaluate(expr->lhs) + evaluate(expr->rhs);
        break;

    case EXPR_SUB:
        result = evaluate(expr->lhs) - evaluate(expr->rhs);
        break;

    case EXPR_MUL:
        result = evaluate(expr->lhs) * evaluate(expr->rhs);
        break;

    case EXPR_DIV:
        result = evaluate(expr->lhs) / evaluate(expr->rhs);
        break;

    case EXPR_MOD:
        result = evaluate(expr->lhs) % evaluate(expr->rhs);
        break;

    case EXPR_LSHIFT:
        result = evaluate(expr->lhs) << evaluate(expr->rhs);
        break;

    case EXPR_RSHIFT:
        result = evaluate(expr->lhs) >> evaluate(expr->rhs);
        break;

    case EXPR_EQ:
        result = (evaluate(expr->lhs) == evaluate(expr->rhs));
        break;

    case EXPR_NEQ:
        result = (evaluate(expr->lhs) != evaluate(expr->rhs));
        break;

    case EXPR_L:
        result = (evaluate(expr->lhs) < evaluate(expr->rhs));
        break;

    case EXPR_LEQ:
        result = (evaluate(expr->lhs) <= evaluate(expr->rhs));
        break;

    case EXPR_BIT_AND:
        result = (evaluate(expr->lhs) & evaluate(expr->rhs));
        break;

    case EXPR_BIT_XOR:
        result = (evaluate(expr->lhs) ^ evaluate(expr->rhs));
        break;

    case EXPR_BIT_OR:
        result = (evaluate(expr->lhs) | evaluate(expr->rhs));
        break;

    case EXPR_LOG_AND:
        result = (evaluate(expr->lhs) && evaluate(expr->rhs));
        break;

    case EXPR_LOG_OR:
        result = (evaluate(expr->lhs) || evaluate(expr->rhs));
        break;

    default:
        report_error(NULL, "cannot evaluate");
        break;
    }

    return result;
}


/*
apply integer promotion
* In this implementation, integer promotions convert 'char' and 'short' (regardless of sign) to 'int' because
    * The size of 'char' is 1 byte.
    * The size of 'short' is 2 bytes.
    * The size of 'int' is 4 bytes.
    * Therefore, 'int' can represent 'char', 'unsigned char', 'short' and 'unsigned short'.
*/
static Expression *apply_integer_promotion(Expression *expr)
{
    if((expr->type->kind == TY_CHAR)
    || (expr->type->kind == TY_UCHAR)
    || (expr->type->kind == TY_SHORT)
    || (expr->type->kind == TY_USHORT))
    {
        expr->type = new_type(TY_INT, TQ_NONE);
    }

    return expr;
}


/*
apply usual arithmetic conversion
*/
static void apply_arithmetic_conversion(Expression *lhs, Expression *rhs)
{
    // perform integer promotions on both operands at first
    lhs = apply_integer_promotion(lhs);
    rhs = apply_integer_promotion(rhs);

    if(lhs->type->kind == rhs->type->kind)
    {
        // If both operands have the same type, then no further conversion is needed.
    }
    else
    {
        if((is_signed(lhs->type) && is_signed(rhs->type)) || (is_unsigned(lhs->type) && is_unsigned(rhs->type)))
        {
            // Otherwise, if both operands have signed integer types or both have unsigned integer types, the operand with the type of lesser integer conversion rank is converted to the type of the operand with greater rank.
            if(lhs->type->kind < rhs->type->kind)
            {
                lhs->type = rhs->type;
            }
            else
            {
                rhs->type = lhs->type;
            }
        }
        else
        {
            // Otherwise, if the operand that has unsigned integer type has rank greater or equal to the rank of the type of the other operand, then the operand with signed integer type is converted to the type of the operand with unsigned integer type.
            if(is_unsigned(lhs->type) && is_signed(rhs->type) && (get_conversion_rank(lhs->type) >= get_conversion_rank(rhs->type)))
            {
                rhs->type = lhs->type;
            }
            else if(is_unsigned(rhs->type) && is_signed(lhs->type) && (get_conversion_rank(rhs->type) >= get_conversion_rank(lhs->type)))
            {
                lhs->type = rhs->type;
            }
            // Otherwise, if the type of the operand with signed integer type can represent all of the values of the type of the operand with unsigned integer type, then the operand with unsigned integer type is converted to the type of the operand with signed integer type.
            // In this implementation, this rule can be checked only by comparing integer conversion ranks of operands.
            else if(is_signed(lhs->type) && is_unsigned(rhs->type) && (get_conversion_rank(lhs->type) > get_conversion_rank(rhs->type)))
            {
                rhs->type = lhs->type;
            }
            else if(is_signed(rhs->type) && is_unsigned(lhs->type) && (get_conversion_rank(rhs->type) > get_conversion_rank(lhs->type)))
            {
                lhs->type = rhs->type;
            }
            // Otherwise, both operands are converted to the unsigned integer type corresponding to the type of the operand with signed integer type.
            else
            {
                if(is_signed(lhs->type))
                {
                    lhs->type = discard_sign(lhs->type);
                    rhs->type = lhs->type;
                }
                else
                {
                    rhs->type = discard_sign(rhs->type);
                    lhs->type = rhs->type;
                }
            }
        }
    }
}


/*
apply implicit conversion on expression
* If the argument has an array type, this function returns the expression with an pointer type to the initial element of the array.
* If the argument has a function type, this function returns the expression with an pointer type to the function.
* Otherwise, this function returns the argument.
*/
static Expression *apply_implicit_conversion(Expression *expr)
{
    Expression *conv = expr;

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

    default:
        break;
    }

    return valid;
}
