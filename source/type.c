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


// macro
#define SIZEOF_VOID     (1)
#define SIZEOF_CHAR     (1)
#define SIZEOF_SHORT    (2)
#define SIZEOF_INT      (4)
#define SIZEOF_LONG     (8)
#define SIZEOF_PTR      (8)


// global variable
static Type void_type   = {TY_VOID,   SIZEOF_VOID};
static Type char_type   = {TY_CHAR,   SIZEOF_CHAR};
static Type uchar_type  = {TY_UCHAR,  SIZEOF_CHAR};
static Type short_type  = {TY_SHORT,  SIZEOF_SHORT};
static Type ushort_type = {TY_USHORT, SIZEOF_SHORT};
static Type int_type    = {TY_INT,    SIZEOF_INT};
static Type uint_type   = {TY_UINT,   SIZEOF_INT};
static Type long_type   = {TY_LONG,   SIZEOF_LONG};
static Type ulong_type  = {TY_ULONG,  SIZEOF_LONG};


/*
make a new type
*/
Type *new_type(TypeKind kind)
{
    Type *type;

    switch(kind)
    {
    case TY_VOID:
        type = &void_type;
        break;

    case TY_CHAR:
        type = &char_type;
        break;

    case TY_UCHAR:
        type = &uchar_type;
        break;

    case TY_SHORT:
        type = &short_type;
        break;

    case TY_USHORT:
        type = &ushort_type;
        break;

    case TY_INT:
        type = &int_type;
        break;

    case TY_UINT:
        type = &uint_type;
        break;

    case TY_LONG:
        type = &long_type;
        break;

    case TY_ULONG:
        type = &ulong_type;
        break;

    default:
        type = calloc(1, sizeof(Type));
        type->size = 0;
        type->kind = kind;
        type->base = NULL;
        type->len = 0;
        type->args = NULL;
        type->next = NULL;
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
        || (type->kind == TY_UCHAR)
        || (type->kind == TY_SHORT)
        || (type->kind == TY_USHORT)
        || (type->kind == TY_INT)
        || (type->kind == TY_UINT)
        || (type->kind == TY_LONG)
        || (type->kind == TY_ULONG)
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


/*
make a function type
*/
Type *new_type_function(Type *base, Type *args)
{
    Type *type = new_type(TY_FUNC);
    type->base = base;
    type->args = args;

    return type;
}
