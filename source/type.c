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
#include <string.h>

#include "9cc.h"


// macro
#define SIZEOF_VOID     (1)
#define SIZEOF_CHAR     (1)
#define SIZEOF_SHORT    (2)
#define SIZEOF_INT      (4)
#define SIZEOF_LONG     (8)
#define SIZEOF_PTR      (8)
#define SIZEOF_ENUM     (SIZEOF_INT)

#define ALIGNOF_VOID     (1)
#define ALIGNOF_CHAR     (1)
#define ALIGNOF_SHORT    (2)
#define ALIGNOF_INT      (4)
#define ALIGNOF_LONG     (8)
#define ALIGNOF_PTR      (8)
#define ALIGNOF_ENUM     (ALIGNOF_INT)

#define RANK_CHAR     (1)
#define RANK_SHORT    (2)
#define RANK_INT      (3)
#define RANK_LONG     (4)


// global variable
static Type void_type   = {TY_VOID,   SIZEOF_VOID,  ALIGNOF_VOID,  false};
static Type char_type   = {TY_CHAR,   SIZEOF_CHAR,  ALIGNOF_CHAR,  true};
static Type uchar_type  = {TY_UCHAR,  SIZEOF_CHAR,  ALIGNOF_CHAR,  true};
static Type short_type  = {TY_SHORT,  SIZEOF_SHORT, ALIGNOF_SHORT, true};
static Type ushort_type = {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, true};
static Type int_type    = {TY_INT,    SIZEOF_INT,   ALIGNOF_INT,   true};
static Type uint_type   = {TY_UINT,   SIZEOF_INT,   ALIGNOF_INT,   true};
static Type long_type   = {TY_LONG,   SIZEOF_LONG,  ALIGNOF_LONG,  true};
static Type ulong_type  = {TY_ULONG,  SIZEOF_LONG,  ALIGNOF_LONG,  true};


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
        type->align = 0;
        type->complete = false;
        type->kind = kind;
        type->base = NULL;
        type->len = 0;
        type->args = NULL;
        type->next = NULL;
        type->member = NULL;
        break;
    }

    return type;
}


/*
get the integer conversion rank of an integer type
*/
int get_conversion_rank(const Type *type)
{
    switch(type->kind)
    {
    case TY_CHAR:
    case TY_UCHAR:
        return RANK_CHAR;

    case TY_SHORT:
    case TY_USHORT:
        return RANK_SHORT;

    case TY_INT:
    case TY_UINT:
        return RANK_INT;

    case TY_LONG:
    case TY_ULONG:
        return RANK_LONG;

    default:
        return RANK_INT;
    }
}


/*
discard sign of an integer type
*/
Type *discard_sign(const Type *type)
{
    switch(type->kind)
    {
    case TY_CHAR:
    case TY_UCHAR:
        return &char_type;

    case TY_SHORT:
    case TY_USHORT:
        return &short_type;

    case TY_INT:
    case TY_UINT:
        return &int_type;

    case TY_LONG:
    case TY_ULONG:
        return &long_type;

    default:
        return &int_type;
    }
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
check if a given type is a signed integer type
*/
bool is_signed(const Type *type)
{
    return (
           (type->kind == TY_CHAR)
        || (type->kind == TY_SHORT)
        || (type->kind == TY_INT)
        || (type->kind == TY_LONG)
        );
}


/*
check if a given type is an unsigned integer type
*/
bool is_unsigned(const Type *type)
{
    return (
           (type->kind == TY_UCHAR)
        || (type->kind == TY_USHORT)
        || (type->kind == TY_UINT)
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
make an enumeration type
*/
Type *new_type_enum(void)
{
    Type *type = new_type(TY_ENUM);
    type->size = SIZEOF_ENUM;
    type->align = ALIGNOF_ENUM;

    return type;
}


/*
make a pointer type
*/
Type *new_type_pointer(Type *base)
{
    Type *type = new_type(TY_PTR);
    type->size = SIZEOF_PTR;
    type->align = ALIGNOF_PTR;
    type->complete = true;
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
    type->align = base->align;
    type->complete = true;
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
    type->complete = true;
    type->base = base;
    type->args = args;

    return type;
}


/*
make a new member of structure
*/
Member *new_member(const Token *token, Type *type)
{
    Member *member = calloc(1, sizeof(Member));
    member->next = NULL;
    member->type = type;
    member->name = make_identifier(token);
    member->offset = 0;

    return member;
}


/*
find member of structure
*/
Member *find_member(const Token *token, const Type *type)
{
    for(Member *member = type->member; member != NULL; member = member->next)
    {
        if(strncmp(token->str, member->name, token->len) == 0)
        {
            return member;
        }
    }

    report_error(NULL, "cannot find a member \"%s\"", make_identifier(token));
    return NULL;
}
