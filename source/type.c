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
static Type void_types[] = {
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_NONE,                              false},
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_CONST,                             false},
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_RESTRICT,                          false},
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_CONST | TQ_RESTRICT,               false},
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_VOLATILE,                          false},
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_CONST | TQ_VOLATILE,               false},
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_RESTRICT | TQ_VOLATILE,            false},
    {TY_VOID, SIZEOF_VOID, ALIGNOF_VOID, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, false},
};
static Type char_types[] = {
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_NONE,                              true},
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST,                             true},
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_RESTRICT,                          true},
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_RESTRICT,               true},
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_VOLATILE,                          true},
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_VOLATILE,               true},
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_CHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type uchar_types[] = {
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_NONE,                              true},
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST,                             true},
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_RESTRICT,                          true},
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_RESTRICT,               true},
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_VOLATILE,                          true},
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_VOLATILE,               true},
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_UCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type short_types[] = {
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_NONE,                              true},
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST,                             true},
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_RESTRICT,                          true},
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST | TQ_RESTRICT,               true},
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_VOLATILE,                          true},
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST | TQ_VOLATILE,               true},
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_SHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type ushort_types[] = {
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_NONE,                              true},
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST,                             true},
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_RESTRICT,                          true},
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST | TQ_RESTRICT,               true},
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_VOLATILE,                          true},
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST | TQ_VOLATILE,               true},
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_USHORT, SIZEOF_SHORT, ALIGNOF_SHORT, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type int_types[] = {
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_NONE,                              true},
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST,                             true},
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_RESTRICT,                          true},
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST | TQ_RESTRICT,               true},
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_VOLATILE,                          true},
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST | TQ_VOLATILE,               true},
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_INT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type uint_types[] = {
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_NONE,                              true},
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST,                             true},
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_RESTRICT,                          true},
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST | TQ_RESTRICT,               true},
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_VOLATILE,                          true},
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST | TQ_VOLATILE,               true},
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_UINT, SIZEOF_INT, ALIGNOF_INT, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type long_types[] = {
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_NONE,                              true},
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST,                             true},
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_RESTRICT,                          true},
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST | TQ_RESTRICT,               true},
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_VOLATILE,                          true},
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST | TQ_VOLATILE,               true},
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_LONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type ulong_types[] = {
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_NONE,                              true},
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST,                             true},
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_RESTRICT,                          true},
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST | TQ_RESTRICT,               true},
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_VOLATILE,                          true},
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST | TQ_VOLATILE,               true},
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_ULONG, SIZEOF_LONG, ALIGNOF_LONG, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};


/*
make a new type
*/
Type *new_type(TypeKind kind, TypeQualifier qual)
{
    Type *type;

    switch(kind)
    {
    case TY_VOID:
        type = &void_types[qual];
        break;

    case TY_CHAR:
        type = &char_types[qual];
        break;

    case TY_UCHAR:
        type = &uchar_types[qual];
        break;

    case TY_SHORT:
        type = &short_types[qual];
        break;

    case TY_USHORT:
        type = &ushort_types[qual];
        break;

    case TY_INT:
        type = &int_types[qual];
        break;

    case TY_UINT:
        type = &uint_types[qual];
        break;

    case TY_LONG:
        type = &long_types[qual];
        break;

    case TY_ULONG:
        type = &ulong_types[qual];
        break;

    default:
        type = calloc(1, sizeof(Type));
        type->size = 0;
        type->align = 0;
        type->qual = qual;
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
        return &char_types[type->qual];

    case TY_SHORT:
    case TY_USHORT:
        return &short_types[type->qual];

    case TY_INT:
    case TY_UINT:
        return &int_types[type->qual];

    case TY_LONG:
    case TY_ULONG:
        return &long_types[type->qual];

    default:
        return &int_types[type->qual];
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
    Type *type = new_type(TY_ENUM, TQ_NONE);
    type->size = SIZEOF_ENUM;
    type->align = ALIGNOF_ENUM;

    return type;
}


/*
make a pointer type
*/
Type *new_type_pointer(Type *base)
{
    Type *type = new_type(TY_PTR, TQ_NONE);
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
    Type *type = new_type(TY_ARRAY, TQ_NONE);
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
    Type *type = new_type(TY_FUNC, TQ_NONE);
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
