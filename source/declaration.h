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
    SC_TYPEDEF,  // "typdef"
    SC_EXTERN,   // "extern"
    SC_STATIC,   // "static"
    SC_AUTO,     // "auto"
    SC_REGISTER, // "register"
};

// structure for contents in data segment
struct DataSegment
{
    const char *label;   // label (symbol)
    size_t size;         // size of data
    bool zero;           // flag indicating that data is zero or non-zero
    long value;          // value of data
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

DataSegment *new_string_data_segment(const char *label);
Statement *declaration(bool is_local);
Type *declaration_specifiers(StorageClassSpecifier *sclass);
Type *declarator(Type *type, Token **token, List(Variable) **arg_vars);
Type *type_name(void);
bool peek_declaration_specifiers(void);
bool peek_type_name(void);

#endif /* !__DECLARATION_H__ */
