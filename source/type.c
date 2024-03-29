#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "type.h"

// definition of list operations
#include "list.h"
define_list_operations(Member)
define_list_operations(Type)

// macro
#define RANK_BOOL     (0)
#define RANK_CHAR     (1)
#define RANK_SHORT    (2)
#define RANK_INT      (3)
#define RANK_LONG     (4)
#define WIDTHOF_BOOL     (1)
#define WIDTHOF_CHAR     (8 * SIZEOF_CHAR)
#define WIDTHOF_SHORT    (8 * SIZEOF_SHORT)
#define WIDTHOF_INT      (8 * SIZEOF_INT)
#define WIDTHOF_LONG     (8 * SIZEOF_LONG)
#define WIDTHOF_ENUM     (WIDTHOF_INT)

// function prototype
static bool is_reserved_kind(TypeKind kind);

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
static Type bool_types[] = {
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_NONE,                              true},
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_CONST,                             true},
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_RESTRICT,                          true},
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_CONST | TQ_RESTRICT,               true},
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_VOLATILE,                          true},
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_CONST | TQ_VOLATILE,               true},
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_BOOL, SIZEOF_BOOL, ALIGNOF_BOOL, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
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
static Type schar_types[] = {
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_NONE,                              true},
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST,                             true},
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_RESTRICT,                          true},
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_RESTRICT,               true},
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_VOLATILE,                          true},
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_VOLATILE,               true},
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_SCHAR, SIZEOF_CHAR, ALIGNOF_CHAR, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
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
static Type float_types[] = {
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_NONE,                              true},
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_CONST,                             true},
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_RESTRICT,                          true},
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_CONST | TQ_RESTRICT,               true},
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_VOLATILE,                          true},
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_CONST | TQ_VOLATILE,               true},
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_FLOAT, SIZEOF_FLOAT, ALIGNOF_FLOAT, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type double_types[] = {
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_NONE,                              true},
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_CONST,                             true},
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_RESTRICT,                          true},
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_CONST | TQ_RESTRICT,               true},
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_VOLATILE,                          true},
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_CONST | TQ_VOLATILE,               true},
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_DOUBLE, SIZEOF_DOUBLE, ALIGNOF_DOUBLE, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type ldouble_types[] = {
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_NONE,                              true},
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_CONST,                             true},
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_RESTRICT,                          true},
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_CONST | TQ_RESTRICT,               true},
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_VOLATILE,                          true},
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_CONST | TQ_VOLATILE,               true},
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_RESTRICT | TQ_VOLATILE,            true},
    {TY_LDOUBLE, SIZEOF_LDOUBLE, ALIGNOF_LDOUBLE, TQ_CONST | TQ_RESTRICT | TQ_VOLATILE, true},
};
static Type *reserved_types[] = {
    void_types,
    bool_types,
    char_types,
    schar_types,
    uchar_types,
    short_types,
    ushort_types,
    int_types,
    uint_types,
    long_types,
    ulong_types,
    float_types,
    double_types,
    ldouble_types,
};


/*
make a new type
*/
Type *new_type(TypeKind kind, TypeQualifier qual)
{
    Type *type;

    if(is_reserved_kind(kind))
    {
        type = &reserved_types[kind][qual];
    }
    else
    {
        type = calloc(1, sizeof(Type));
        type->size = 0;
        type->align = 0;
        type->qual = qual;
        type->complete = false;
        type->kind = kind;
        type->base = NULL;
        type->len = 0;
        type->args = NULL;
        type->members = NULL;
        type->atomic = false;
        type->variadic = false;
    }

    return type;
}


/*
copy a type
*/
Type *copy_type(const Type *type, TypeQualifier qual)
{
    Type *copy;

    if(is_reserved_kind(type->kind))
    {
        copy = &reserved_types[type->kind][qual];
    }
    else
    {
        copy = calloc(1, sizeof(Type));
        *copy = *type;
        copy->qual = qual;
    }

    return copy;
}


/*
make an atomic type
*/
Type *make_atomic_type(const Type *type)
{
    Type *copy = calloc(1, sizeof(Type));
    *copy = *type;
    copy->atomic = true;

    return copy;
}


/*
get the integer conversion rank of an integer type
*/
int get_conversion_rank(const Type *type)
{
    switch(type->kind)
    {
    case TY_BOOL:
        return RANK_BOOL;

    case TY_CHAR:
    case TY_SCHAR:
    case TY_UCHAR:
        return RANK_CHAR;

    case TY_SHORT:
    case TY_USHORT:
        return RANK_SHORT;

    case TY_INT:
    case TY_UINT:
    case TY_ENUM:
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
    case TY_LONG:
    case TY_ULONG:
        return &long_types[type->qual];

    case TY_INT:
    case TY_UINT:
    default:
        return &int_types[type->qual];
    }
}


/*
get the width of a bit-field with a given type
* This function returns -1 if the member type is not applicable as a bit-field.
*/
long get_bitfield_width(const Type *type)
{
    switch(type->kind)
    {
    case TY_BOOL:
        return WIDTHOF_BOOL;

    case TY_CHAR:
    case TY_UCHAR:
        return WIDTHOF_CHAR;

    case TY_SHORT:
    case TY_USHORT:
        return WIDTHOF_SHORT;

    case TY_INT:
    case TY_UINT:
        return WIDTHOF_INT;

    case TY_LONG:
    case TY_ULONG:
        return WIDTHOF_LONG;

    case TY_ENUM:
        return WIDTHOF_ENUM;

    default:
        return -1;
    }
}


/*
get parameter class of a given type
*/
ParameterClassKind get_parameter_class(const Type *type)
{
    if(is_integer(type) || is_pointer(type))
    {
        return PC_INTEGER;
    }
    else if((type->kind == TY_FLOAT) || (type->kind == TY_DOUBLE))
    {
        return PC_SSE;
    }
    else if(type->size > 2 * 8)
    {
        return PC_MEMORY;
    }
    else
    {
        return PC_NO_CLASS;
    }
}


/*
check if a given type is a void type
*/
bool is_void(const Type *type)
{
    return (type->kind == TY_VOID);
}


/*
check if a given type is a bool type
*/
bool is_bool(const Type *type)
{
    return (type->kind == TY_BOOL);
}


/*
check if a given type is a signed integer type
*/
bool is_signed(const Type *type)
{
    return (
           (type->kind == TY_CHAR)
        || (type->kind == TY_SCHAR)
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
           (type->kind == TY_BOOL)
        || (type->kind == TY_UCHAR)
        || (type->kind == TY_USHORT)
        || (type->kind == TY_UINT)
        || (type->kind == TY_ULONG)
        );
}


/*
check if a given type is an enumerated type
*/
bool is_enumerated(const Type *type)
{
    return (type->kind == TY_ENUM);
}


/*
check if a given type is an integer type
*/
bool is_integer(const Type *type)
{
    return (type->kind == TY_CHAR) || is_signed(type) || is_unsigned(type) || is_enumerated(type);
}


/*
check if a given type is a real floating type
*/
bool is_real_floating(const Type *type)
{
    return (type->kind == TY_FLOAT) || (type->kind == TY_DOUBLE) || (type->kind == TY_LDOUBLE);
}


/*
check if a given type is a floating type
*/
bool is_floating(const Type *type)
{
    return is_real_floating(type);
}


/*
check if a given type is a real type
*/
bool is_real(const Type *type)
{
    return is_integer(type) || is_real_floating(type);
}


/*
check if a given type is an arithmetic type
*/
bool is_arithmetic(const Type *type)
{
    return is_integer(type) || is_floating(type);
}


/*
check if a given type is a pointer type
*/
bool is_pointer(const Type *type)
{
    return (type->kind == TY_PTR);
}


/*
check if a given type is a scalar type
*/
bool is_scalar(const Type *type)
{
    return is_arithmetic(type) || is_pointer(type);
}


/*
check if a given type is an array type
*/
bool is_array(const Type *type)
{
    return (type->kind == TY_ARRAY);
}


/*
check if a given type is a structure type
*/
bool is_struct(const Type *type)
{
    return (type->kind == TY_STRUCT);
}


/*
check if a given type is an union type
*/
bool is_union(const Type *type)
{
    return (type->kind == TY_UNION);
}


/*
check if a given type is a structure type or an union type
*/
bool is_struct_or_union(const Type *type)
{
    return is_struct(type) || is_union(type);
}


/*
check if a given type is a function type
*/
bool is_function(const Type *type)
{
    return (type->kind == TY_FUNC);
}


/*
check if given types are compatible
*/
bool is_compatible(const Type *self, const Type *other)
{
    // Both types are a structure type, an union type or an enumerated type.
    if((is_struct(self) && is_struct(other)) || (is_union(self) && is_union(other)) || (is_enumerated(self) && is_enumerated(other)))
    {
        // If one is declared with a tag, the other shall be declared with the same tag.
        if(self->tag != NULL)
        {
            return (other->tag != NULL) && (self->tag == other->tag);
        }
        if(other->tag != NULL)
        {
            return (self->tag != NULL) && (self->tag == other->tag);
        }

        ListEntry(Member) *cursor_self = get_first_entry(Member)(self->members);
        ListEntry(Member) *cursor_other = get_first_entry(Member)(other->members);
        while(!(end_iteration(Member)(self->members, cursor_self) || end_iteration(Member)(other->members, cursor_other)))
        {
            Member *member_self = get_element(Member)(cursor_self);
            Member *member_other = get_element(Member)(cursor_other);

            if(strcmp(member_self->name, member_other->name) != 0)
            {
                return false;
            }
            if(!is_compatible(member_self->type, member_other->type))
            {
                return false;
            }

            cursor_self = next_entry(Member, cursor_self);
            cursor_other = next_entry(Member, cursor_other);
        }

        return (end_iteration(Member)(self->members, cursor_self) && end_iteration(Member)(other->members, cursor_other));
    }

    // Only one of types is an enumerated type.
    // This implementation regards enumerated types compatible with 'unsigned int'.
    else if(is_enumerated(self))
    {
        return (other->kind == TY_UINT);
    }
    else if(is_enumerated(other))
    {
        return (self->kind == TY_UINT);
    }

    // Both types are a pointer type.
    else if(is_pointer(self) && is_pointer(other))
    {
        return (self->qual == other->qual) && is_compatible(self->base, other->base);
    }

    // Both types are an array type.
    else if(is_array(self) && is_array(other))
    {
        return is_compatible(self->base, other->base) && (self->len == other->len);
    }

    // Both types are a function type.
    else if(is_function(self) && is_function(other))
    {
        if(is_compatible(self->base, other->base))
        {
            ListEntry(Type) *cursor_self = get_first_entry(Type)(self->args);
            ListEntry(Type) *cursor_other = get_first_entry(Type)(other->args);
            while(!(end_iteration(Type)(self->args, cursor_self) || end_iteration(Type)(other->args, cursor_other)))
            {
                if(!is_compatible(cursor_self->element, cursor_other->element))
                {
                    return false;
                }

                cursor_self = next_entry(Type, cursor_self);
                cursor_other = next_entry(Type, cursor_other);
            }

            return (end_iteration(Type)(self->args, cursor_self) && end_iteration(Type)(other->args, cursor_other));
        }
        else
        {
            return false;
        }
    }

    // other cases
    else
    {
        return (self->kind == other->kind);
    }
}


/*
make a size_t type
* This implementation regards 'size_t' as 'unsigned long'.
*/
Type *new_type_size_t(TypeQualifier qual)
{
    return new_type(TY_ULONG, qual);
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
    type->complete = (len > 0);
    type->base = base;
    type->len = len;

    return type;
}


/*
make a function type
*/
Type *new_type_function(Type *base, List(Type) *args, bool variadic)
{
    Type *type = new_type(TY_FUNC, TQ_NONE);
    type->complete = true;
    type->base = base;
    type->args = args;
    type->variadic = variadic;

    return type;
}


/*
make a new member of structure
*/
Member *new_member(const char *name, Type *type)
{
    Member *member = calloc(1, sizeof(Member));
    member->type = type;
    member->name = name;
    member->offset = 0;
    member->width = 0;
    member->value = 0;
    member->bitfield = false;

    return member;
}


/*
find member of structure
*/
Member *find_member(const Token *token, const Type *type)
{
    for_each_entry(Member, cursor, type->members)
    {
        Member *member = get_element(Member)(cursor);
        if(strncmp(token->str, member->name, token->len) == 0)
        {
            return member;
        }
    }

    report_error(NULL, "cannot find a member \"%s\"", make_identifier(token));
    return NULL;
}

/*
adjust alignment
*/
size_t adjust_alignment(size_t target, size_t alignment)
{
    return (target + (alignment - 1)) & ~(alignment - 1);
}


/*
check if a given kind of type is reserved
*/
static bool is_reserved_kind(TypeKind kind)
{
    return (TY_VOID <= kind) && (kind <= TY_LDOUBLE);
}
