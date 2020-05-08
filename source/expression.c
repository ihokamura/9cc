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
    node->type = member->type;
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
        node->type = operand->type;
        break;

    case EXPR_NEG:
        node->type = new_type(TY_INT, TQ_NONE);
        break;

    case EXPR_PLUS:
    case EXPR_MINUS:
    case EXPR_COMPL:
        node->type = operand->type;
        break;

    default:
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

    switch(kind)
    {
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    case EXPR_L:
    case EXPR_LEQ:
    case EXPR_BIT_AND:
    case EXPR_BIT_XOR:
    case EXPR_BIT_OR:
        apply_arithmetic_conversion(lhs, rhs);
        node->type = lhs->type;
        break;

    case EXPR_LSHIFT:
    case EXPR_RSHIFT:
        lhs = apply_integer_promotion(lhs);
        rhs = apply_integer_promotion(rhs);
        node->type = lhs->type;
        break;

    case EXPR_EQ:
    case EXPR_NEQ:
        if(is_integer(lhs->type) && is_integer(rhs->type))
        {
            apply_arithmetic_conversion(lhs, rhs);
        }
        node->type = new_type(TY_INT, TQ_NONE);
        break;

    case EXPR_PTR_ADD:
    case EXPR_PTR_SUB:
        node->type = lhs->type;
        break;

    case EXPR_PTR_DIFF:
        // The type of the result of pointer difference is 'ptrdiff_t'.
        // This implementation regards 'ptrdiff_t' as 'long'.
        node->type = new_type(TY_LONG, TQ_NONE);
        break;

    case EXPR_ASSIGN:
        if(is_array(rhs->type))
        {
            // implicitly convert array to pointer
            node->type = new_type_pointer(rhs->type->base);
        }
        else if(is_function(rhs->type))
        {
            // implicitly convert function to pointer
            node->type = new_type_pointer(rhs->type);
        }
        else
        {
            node->type = lhs->type;
        }
        break;

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

    case EXPR_COMMA:
        node->type = rhs->type;
        break;

    case EXPR_LOG_AND:
    case EXPR_LOG_OR:
    default:
        node->type = new_type(TY_INT, TQ_NONE);
        break;
    }

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

            // implicitly convert array to pointer
            if(is_array(node->type))
            {
                node = new_node_unary(EXPR_ADDR, new_node_unary(EXPR_DEREF, node));
            }
            if(is_array(index->type))
            {
                index = new_node_unary(EXPR_ADDR, new_node_unary(EXPR_DEREF, index));
            }

            if(is_pointer(node->type) && is_integer(index->type))
            {
                operand = new_node_binary(EXPR_PTR_ADD, node, index);
            }
            else if(is_integer(node->type) && is_pointer(index->type))
            {
                operand = new_node_binary(EXPR_PTR_ADD, index, node);
            }
            else
            {
                report_error(NULL, "bad operand for [] operator\n");
            }

            node = new_node_unary(EXPR_DEREF, operand);
            expect_reserved("]");
        }
        else if(consume_reserved("("))
        {
            // function call
            if(is_function(node->type))
            {
                // implicitly convert function to pointer
                node = new_node_unary(EXPR_ADDR, node);
            }

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
            Expression *struct_node = node;
            node = new_node_member(struct_node, find_member(token, struct_node->type));
        }
        else if(consume_reserved("->"))
        {
            // access to member (by pointer)
            Token *token = expect_identifier();
            Expression *struct_node = new_node_unary(EXPR_DEREF, node);
            node = new_node_member(struct_node, find_member(token, struct_node->type));
        }
        else if(consume_reserved("++"))
        {
            // postfix increment
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix increment operator ++\n");
            }
            node = new_node_unary(EXPR_POST_INC, node);
        }
        else if(consume_reserved("--"))
        {
            // postfix decrement
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix decrement operator --\n");
            }
            node = new_node_unary(EXPR_POST_DEC, node);
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

        if(is_integer(operand->type))
        {
            node = new_node_binary(EXPR_ADD_EQ, operand, new_node_constant(TY_INT, 1));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(EXPR_PTR_ADD_EQ, operand, new_node_constant(TY_INT, 1));
        }
        else
        {
            report_error(NULL, "bad operand for prefix increment operator ++");
        }
    }
    else if(consume_reserved("--"))
    {
        Expression *operand = unary();

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
            report_error(NULL, "bad operand for prefix decrement operator --");
        }
    }
    else if(consume_reserved("&"))
    {
        node = new_node_unary(EXPR_ADDR, unary());
    }
    else if (consume_reserved("*"))
    {
        Expression *operand = unary();

        // implicitly convert function to pointer
        if(is_function(operand->type))
        {
            operand = new_node_unary(EXPR_ADDR, operand);
        }

        node = new_node_unary(EXPR_DEREF, operand);
    }
    else if(consume_reserved("+"))
    {
        node = new_node_unary(EXPR_PLUS, apply_integer_promotion(unary()));
    }
    else if(consume_reserved("-"))
    {
        node = new_node_unary(EXPR_MINUS, apply_integer_promotion(unary()));
    }
    else if (consume_reserved("~"))
    {
        node = new_node_unary(EXPR_COMPL, apply_integer_promotion(unary()));
    }
    else if (consume_reserved("!"))
    {
        node = new_node_unary(EXPR_NEG, unary());
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

            Expression *operand = unary();
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
        if(consume_reserved("*"))
        {
            Expression *lhs = node;
            Expression *rhs = cast();
            if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
            {
                node = new_node_binary(EXPR_MUL, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary *");
            }
        }
        else if(consume_reserved("/"))
        {
            Expression *lhs = node;
            Expression *rhs = cast();
            if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
            {
                node = new_node_binary(EXPR_DIV, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary /");
            }
        }
        else if(consume_reserved("%"))
        {
            Expression *lhs = node;
            Expression *rhs = cast();
            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_MOD, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary %%");
            }
        }
        else
        {
            return node;
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
            Expression *lhs = node;
            Expression *rhs = multiplicative();

            // implicitly convert array to pointer
            if(is_array(lhs->type))
            {
                lhs = new_node_unary(EXPR_ADDR, new_node_unary(EXPR_DEREF, lhs));
            }
            if(is_array(rhs->type))
            {
                rhs = new_node_unary(EXPR_ADDR, new_node_unary(EXPR_DEREF, rhs));
            }

            if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
            {
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
            Expression *lhs = node;
            Expression *rhs = multiplicative();

            // implicitly convert array to pointer
            if(is_array(lhs->type))
            {
                lhs = new_node_unary(EXPR_ADDR, new_node_unary(EXPR_DEREF, lhs));
            }
            if(is_array(rhs->type))
            {
                rhs = new_node_unary(EXPR_ADDR, new_node_unary(EXPR_DEREF, rhs));
            }

            if(is_arithmetic(lhs->type) && is_arithmetic(rhs->type))
            {
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
        if(consume_reserved("<<"))
        {
            Expression *lhs = node;
            Expression *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_LSHIFT, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary <<");
            }
        }
        else if(consume_reserved(">>"))
        {
            Expression *lhs = node;
            Expression *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_RSHIFT, lhs, rhs);
            }
            else
            {
                report_error(NULL, "invalid operands to binary >>");
            }
        }
        else
        {
            return node;
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
        if(consume_reserved("<"))
        {
            node = new_node_binary(EXPR_L, node, shift());
        }
        else if(consume_reserved("<="))
        {
            node = new_node_binary(EXPR_LEQ, node, shift());
        }
        else if(consume_reserved(">"))
        {
            node = new_node_binary(EXPR_L, shift(), node);
        }
        else if(consume_reserved(">="))
        {
            node = new_node_binary(EXPR_LEQ, shift(), node);
        }
        else
        {
            return node;
        }
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
        if(consume_reserved("=="))
        {
            node = new_node_binary(EXPR_EQ, node, relational());
        }
        else if(consume_reserved("!="))
        {
            node = new_node_binary(EXPR_NEQ, node, relational());
        }
        else
        {
            return node;
        }
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
            node = new_node_binary(EXPR_BIT_AND, node, equality());
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
            node = new_node_binary(EXPR_BIT_XOR, node, bitwise_and());
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
            node = new_node_binary(EXPR_BIT_OR, node, bitwise_xor());
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
            node = new_node_binary(EXPR_LOG_AND, node, bitwise_or());
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
            node = new_node_binary(EXPR_LOG_OR, node, logical_and());
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
        Expression *ternary = new_expression(EXPR_COND);

        ternary->operand = node;
        ternary->lhs = expression();
        expect_reserved(":");
        ternary->rhs = conditional();

        // copy LHS and RHS since their types may not be modified
        Expression lhs = *ternary->lhs;
        Expression rhs = *ternary->rhs;
        if(is_integer(lhs.type) && is_integer(rhs.type))
        {
            apply_arithmetic_conversion(&lhs, &rhs);
        }
        ternary->type = lhs.type;

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

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment *=");
        }

        node = new_node_binary(EXPR_MUL_EQ, lhs, rhs);
    }
    else if(consume_reserved("/="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment /=");
        }

        node = new_node_binary(EXPR_DIV_EQ, lhs, rhs);
    }
    else if(consume_reserved("%="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment %=");
        }

        node = new_node_binary(EXPR_MOD_EQ, lhs, rhs);
    }
    else if(consume_reserved("+="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(EXPR_ADD_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
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

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(EXPR_SUB_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
        {
            node = new_node_binary(EXPR_PTR_SUB_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }
    }
    else if(consume_reserved("<<="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment <<=");
        }

        node = new_node_binary(EXPR_LSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved(">>="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment >>=");
        }

        node = new_node_binary(EXPR_RSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved("&="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment &=");
        }

        node = new_node_binary(EXPR_AND_EQ, lhs, rhs);
    }
    else if(consume_reserved("^="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment ^=");
        }

        node = new_node_binary(EXPR_XOR_EQ, lhs, rhs);
    }
    else if(consume_reserved("|="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment |=");
        }

        node = new_node_binary(EXPR_OR_EQ, lhs, rhs);
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
