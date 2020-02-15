/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* type handler
*/

#include <stdlib.h>

#include "9cc.h"


// global variable
static const size_t SIZEOF_CHAR = 1; // size of char type
static const size_t SIZEOF_SHORT = 2; // size of short type
static const size_t SIZEOF_INT = 4; // size of int type
static const size_t SIZEOF_LONG = 8; // size of long type
static const size_t SIZEOF_PTR = 8; // size of pointer type


/*
make a new type
*/
Type *new_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(Type));
    type->kind = kind;
    type->base = NULL;
    type->len = 1;

    switch(kind)
    {
    case TY_CHAR:
        type->size = SIZEOF_CHAR;
        break;

    case TY_SHORT:
        type->size = SIZEOF_SHORT;
        break;

    case TY_INT:
        type->size = SIZEOF_INT;
        break;

    case TY_LONG:
        type->size = SIZEOF_LONG;
        break;

    default:
        type->size = SIZEOF_INT;
        break;
    }

    return type;
}


/*
check if a given type is an integer type
*/
bool is_integer(const Type *type)
{
    return (
           (type->kind == TY_CHAR)
        || (type->kind == TY_SHORT)
        || (type->kind == TY_INT)
        || (type->kind == TY_LONG)
        );
}


/*
check if a given type is a pointer type
*/
bool is_pointer(const Type *type)
{
    return (type->kind == TY_PTR);
}


/*
check if a given type is an array type
*/
bool is_array(const Type *type)
{
    return (type->kind == TY_ARRAY);
}


/*
check if a given type is a pointer type or an array type
*/
bool is_pointer_or_array(const Type *type)
{
    return (is_pointer(type) || is_array(type));
}


/*
make a pointer type
*/
Type *new_type_pointer(Type *base)
{
    Type *type = new_type(TY_PTR);
    type->size = SIZEOF_PTR;
    type->base = base;

    return type;
}


/*
make an array type
*/
Type *new_type_array(Type *base, size_t len)
{
    Type *type = new_type(TY_ARRAY);
    type->size = base->size * len;
    type->base = base;
    type->len = len;

    return type;
}