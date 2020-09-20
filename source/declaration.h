/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __DECLARATION_H__
#define __DECLARATION_H__

#include "9cc.h"

// kind of storage class specifiers
typedef enum StorageClassSpecifier StorageClassSpecifier;
enum StorageClassSpecifier
{
    SC_NONE         = 0,      // no specifier
    SC_TYPEDEF      = 1 << 0, // "typdef"
    SC_EXTERN       = 1 << 1, // "extern"
    SC_STATIC       = 1 << 2, // "static"
    SC_THREAD_LOCAL = 1 << 3, // "_Thread_local"
    SC_AUTO         = 1 << 4, // "auto"
    SC_REGISTER     = 1 << 5, // "register"
};

// kind of function specifiers
typedef enum FunctionSpecifier FunctionSpecifier;
enum FunctionSpecifier
{
    FS_NONE     = 0,      // no specifier
    FS_INLINE   = 1 << 0, // "inline"
    FS_NORETURN = 1 << 1, // "_Noreturn"
};

// structure for contents in data segment
struct InitializerMap
{
    const char *label;        // label (symbol)
    const Expression *assign; // assignment expression in initializer
    size_t size;              // size of data
    size_t offset;            // offset from base
    bool initialized;         // flag indicating that data is initialized or not
    bool zero;                // flag indicating that data is zero or not
};

// structure for declaration
struct Declaration
{
    Variable *var; // declared variable
};

// structure for enumerator
struct Enumerator
{
    const char *name; // name of enumerator
    int value;        // value of enumerator
};

InitializerMap *new_string_initializer_map(const char *label, size_t offset);
Statement *declaration(bool local);
Type *declaration_specifiers(size_t *align, StorageClassSpecifier *sclass, FunctionSpecifier *fspec);
Type *declarator(Type *type, Token **token, List(Variable) **arg_vars);
Type *type_name(void);
Initializer *initializer(void);
List(InitializerMap) *make_initializer_map(Type *type, const Initializer *init);
bool peek_declaration_specifiers(void);
bool peek_type_name(void);

extern const char *STATIC_VARIABLE_PUNCTUATOR;

#endif /* !__DECLARATION_H__ */
