/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __DECLARATION_H__
#define __DECLARATION_H__

#include "9cc.h"

DataSegment *new_data_segment(void);
Statement *declaration(bool is_local);
Type *declaration_specifiers(StorageClassSpecifier *sclass);
Type *declarator(Type *type, Token **token, List(Variable) **arg_vars);
Type *type_name(void);
bool peek_declaration_specifiers(void);
bool peek_type_name(void);

#endif /* !__DECLARATION_H__ */
