/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __TYPE_H__
#define __TYPE_H__

#include "9cc.h"

Type *new_type(TypeKind kind, TypeQualifier qual);
int get_conversion_rank(const Type *type);
Type *discard_sign(const Type *type);
bool is_void(const Type *type);
bool is_signed(const Type *type);
bool is_unsigned(const Type *type);
bool is_integer(const Type *type);
bool is_arithmetic(const Type *type);
bool is_pointer(const Type *type);
bool is_scalar(const Type *type);
bool is_array(const Type *type);
bool is_struct(const Type *type);
bool is_union(const Type *type);
bool is_function(const Type *type);
bool is_compatible(const Type *self, const Type *other);
Type *new_type_enum(void);
Type *new_type_pointer(Type *base);
Type *new_type_array(Type *base, size_t len);
Type *new_type_function(Type *base, Type *args);
Member *new_member(const Token *token, Type *type);
Member *find_member(const Token *token, const Type *type);
size_t adjust_alignment(size_t target, size_t alignment);

#endif /* !__TYPE_H__ */
