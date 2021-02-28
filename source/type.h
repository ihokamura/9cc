#ifndef TYPE_H
#define TYPE_H

#include "9cc.h"

// size of type
#define SIZEOF_VOID     (1)
#define SIZEOF_BOOL     (1)
#define SIZEOF_CHAR     (1)
#define SIZEOF_SHORT    (2)
#define SIZEOF_INT      (4)
#define SIZEOF_LONG     (8)
#define SIZEOF_PTR      (8)
#define SIZEOF_ENUM     (SIZEOF_INT)
#define SIZEOF_FLOAT    (4)
#define SIZEOF_DOUBLE   (8)
#define SIZEOF_LDOUBLE  (16)

// alignment of type
#define ALIGNOF_VOID     (1)
#define ALIGNOF_BOOL     (1)
#define ALIGNOF_CHAR     (1)
#define ALIGNOF_SHORT    (2)
#define ALIGNOF_INT      (4)
#define ALIGNOF_LONG     (8)
#define ALIGNOF_PTR      (8)
#define ALIGNOF_ENUM     (ALIGNOF_INT)
#define ALIGNOF_FLOAT    (4)
#define ALIGNOF_DOUBLE   (8)
#define ALIGNOF_LDOUBLE  (16)

// kind of types
typedef enum TypeKind TypeKind;
enum TypeKind
{
    TY_VOID,    // void
    TY_BOOL,    // bool
    TY_CHAR,    // char
    TY_SCHAR,   // signed char
    TY_UCHAR,   // unsigned char
    TY_SHORT,   // short
    TY_USHORT,  // unsigned short
    TY_INT,     // int
    TY_UINT,    // unsigned int
    TY_LONG,    // long
    TY_ULONG,   // unsigned long
    TY_FLOAT,   // float
    TY_DOUBLE,  // double
    TY_LDOUBLE, // long double
    TY_PTR,     // pointer
    TY_ARRAY,   // array
    TY_ATOMIC,  // atomic type
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
    TQ_ATOMIC   = 1 << 3, // _Atomic
};

// kind of parameter class
typedef enum ParameterClassKind ParameterClassKind;
enum ParameterClassKind
{
    PC_INTEGER,  // INTEGER
    PC_SSE,      // SSE
    PC_NO_CLASS, // NO_CLASS
    PC_MEMORY,   // MEMORY
};

// structure for member
struct Member
{
    Type *type;       // type of member
    const char *name; // name of member
    size_t offset;    // offset of member (only for TY_STRUCT, TY_UNION)
    size_t width;     // width of bit-field (only for TY_STRUCT, TY_UNION)
    int value;        // value of member (only for TY_ENUM)
    bool bitfield;    // flag indicating that the member is bit-field (only for TY_STRUCT, TY_UNION)
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
    bool atomic;           // flag indicating that the type is atomic
    bool variadic;         // flag indicating that arguments is variadic (only for TY_FUNC)
};

Type *new_type(TypeKind kind, TypeQualifier qual);
ListEntry(Type) *new_type_list(Type *element);
Type *copy_type(const Type *type, TypeQualifier qual);
Type *make_atomic_type(const Type *type);
int get_conversion_rank(const Type *type);
Type *discard_sign(const Type *type);
long get_bitfield_width(const Type *type);
ParameterClassKind get_parameter_class(const Type *type);
bool is_void(const Type *type);
bool is_bool(const Type *type);
bool is_signed(const Type *type);
bool is_unsigned(const Type *type);
bool is_integer(const Type *type);
bool is_real_floating(const Type *type);
bool is_floating(const Type *type);
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
Type *new_type_size_t(TypeQualifier qual);
Type *new_type_enum(void);
Type *new_type_pointer(Type *base);
Type *new_type_array(Type *base, size_t len);
Type *new_type_function(Type *base, List(Type) *args, bool variadic);
Member *new_member(const char *name, Type *type);
Member *find_member(const Token *token, const Type *type);
size_t adjust_alignment(size_t target, size_t alignment);

#endif /* !TYPE_H */
