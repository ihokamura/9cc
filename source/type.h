/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __TYPE_H__
#define __TYPE_H__

#include "9cc.h"

// kind of types
typedef enum TypeKind TypeKind;
enum TypeKind
{
    TY_VOID,    // void
    TY_CHAR,    // char
    TY_SCHAR,   // signed char
    TY_UCHAR,   // unsigned char
    TY_SHORT,   // short
    TY_USHORT,  // unsigned short
    TY_INT,     // int
    TY_UINT,    // unsigned int
    TY_LONG,    // long
    TY_ULONG,   // unsigned long
    TY_PTR,     // pointer
    TY_ARRAY,   // array
    TY_STRUCT,  // structure
    TY_UNION,   // union
    TY_ENUM,    // enumeration
    TY_TYPEDEF, // typedef name
    TY_FUNC,    // function
};

// kind of type qualifiers
typedef enum TypeQualifier TypeQualifier;
enum TypeQualifier
{
    TQ_NONE     = 0,      // no qualifier
    TQ_CONST    = 1 << 0, // const
    TQ_RESTRICT = 1 << 1, // restrict
    TQ_VOLATILE = 1 << 2, // volatile
};

// kind of parameter class
typedef enum ParameterClassKind ParameterClassKind;
enum ParameterClassKind
{
    PC_INTEGER,  // INTEGER
    PC_NO_CLASS, // NO_CLASS
    PC_MEMORY,   // MEMORY
};

// structure for member
struct Member
{
    Type *type;       // type of member
    const char *name; // name of member
    size_t offset;    // offset of member (only for TY_STRUCT, TY_UNION)
    int value;        // value of member (only for TY_ENUM)
};

// structure for type
struct Type
{
    TypeKind kind;         // kind of type
    size_t size;           // size of type
    size_t align;          // alignment of type
    TypeQualifier qual;    // qualification of type
    bool complete;         // flag indicating that the tyee is complete or incomplete
    Type *base;            // base type (only for TY_PTR, TY_ARRAY, TY_FUNC)
    size_t len;            // length of array (only for TY_ARRAY)
    List(Type) *args;      // type of arguments (only for TY_FUNC)
    Tag *tag;              // tag (only for TY_STRUCT, TY_UNION, TY_ENUM)
    List(Member) *members; // members (only for TY_STRUCT, TY_UNION, TY_ENUM)
};

Type *new_type(TypeKind kind, TypeQualifier qual);
ListEntry(Type) *new_type_list(Type *element);
Type *copy_type(const Type *type);
int get_conversion_rank(const Type *type);
Type *discard_sign(const Type *type);
ParameterClassKind get_parameter_class(const Type *type);
bool is_void(const Type *type);
bool is_signed(const Type *type);
bool is_unsigned(const Type *type);
bool is_integer(const Type *type);
bool is_real(const Type *type);
bool is_arithmetic(const Type *type);
bool is_pointer(const Type *type);
bool is_scalar(const Type *type);
bool is_array(const Type *type);
bool is_struct(const Type *type);
bool is_union(const Type *type);
bool is_struct_or_union(const Type *type);
bool is_function(const Type *type);
bool is_compatible(const Type *self, const Type *other);
Type *new_type_enum(void);
Type *new_type_pointer(Type *base);
Type *new_type_array(Type *base, size_t len);
Type *new_type_function(Type *base, List(Type) *args);
Member *new_member(const char *name, Type *type);
Member *find_member(const Token *token, const Type *type);
size_t adjust_alignment(size_t target, size_t alignment);

#endif /* !__TYPE_H__ */
