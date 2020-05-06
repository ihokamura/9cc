/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include "9cc.h"

Expression *new_expression(ExpressionKind kind);
Expression *new_node_constant(TypeKind kind, long value);
Expression *new_node_subscript(Expression *base, size_t index);
Expression *new_node_member(Expression *expr, Member *member);
Expression *new_node_binary(ExpressionKind kind, Expression *lhs, Expression *rhs);
Expression *assign(void);
Expression *expression(void);
long const_expression(void);
long evaluate(Expression *expr);

#endif /* !__EXPRESSION_H__ */
