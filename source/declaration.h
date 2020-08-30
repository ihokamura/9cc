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
    SC_NONE,     // no specifier
    SC_TYPEDEF,  // "typdef"
    SC_EXTERN,   // "extern"
    SC_STATIC,   // "static"
    SC_AUTO,     // "auto"
    SC_REGISTER, // "register"
};

// structure for contents in data segment
struct InitializerMap
{
    const char *label;        // label (symbol)
    const Expression *assign; // assignment expression in initializer
    size_t size;              // size of data
    size_t offset;            // offset from base
    bool initialized;         // flag indicating that data is initialized or not
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
Type *declaration_specifiers(StorageClassSpecifier *sclass);
Type *declarator(Type *type, Token **token, List(Variable) **arg_vars);
Type *type_name(void);
bool peek_declaration_specifiers(void);
bool peek_type_name(void);

extern const char *STATIC_VARIABLE_PUNCTUATOR;

#endif /* !__DECLARATION_H__ */
