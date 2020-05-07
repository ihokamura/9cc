/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __PARSER_H__
#define __PARSER_H__

#include "9cc.h"

Variable *new_var(const char *name, Type *type, bool local);
Variable *new_gvar(const Token *token, Type *type, bool entity);
Variable *new_lvar(const Token *token, Type *type);
StringLiteral *new_string(const Token *token);
void construct(Program *prog);
Identifier *push_identifier_scope(const char *name);
Tag *push_tag_scope(const char *name);
Scope enter_scope(void);
void leave_scope(Scope scope);
int get_current_scope_depth(void);
Identifier *find_identifier(const Token *token);
Tag *find_tag(const Token *token);

#endif /* !__PARSER_H__ */
