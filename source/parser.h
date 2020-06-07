/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __PARSER_H__
#define __PARSER_H__

#include "9cc.h"

// structure for function
struct Function
{
    char *name;             // name of function
    Type *type;             // type of function
    List(Variable) *args;   // arguments
    List(Statement) *body;  // body of function definition
    List(Variable) *locals; // list of local variables (including arguments)
    size_t stack_size;      // size of stack in bytes
};

// structure for identifier
struct Identifier
{
    const char *name; // identifier
    Variable *var;    // variable
    Enumerator *en;   // enumerator
    Type *type_def;   // type definition
    int depth;        // depth of scope
};

// structure for program
struct Program
{
    List(Variable) *gvars; // list of global variables
    List(Function) *funcs; // list of functions
};

// structure for scope
struct Scope
{
    List(Identifier) *ident_list;      // list of ordinary identifiers visible in the current scope
    ListEntry(Identifier) *ident_head; // head of list of ordinary identifiers in the previous scope
    List(Tag) *tag_list;               // list of tags visible in the current scope
    ListEntry(Tag) *tag_head;          // head of list of tags in the previous scope
    int depth;                         // depth of the current scope
};

// structure for string-literal
struct StringLiteral
{
    Variable *var;       // global variable which points to string-literal
    const char *content; // array of characters consisting of string-literal
};

// structure for tag
struct Tag
{
    const char *name; // name of tag
    Type *type;       // type 
    int depth;        // depth of scope
};

// structure for variable
struct Variable
{
    const char *name;        // name of variable
    Type *type;              // type of variable
    Statement *init;         // initializer
    bool local;              // flag indicating that the variable is local or global
    size_t offset;           // offset from base pointer (rbp) (only for local variable)
    StringLiteral *str;      // information of string-literal (only for string-literal)
    bool entity;             // flag indicating that the variable has an entity in the current translation unit (only for global variable)
    List(DataSegment) *data; // contents of data segment (only for global variable)
};

Variable *new_var(const char *name, Type *type, bool local);
Variable *new_gvar(const char *name, Type *type, bool entity);
Variable *new_lvar(const char *name, Type *type);
StringLiteral *new_string(const Token *token);
void construct(Program *prog);
Identifier *push_identifier_scope(const char *name);
Tag *push_tag_scope(const char *name, Type *type);
Scope enter_scope(void);
void leave_scope(Scope scope);
int get_current_scope_depth(void);
Identifier *find_identifier(const Token *token);
Tag *find_tag(const Token *token);

extern const size_t STACK_ALIGNMENT;

#endif /* !__PARSER_H__ */
