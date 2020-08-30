/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser for declaration
*/

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declaration.h"
#include "expression.h"
#include "parser.h"
#include "statement.h"
#include "tokenizer.h"
#include "type.h"

// definition of list operations
#include "list.h"
typedef struct Initializer Initializer;
typedef struct Designator Designator;
define_list(Initializer)
define_list(Designator)
define_list_operations(Declaration)
define_list_operations(InitializerMap)
define_list_operations(Initializer)
define_list_operations(Designator)

// macro
#define INVALID_DECLSPEC (-1) // invalid declaration specifier
#define TYPESPEC_SIZE ((size_t)12) // number of type specifiers
#define max(a, b) ((a) > (b) ? (a) : (b))

// kind of type specifiers
typedef enum TypeSpecifier TypeSpecifier;
enum TypeSpecifier
{
    TS_VOID,     // "void"
    TS_CHAR,     // "char"
    TS_SHORT,    // "short"
    TS_INT,      // "int"
    TS_LONG,     // "long"
    TS_SIGNED,   // "signed"
    TS_UNSIGNED, // "unsigned"
    TS_BOOL,     // "_Bool"
    TS_STRUCT,   // structure
    TS_UNION,    // union
    TS_ENUM,     // enumeration
    TS_TYPEDEF,  // typedef name
};

// structure for initializer
struct Initializer
{
    List(Initializer) *list; // initializer-list
    List(Designator) *desig; // designation
    Expression *assign;      // assignment expression
};

// structure for designator
struct Designator
{
    long index;         // index of array
    const Token *token; // member name of structure or union
};

// function prototype
static Declaration *new_declaration(Variable *var);
static Member *new_enumerator(const char *name, int value);
static Initializer *new_initializer(void);
static InitializerMap *new_initializer_map(const Expression *assign, size_t size, size_t offset);
static InitializerMap *new_uninitialized_map(size_t size, size_t offset);
static List(Declaration) *init_declarator_list(Type *type, StorageClassSpecifier sclass, bool local);
static Declaration *init_declarator(Type *type, StorageClassSpecifier sclass, bool local);
static StorageClassSpecifier storage_class_specifier(void);
static TypeSpecifier type_specifier(Type **type);
static Type *struct_or_union_specifier(void);
static List(Member) *struct_declaration_list(void);
static List(Member) *struct_declaration(void);
static Type *specifier_qualifier_list(void);
static List(Member) *struct_declarator_list(Type *type);
static Type *enum_specifier(void);
static List(Member) *enumerator_list(void);
static Member *enumerator(int val);
static TypeQualifier type_qualifier(void);
static Type *direct_declarator(Type *type, Token **token, List(Variable) **arg_vars);
static Type *pointer(Type *base);
static List(Type) *parameter_type_list(List(Variable) **arg_vars, bool *variadic);
static List(Type) *parameter_list(List(Variable) **arg_vars);
static Type *parameter_declaration(Variable **arg_var, bool omit_name);
static Type *abstract_declarator(Type *type);
static Type *direct_abstract_declarator(Type *type);
static Type *declarator_suffixes(Type *type, List(Variable) **arg_vars);
static Initializer *initializer(void);
static List(Initializer) *initializer_list(void);
static List(Designator) *designation(void);
static List(Designator) *designator_list(void);
static Designator *designator(void);
static Initializer *designation_and_initializer(void);
static size_t get_designation_offset(Type **type, const List(Designator) *designation);
static List(InitializerMap) *make_initializer_map(Type *type, const Initializer *init);
static List(InitializerMap) *make_initializer_map_sub(List(InitializerMap) *init_maps, Type *type, const Initializer *init, size_t offset);
static List(InitializerMap) *append_uninitialized_map(List(InitializerMap) *init_maps, size_t size, size_t offset);
static int compare_offset(const void *data1, const void *data2);
static Type *determine_type(const int *spec_list, Type *type, TypeQualifier qual);
static bool can_determine_type(const int *spec_list);
static bool is_string(const Expression *expr);
static bool peek_storage_class_specifier(void);
static bool peek_type_specifier(void);
static bool peek_reserved_type_specifier(void);
static bool peek_user_type_specifier(void);
static bool peek_type_qualifier(void);
static bool peek_declarator(void);
static bool peek_pointer(void);
static bool peek_typedef_name(void);
static bool peek_abstract_declarator(void);
static bool peek_direct_abstract_declarator(void);
static bool peek_declarator_suffix(void);
static bool peek_designator(void);
static char *add_block_scope_label(const char *name);


// global variable
const char *STATIC_VARIABLE_PUNCTUATOR = ".";
static const struct {int spec_list[TYPESPEC_SIZE]; TypeKind type_kind;} TYPE_SPECS_MAP[] = {
    // synonym of 'void'
    {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, TY_VOID},    // void
    // synonym of 'char'
    {{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, TY_CHAR},    // char
    // synonym of 'signed char'
    {{0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, TY_SCHAR},   // signed char
    // synonym of 'unsigned char'
    {{0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, TY_UCHAR},   // unsigned char
    // synonym of 'short'
    {{0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, TY_SHORT},   // short
    {{0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0}, TY_SHORT},   // signed short
    {{0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}, TY_SHORT},   // short int
    {{0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0}, TY_SHORT},   // signed short int
    // synonym of 'unsigned short'
    {{0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0}, TY_USHORT},  // unsigned short
    {{0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0}, TY_USHORT},  // unsigned short int
    // synonym of 'int'
    {{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, TY_INT},     // int
    {{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, TY_INT},     // signed
    {{0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0}, TY_INT},     // signed int
    // synonym of 'unsigned'
    {{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, TY_UINT},    // unsigned
    {{0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0}, TY_UINT},    // unsigned int
    // synonym of 'long'
    {{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, TY_LONG},    // long
    {{0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0}, TY_LONG},    // signed long
    {{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0}, TY_LONG},    // long int
    {{0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0}, TY_LONG},    // signed long int
    // synonym of 'unsigned long'
    {{0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0}, TY_ULONG},   // unsigned long
    {{0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0}, TY_ULONG},   // unsigned long int
    // synonym of 'long long', which is equivalent to 'long' in this implementation
    {{0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0}, TY_LONG},    // long long
    {{0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0}, TY_LONG},    // signed long long
    {{0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0}, TY_LONG},    // long long int
    {{0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0}, TY_LONG},    // signed long long int
    // synonym of 'unsigned long long', which is equivalent to 'unsigned long' in this implementation
    {{0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0}, TY_ULONG},   // unsigned long long
    {{0, 0, 0, 1, 2, 0, 1, 0, 0, 0, 0, 0}, TY_ULONG},   // unsigned long long int
    // synonym of '_Bool'
    {{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, TY_BOOL},    // _Bool
    // other type specifiers
    {{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, TY_STRUCT},  // structure
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, TY_UNION},   // union
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, TY_ENUM},    // enumeration
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, TY_TYPEDEF}, // typedef name
}; // map from list of specifiers to kind of type
static const size_t TYPE_SPECS_MAP_SIZE = sizeof(TYPE_SPECS_MAP) / sizeof(TYPE_SPECS_MAP[0]); // size of map from list of specifiers to kind of type


/*
make a new declaration
*/
static Declaration *new_declaration(Variable *var)
{
    Declaration *decl = calloc(1, sizeof(Declaration));
    decl->var = var;

    return decl;
}


/*
make a new enumerator
*/
static Member *new_enumerator(const char *name, int value)
{
    Enumerator *en = calloc(1, sizeof(Enumerator));
    en->name = name;
    en->value = value;
    push_identifier_scope(en->name)->en = en;

    Member *member = new_member(name, new_type(TY_INT, TQ_CONST));
    member->value = value;

    return member;
}

/*
make a new initializer
*/
static Initializer *new_initializer(void)
{
    Initializer *init = calloc(1, sizeof(Initializer));
    init->list = NULL;
    init->desig = NULL;
    init->assign = NULL;

    return init;
}


/*
make a new map from offset to initializer
*/
static InitializerMap *new_initializer_map(const Expression *assign, size_t size, size_t offset)
{
    InitializerMap *init_map = calloc(1, sizeof(InitializerMap));
    init_map->label = NULL;
    init_map->assign = assign;
    init_map->size = size;
    init_map->offset= offset;
    init_map->initialized = true;

    return init_map;
}


/*
make a new uninitialized map from offset to initializer
*/
static InitializerMap *new_uninitialized_map(size_t size, size_t offset)
{
    InitializerMap *init_map = new_initializer_map(NULL, size, offset);
    init_map->initialized = false;

    return init_map;
}


/*
make a new string data segment
*/
InitializerMap *new_string_initializer_map(const char *label, size_t offset)
{
    InitializerMap *init_map = new_initializer_map(NULL, SIZEOF_PTR, offset);
    init_map->label = label;

    return init_map;
}


/*
make a declaration
```
declaration ::= declaration-specifiers init-declarator-list? ";"
```
*/
Statement *declaration(bool local)
{
    // parse declaration specifier
    StorageClassSpecifier sclass;
    Type *type = declaration_specifiers(&sclass);

    // parse init-declarator-list
    Statement *stmt = new_statement(STMT_DECL);
    if(peek_declarator())
    {
        stmt->decl = init_declarator_list(type, sclass, local);
    }
    else
    {
        stmt->decl = new_list(Declaration)(); // make a dummy list
    }

    expect_reserved(";");

    return stmt;
}


/*
make a declaration specifier
```
declaration-specifiers ::= (storage-class-specifier | type-specifier | type-qualifier)*
```
* This function sets INVALID_DECLSPEC to the argument 'sclass' if no storage-class specifier is given.
*/
Type *declaration_specifiers(StorageClassSpecifier *sclass)
{
    int spec_list[TYPESPEC_SIZE] = {0};
    Type *type = NULL;
    TypeQualifier qual = TQ_NONE;
    *sclass = INVALID_DECLSPEC;

    // parse storage-class specifiers and type specifiers
    while(true)
    {
        if(peek_storage_class_specifier())
        {
            if(*sclass != INVALID_DECLSPEC)
            {
                report_error(NULL, "multiple storage classes in declaration specifiers");
            }
            *sclass = storage_class_specifier();
            continue;
        }

        if(peek_type_qualifier())
        {
            qual |= type_qualifier();
            continue;
        }

        if(!peek_reserved_type_specifier() && can_determine_type(spec_list))
        {
            break;
        }

        spec_list[type_specifier(&type)]++;
    }

    return determine_type(spec_list, type, qual);
}


/*
make a init-declarator-list
```
init-declarator-list ::= init-declarator ("," init-declarator)*
```
*/
static List(Declaration) *init_declarator_list(Type *type, StorageClassSpecifier sclass, bool local)
{
    List(Declaration) *list = new_list(Declaration)();
    add_list_entry_tail(Declaration)(list, init_declarator(type, sclass, local));

    while(consume_reserved(","))
    {
        add_list_entry_tail(Declaration)(list, init_declarator(type, sclass, local));
    }

    return list;
}


/*
make a init-declarator-list
```
init-declarator ::= declarator ("=" initializer)?
```
*/
static Declaration *init_declarator(Type *type, StorageClassSpecifier sclass, bool local)
{
    // parse declarator
    Token *token;
    type = declarator(type, &token, false);

    // check duplicated declaration
    Identifier *ident = find_identifier(token);
    if(ident != NULL)
    {
        bool duplicated = false;

        if(sclass == SC_EXTERN)
        {
            // return the prior declaration of identifier with external linkage
            return new_declaration(ident->var);
        }

        if(!local && (sclass == SC_STATIC))
        {
            if(consume_reserved("="))
            {
                if(get_first_element(InitializerMap)(ident->var->inits)->initialized)
                {
                    duplicated = true;
                    goto report_duplicated_declaration;
                }
                // initialize the prior declaration of identifier with internal linkage
                ident->var->inits = make_initializer_map(ident->var->type, initializer());
            }
            // return the prior declaration of identifier with internal linkage
            return new_declaration(ident->var);
        }

        duplicated = (local && (ident->depth == get_current_scope_depth()));

report_duplicated_declaration:
        if(duplicated)
        {
            report_error(token->str, "duplicated declaration of '%s'\n", ident->name);
        }
    }

    char *name = make_identifier(token);
    if(sclass == SC_TYPEDEF)
    {
        // make a dummy declaration
        push_identifier_scope(name)->type_def = type;
        return new_declaration(NULL);
    }
    else
    {
        // make a new node for variable
        Expression *expr = new_expression(EXPR_VAR, type);

        if(local && !((sclass == SC_EXTERN) || (sclass == SC_STATIC)))
        {
            // block scope identifier for an object with automatic storage duration
            expr->var = new_lvar(name, type);
        }
        else
        {
            if(local && (sclass == SC_STATIC))
            {
                // block scope identifier for an object with static storage duration
                name = add_block_scope_label(name);
            }
            bool emit = (type->kind != TY_FUNC) && (sclass != SC_EXTERN);
            expr->var = new_gvar(name, type, sclass, emit);
        }

        // parse initializer
        Initializer *init = consume_reserved("=") ? initializer() : NULL;
        expr->var->inits = make_initializer_map(expr->var->type, init);

        return new_declaration(expr->var);
    }
}


/*
make a storage-class specifier
```
storage-class-specifier ::= "typedef"
                          | "extern"
                          | "static"
                          | "auto"
                          | "register"
```
*/
static StorageClassSpecifier storage_class_specifier(void)
{
    if(consume_reserved("typedef"))
    {
        return SC_TYPEDEF;
    }
    else if(consume_reserved("extern"))
    {
        return SC_EXTERN;
    }
    else if(consume_reserved("static"))
    {
        return SC_STATIC;
    }
    else if(consume_reserved("auto"))
    {
        return SC_AUTO;
    }
    else if(consume_reserved("register"))
    {
        return SC_REGISTER;
    }
    else
    {
        report_error(NULL, "invalid storage-class specifier\n");
        return INVALID_DECLSPEC;
    }
}


/*
make a type specifier
```
type-specifier ::= "void"
                 | "char"
                 | "short"
                 | "int"
                 | "long"
                 | "signed"
                 | "unsigned"
                 | "_Bool"
                 | struct-or-union-specifier
                 | enum-specifier
                 | typedef-name
```
*/
static TypeSpecifier type_specifier(Type **type)
{
    if(consume_reserved("void"))
    {
        return TS_VOID;
    }
    else if(consume_reserved("char"))
    {
        return TS_CHAR;
    }
    else if(consume_reserved("short"))
    {
        return TS_SHORT;
    }
    else if(consume_reserved("int"))
    {
        return TS_INT;
    }
    else if(consume_reserved("long"))
    {
        return TS_LONG;
    }
    else if(consume_reserved("signed"))
    {
        return TS_SIGNED;
    }
    else if(consume_reserved("unsigned"))
    {
        return TS_UNSIGNED;
    }
    else if(consume_reserved("_Bool"))
    {
        return TS_BOOL;
    }
    else if(peek_reserved("struct"))
    {
        *type = struct_or_union_specifier();
        return TS_STRUCT;
    }
    else if(peek_reserved("union"))
    {
        *type = struct_or_union_specifier();
        return TS_UNION;
    }
    else if(peek_reserved("enum"))
    {
        *type = enum_specifier();
        return TS_ENUM;
    }
    else
    {
        Token *token = expect_identifier();
        Identifier *ident = find_identifier(token);
        if((ident != NULL) && (ident->type_def != NULL))
        {
            *type = ident->type_def;
            return TS_TYPEDEF;
        }
        else
        {
            report_error(token->str, "invalid type specifier\n");
            return INVALID_DECLSPEC;
        }
    }
}


/*
make a struct-or-union-specifier
```
struct-or-union-specifier ::= ("struct" | "union") identifier? "{" struct-declaration-list "}"
                            | ("struct" | "union") identifier
```
*/
static Type *struct_or_union_specifier(void)
{
    // parse "struct" or "union"
    TypeSpecifier spec_kind;
    TypeKind type_kind;
    Type *type = NULL;
    if(consume_reserved("struct"))
    {
        spec_kind = TS_STRUCT;
        type_kind = TY_STRUCT;
    }
    else if(consume_reserved("union"))
    {
        spec_kind = TS_UNION;
        type_kind = TY_UNION;
    }
    else
    {
        report_error(NULL, "expected 'struct' or 'union'");
    }

    // parse tag
    bool parse_content = true;
    char *name = NULL;
    Tag *tag = NULL;
    Token *token = NULL;
    if(consume_token(TK_IDENT, &token))
    {
        name = make_identifier(token);
        tag = find_tag(token);

        if((tag != NULL) && (tag->type->kind != type_kind) && (tag->depth == get_current_scope_depth()))
        {
            report_error(token->str, "'%s' defined as wrong kind of tag", name);
        }

        if(tag == NULL)
        {
            // declaration of a new tag
            type = new_type(type_kind, TQ_NONE);
            type->tag = push_tag_scope(name, type);
        }
        else
        {
            if(tag->type->kind == type_kind)
            {
                // save the tagged type
                type = tag->type;
            }
        }

        parse_content = consume_reserved("{");
        if(parse_content)
        {
            if((tag != NULL) && (tag->depth < get_current_scope_depth()))
            {
                // make a new type since this is a declaration of a new tag in the current scope
                type = new_type(type_kind, TQ_NONE);
                type->tag = push_tag_scope(name, type);
            }

            if(type->complete)
            {
                report_error(token->str, "redefinition of tag '%s %s'", (type_kind == TY_STRUCT ? "struct" : "union"), name);
            }
        }
    }
    else
    {
        // structure content or union content is required if there is no tag
        expect_reserved("{");
        type = new_type(type_kind, TQ_NONE);
    }

    if(parse_content)
    {
        // parse structure content or union content
        type->members = struct_declaration_list();

        if(spec_kind == TS_STRUCT)
        {
            // set offset of members and determine size and alignment of the structure type
            size_t offset = 0;
            size_t alignment = 0;
            for_each_entry(Member, cursor, type->members)
            {
                Member *member = get_element(Member)(cursor);
                member->offset = adjust_alignment(offset, member->type->align);
                offset = member->offset + member->type->size;
                if(alignment < member->type->align)
                {
                    alignment = member->type->align;
                }
            }
            type->size = adjust_alignment(offset, alignment);
            type->align = alignment;
        }
        else if(spec_kind == TS_UNION)
        {
            // determine size and alignment of the union type
            size_t size = 0;
            size_t alignment = 0;
            for_each_entry(Member, cursor, type->members)
            {
                Member *member = get_element(Member)(cursor);
                member->offset = 0; // offset is always 0
                if(size < member->type->size)
                {
                    size = member->type->size;
                }
                if(alignment < member->type->align)
                {
                    alignment = member->type->align;
                }
            }
            type->size = adjust_alignment(size, alignment);
            type->align = alignment;
        }

        expect_reserved("}");
        type->complete = true;
    }

    return type;
}


/*
make a struct-declaration-list
```
struct-declaration-list ::= struct-declaration struct-declaration*
```
*/
static List(Member) *struct_declaration_list(void)
{
    List(Member) *list = struct_declaration();
    while(!peek_reserved("}"))
    {
        list = concatenate_list(Member)(list, struct_declaration());
    }

    return list;
}


/*
make a struct-declaration
```
struct-declaration ::= specifier-qualifier-list (struct-declarator-list)? ";"
```
*/
static List(Member) *struct_declaration(void)
{
    Type *type = specifier_qualifier_list();
    List(Member) *members = NULL;
    if(peek_declarator())
    {
        members = struct_declarator_list(type);
    }
    else
    {
        // anonymous structure or union
        if(is_struct_or_union(type))
        {
            members = type->members;
        }
        else
        {
            report_error(NULL, "expected structure or union type");
        }
    }
    expect_reserved(";");

    return members;
}


/*
make a specifier-qualifier-list
```
specifier-qualifier-list ::= (type-specifier | type-qualifier)*
```
*/
static Type *specifier_qualifier_list(void)
{
    int spec_list[TYPESPEC_SIZE] = {0};
    Type *type = NULL;
    TypeQualifier qual = TQ_NONE;

    // parse type specifiers
    while(true)
    {
        if(peek_type_qualifier())
        {
            qual |= type_qualifier();
            continue;
        }

        if(!peek_reserved_type_specifier() && can_determine_type(spec_list))
        {
            break;
        }

        spec_list[type_specifier(&type)]++;
    }

    return determine_type(spec_list, type, qual);
}


/*
make a struct-declarator-list
```
struct-declarator-list ::= declarator ("," declarator)*
```
*/
static List(Member) *struct_declarator_list(Type *type)
{
    Token *token;
    Type *decl_type = declarator(type, &token, false);
    List(Member) *members = new_list(Member)();
    add_list_entry_tail(Member)(members, new_member(make_identifier(token), decl_type));
    while(consume_reserved(","))
    {
        decl_type = declarator(type, &token, false);
        add_list_entry_tail(Member)(members, new_member(make_identifier(token), decl_type));
    }

    return members;
}


/*
make an enum-specifier
```
enum-specifier ::= "enum" identifier? "{" enumerator-list (",")? "}"
                 | "enum" identifier
```
*/
static Type *enum_specifier(void)
{
    // parse "enum"
    Type *type = NULL;
    expect_reserved("enum");

    // parse tag
    bool parse_content = true;
    char *name = NULL;
    Tag *tag = NULL;
    Token *token = NULL;
    if(consume_token(TK_IDENT, &token))
    {
        name = make_identifier(token);
        tag = find_tag(token);

        if((tag != NULL) && (tag->type->kind != TY_ENUM) && (tag->depth == get_current_scope_depth()))
        {
            report_error(token->str, "'%s' defined as wrong kind of tag", name);
        }

        if(tag == NULL)
        {
            // declaration of a new tag
            type = new_type_enum();
            type->tag = push_tag_scope(name, type);
        }
        else
        {
            if(tag->type->kind == TY_ENUM)
            {
                // save the tagged type
                type = tag->type;
            }
        }

        parse_content = consume_reserved("{");
        if(parse_content)
        {
            if((tag != NULL) && (tag->depth < get_current_scope_depth()))
            {
                // make a new type since this is a declaration of a new tag in the current scope
                type = new_type_enum();
                type->tag = push_tag_scope(name, type);
            }

            if(type->complete)
            {
                report_error(token->str, "redefinition of tag 'enum %s'", name);
            }
        }
    }
    else
    {
        // enumeration content is required if there is no tag
        expect_reserved("{");
        type = new_type_enum();
    }

    if(parse_content)
    {
        // parse enumeration content
        type->members = enumerator_list();
        consume_reserved(","); //consume the trailing "," if it exists
        expect_reserved("}");
        type->complete = true;
    }

    return type;
}


/*
make an enumerator-list
```
enumerator-list ::= enumerator ("," enumerator)*
```
*/
static List(Member) *enumerator_list(void)
{
    Token *token;
    List(Member) *list = new_list(Member)();

    ListEntry(Member) *cursor = add_list_entry_tail(Member)(list, enumerator(0));
    while(consume_reserved(",") && peek_token(TK_IDENT, &token))
    {
        Member *member = get_element(Member)(cursor);
        cursor = add_list_entry_tail(Member)(list, enumerator(member->value + 1));
    }

    return list;
}


/*
make an enumerator
```
enumerator ::= identifier ("=" const-expression)?
```
*/
static Member *enumerator(int value)
{
    Token *token = expect_identifier();
    if(consume_reserved("="))
    {
        value = const_expression(); // overwrite the default value
    }

    return new_enumerator(make_identifier(token), value);
}


/*
make a type qualifier
```
type-qualifier ::= "const"
                 | "restrict"
                 | "volatile"
```
*/
static TypeQualifier type_qualifier(void)
{
    if(consume_reserved("const"))
    {
        return TQ_CONST;
    }
    else if(consume_reserved("restrict"))
    {
        return TQ_RESTRICT;
    }
    else if(consume_reserved("volatile"))
    {
        return TQ_VOLATILE;
    }
    else
    {
        return TQ_NONE;
    }
}


/*
make a declarator
```
declarator ::= pointer? direct-declarator
```
*/
Type *declarator(Type *type, Token **token, List(Variable) **arg_vars)
{
    if(peek_pointer())
    {
        type = pointer(type);
    }

    type = direct_declarator(type, token, arg_vars);

    return type;
}


/*
make a direct declarator
```
direct-declarator ::= identifier
                    | "(" declarator ")"
                    | direct-declarator "[" const-expression* "]"
                    | direct-declarator "(" parameter-type-list? ")"
```
* This is equivalent to the following.
```
direct-declarator ::= (identifier | "(" declarator ")") declarator-suffixes
declarator-suffixes ::= declarator-suffix*
declarator-suffix ::= ("[" const-expression* "]" | "(" ("void" | parameter-type-list)? ")")
```
*/
static Type *direct_declarator(Type *type, Token **token, List(Variable) **arg_vars)
{
    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_declarator())
        {
            // parse declarator
            Type *placeholder = calloc(1, sizeof(Type));
            Type *whole_type = declarator(placeholder, token, arg_vars);
            expect_reserved(")");

            // parse declarator-suffixes
            type = declarator_suffixes(type, arg_vars);

            // overwrite the placeholder to fix the type
            *placeholder = *type;
            type = whole_type;
            goto direct_declarator_end;
        }
        else
        {
            // restore the saved token
            set_token(saved_token);
        }
    }

    if(consume_token(TK_IDENT, token))
    {
        // do nothing (only consume an identifier)
        // note that an identifier may be omitted in a parameter-declaration
    }

    type = declarator_suffixes(type, arg_vars);

direct_declarator_end:
    return type;
}


/*
make a pointer
```
pointer ::= "*" ("*" | type-qualifier)*
```
*/
static Type *pointer(Type *base)
{
    Type *type = base;

    expect_reserved("*");
    type = new_type_pointer(type);

    while(peek_reserved("*") || peek_type_qualifier())
    {
        if(consume_reserved("*"))
        {
            type = new_type_pointer(type);
        }
        else
        {
            type->qual |= type_qualifier();
        }
    }

    return type;
}


/*
make a parameter-type-list
```
parameter-type-list ::= parameter-list ("," "...")?
```
*/
static List(Type) *parameter_type_list(List(Variable) **arg_vars, bool *variadic)
{
    List(Type) *arg_types = parameter_list(arg_vars);

    *variadic = consume_reserved(",");
    if(*variadic)
    {
        expect_reserved("...");
    }

    return arg_types;
}


/*
make a parameter-list
```
parameter-list ::= parameter-declaration ("," parameter-declaration)*
```
*/
static List(Type) *parameter_list(List(Variable) **arg_vars)
{
    bool omit_name = (arg_vars == NULL);
    List(Type) *arg_types = new_list(Type)();
    Variable *arg_var;

    add_list_entry_tail(Type)(arg_types, parameter_declaration(&arg_var, omit_name));

    if(is_void(get_first_element(Type)(arg_types)))
    {
        // stop to parse parameters
        goto parameter_list_end;
    }

    if(arg_vars != NULL)
    {
        if(arg_var == NULL)
        {
            report_error(NULL, "parameter name omitted");
        }
        else
        {
            add_list_entry_tail(Variable)(*arg_vars, arg_var);
        }
    }

    while(true)
    {
        Token *token = get_token();
        if(consume_reserved(","))
        {
            if(!consume_reserved("..."))
            {
                add_list_entry_tail(Type)(arg_types, parameter_declaration(&arg_var, omit_name));
                if(arg_vars != NULL)
                {
                    if(arg_var == NULL)
                    {
                        report_error(NULL, "parameter name omitted");
                    }
                    else
                    {
                        add_list_entry_tail(Variable)(*arg_vars, arg_var);
                    }
                }
                continue;
            }
            else
            {
                set_token(token);
            }
        }
        break;
    }

parameter_list_end:
    return arg_types;
}


/*
make a parameter declaration
```
parameter-declaration ::= declaration-specifiers (declarator | abstract-declarator?)
```
*/
static Type *parameter_declaration(Variable **arg_var, bool omit_name)
{
    StorageClassSpecifier sclass;
    Type *arg_type = declaration_specifiers(&sclass);

    if(!((sclass == SC_REGISTER) || (sclass == INVALID_DECLSPEC)))
    {
        report_error(NULL, "invalid storage class specified for parameter");
    }

    // The function 'declarator()' can also handle abstract-declarator (declarator without identifier).
    // In this case, the argument 'arg_token' is not modified.
    Token *arg_token = NULL;
    arg_type = declarator(arg_type, &arg_token, NULL);

    // adjust array type to pointer type
    if(is_array(arg_type))
    {
        arg_type = new_type_pointer(arg_type->base);
    }
    // adjust function type to pointer type
    if(is_function(arg_type))
    {
        arg_type = new_type_pointer(arg_type);
    }

    if(omit_name || is_void(arg_type))
    {
        *arg_var = NULL;
    }
    else
    {
        if(arg_token == NULL)
        {
            report_error(NULL, "parameter name omitted");
        }
        else
        {
            *arg_var = new_var(make_identifier(arg_token), arg_type, sclass, true);
        }
    }

    return arg_type;
}


/*
make a type name
```
type-name ::= specifier-qualifier-list abstract-declarator?
```
*/
Type *type_name(void)
{
    Type *type = specifier_qualifier_list();

    if(peek_abstract_declarator())
    {
        type = abstract_declarator(type);
    }

    return type;
}


/*
make an abstract-declarator
```
abstract-declarator ::= pointer | pointer? direct-abstract-declarator
```
*/
static Type *abstract_declarator(Type *type)
{
    if(peek_pointer())
    {
        type = pointer(type);
        if(!peek_direct_abstract_declarator())
        {
            goto abstract_declarator_end;
        }
    }

    type = direct_abstract_declarator(type);

abstract_declarator_end:
    return type;
}


/*
make a direct-abstract-declarator
```
direct-abstract-declarator ::= "(" abstract-declarator ")"
                             | direct-abstract-declarator? "[" const-expression "]"
                             | direct-abstract-declarator? "(" ("void" | parameter-type-list)? ")"
```
* This is equivalent to the following.
```
direct-abstract-declarator ::= ("(" abstract-declarator ")" | declarator-suffix) declarator-suffixes
declarator-suffixes ::= declarator-suffix*
declarator-suffix ::= ("[" const-expression "]" | "(" ("void" | parameter-type-list)? ")")
```
*/
static Type *direct_abstract_declarator(Type *type)
{
    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_abstract_declarator())
        {
            // parse abstract-declarator
            Type *placeholder = calloc(1, sizeof(Type));
            Type *whole_type = abstract_declarator(placeholder);
            expect_reserved(")");

            // parse declarator-suffixes
            type = declarator_suffixes(type, NULL);

            // overwrite the placeholder to fix the type
            *placeholder = *type;
            type = whole_type;
            goto direct_abstract_declarator_end;
        }
        else
        {
            // restore the saved token
            set_token(saved_token);
        }
    }

    if(!peek_declarator_suffix())
    {
        report_error(saved_token->str, "expected '[' or '('");
    }

    type = declarator_suffixes(type, NULL);

direct_abstract_declarator_end:
    return type;
}


/*
make a declarator-suffixes
```
declarator-suffixes ::= ("[" const-expression* "]" | "(" parameter-type-list? ")")*
```
*/
static Type *declarator_suffixes(Type *type, List(Variable) **arg_vars)
{
    if(consume_reserved("["))
    {
        // parse array declarator
        size_t len = 0;
        if(!consume_reserved("]"))
        {
            len = const_expression();
            expect_reserved("]");
        }

        type = declarator_suffixes(type, arg_vars);
        type = new_type_array(type, len);
    }
    else if(consume_reserved("("))
    {
        // parse parameter declarators
        List(Type) *arg_types;
        bool variadic = false;
        if(consume_reserved(")"))
        {
            arg_types = new_list(Type)();
            add_list_entry_tail(Type)(arg_types, new_type(TY_VOID, TQ_NONE));
        }
        else
        {
            arg_types = parameter_type_list(arg_vars, &variadic);
            expect_reserved(")");
        }

        type = declarator_suffixes(type, arg_vars);
        type = new_type_function(type, arg_types, variadic);
    }

    return type;
}


/*
make an initializer
```
initializer ::= assign
              | "{" initializer-list ","? "}"
```
*/
static Initializer *initializer(void)
{
    Initializer *init = new_initializer();

    if(consume_reserved("{"))
    {
        init->list = initializer_list();
        consume_reserved(","); //consume the trailing "," if it exists
        expect_reserved("}");
    }
    else
    {
        init->assign = assign();
    }

    return init;
}


/*
make an initializer-list
```
initializer-list ::= designation? initializer ("," designation? initializer)*
```
*/
static List(Initializer) *initializer_list(void)
{
    List(Initializer) *list = new_list(Initializer)();

    add_list_entry_tail(Initializer)(list, designation_and_initializer());
    while(consume_reserved(",") && !peek_reserved("}"))
    {
        add_list_entry_tail(Initializer)(list, designation_and_initializer());
    }

    return list;
}


/*
make a designation
```
designation ::= designator-list "="
```
*/
static List(Designator) *designation(void)
{
    List(Designator) *list = designator_list();
    expect_reserved("=");

    return list;
}


/*
make a designator-list
```
designator-list ::= designator designator*
```
*/
static List(Designator) *designator_list(void)
{
    List(Designator) *list = new_list(Designator)();

    add_list_entry_tail(Designator)(list, designator());
    while(peek_designator())
    {
        add_list_entry_tail(Designator)(list, designator());
    }

    return list;
}


/*
make a designator
```
designator ::= "[" const-expression "]" | "." identifier
```
*/
static Designator *designator(void)
{
    Designator *desig = calloc(1, sizeof(Designator));
    desig->index = 0;
    desig->token = NULL;
    if(consume_reserved("["))
    {
        desig->index = const_expression();
        expect_reserved("]");
    }
    else if(consume_reserved("."))
    {
        desig->token = expect_identifier();
    }
    else
    {
        report_error(NULL, "expected designator");
    }

    return desig;
}


/*
parse designation and initializer
*/
static Initializer *designation_and_initializer(void)
{
    List(Designator) *desig = peek_designator() ? designation() : NULL;
    Initializer *init = initializer();
    init->desig = desig;

    return init;
}


/*
get offset of designated object
*/
static size_t get_designation_offset(Type **type, const List(Designator) *designation)
{
    size_t offset = 0;

    for_each_entry(Designator, cursor, designation)
    {
        Designator *designator = get_element(Designator)(cursor);
        if(designator->token != NULL)
        {
            Member *member = find_member(designator->token, *type);
            *type = member->type;
            offset += member->offset;
        }
        else
        {
            *type = (*type)->base;
            offset += designator->index * (*type)->size;
        }
    }

    return offset;
}


/*
make list of maps from offset to initializer
*/
static List(InitializerMap) *make_initializer_map(Type *type, const Initializer *init)
{
    List(InitializerMap) *init_maps = new_list(InitializerMap)();

    if(init == NULL)
    {
        // assign zero
        add_list_entry_tail(InitializerMap)(init_maps, new_uninitialized_map(type->size, 0));
    }
    else
    {
        // parse initializers
        make_initializer_map_sub(init_maps, type, init, 0);

        // sort initializers
        size_t len = get_length(InitializerMap)(init_maps);
        InitializerMap *inits_vector = calloc(len, sizeof(InitializerMap));
        size_t i = 0;
        for_each_entry(InitializerMap, cursor, init_maps)
        {
            InitializerMap *map = get_element(InitializerMap)(cursor);
            inits_vector[i] = *map;
            i++;
        }
        qsort(inits_vector, len, sizeof(InitializerMap), compare_offset);

        // make list of data segments filling holes by zero
        InitializerMap *current = &inits_vector[0];
        InitializerMap *next = &inits_vector[1];
        init_maps = new_list(InitializerMap)();
        append_uninitialized_map(init_maps, current->offset, 0);
        for(size_t i = 0; i < len - 1; i++)
        {
            if(current->offset < next->offset)
            {
                add_list_entry_tail(InitializerMap)(init_maps, current);
                size_t offset = current->offset + current->size;
                append_uninitialized_map(init_maps, next->offset - offset, offset);
            }
            current++;
            next++;
        }
        add_list_entry_tail(InitializerMap)(init_maps, current);
        size_t offset = current->offset + current->size;
        append_uninitialized_map(init_maps, type->size - offset, offset);
    }

    return init_maps;
}


/*
make list of maps from offset to initializer (sub-function)
*/
static List(InitializerMap) *make_initializer_map_sub(List(InitializerMap) *init_maps, Type *type, const Initializer *init, size_t offset)
{
    if(init->assign == NULL)
    {
        if(is_array(type))
        {
            size_t index = 0;
            size_t len = 0;
            size_t bound = (type->complete ? type->len : ULONG_MAX);

            for_each_entry(Initializer, cursor, init->list)
            {
                if(index < bound)
                {
                    Initializer *init_element = get_element(Initializer)(cursor);
                    Type *sub_type = type;
                    size_t sub_offset = offset;
                    if(init_element->desig != NULL)
                    {
                        sub_offset += get_designation_offset(&sub_type, init_element->desig);
                        index = get_first_element(Designator)(init_element->desig)->index;
                    }
                    else
                    {
                        sub_type = type->base;
                        sub_offset += index * sub_type->size;
                    }
                    init_maps = concatenate_list(InitializerMap)(init_maps, make_initializer_map_sub(init_maps, sub_type, init_element, sub_offset));
                    index++;
                    len = max(len, index);
                }
                else
                {
                    report_warning(NULL, "excess elements in array initializer");
                }
            }

            if(!type->complete)
            {
                // determine length of array by initializer-list
                type->size = type->base->size * len;
                type->len = len;
                type->complete = true;
            }
        }
        else if(is_struct_or_union(type))
        {
            ListEntry(Member) *memb_cursor = get_first_entry(Member)(type->members);
            for_each_entry(Initializer, init_cursor, init->list)
            {
                if(!end_iteration(Member)(type->members, memb_cursor))
                {
                    Initializer *init_element = get_element(Initializer)(init_cursor);
                    Type *sub_type = type;
                    size_t sub_offset = offset;
                    if(init_element->desig != NULL)
                    {
                        sub_offset += get_designation_offset(&sub_type, init_element->desig);

                        const Token *token = get_first_element(Designator)(init_element->desig)->token;
                        for_each_entry(Member, cursor, type->members)
                        {
                            Member *member = get_element(Member)(cursor);
                            if(strncmp(member->name, token->str, token->len) == 0)
                            {
                                memb_cursor = cursor;
                                break;
                            }
                        }
                    }
                    else
                    {
                        Member *member = get_element(Member)(memb_cursor);
                        sub_type = member->type;
                        sub_offset += member->offset;
                    }

                    init_maps = concatenate_list(InitializerMap)(init_maps, make_initializer_map_sub(init_maps, sub_type, init_element, sub_offset));
                    memb_cursor = next_entry(Member, memb_cursor);
                }
                else
                {
                    report_warning(NULL, "excess elements in %s initializer", is_struct(type) ? "struct" : "union");
                }
            }
        }
        else if(get_first_element(Initializer)(init->list)->assign != NULL)
        {
            // The initializer for a scalar may be enclosed in braces.
            make_initializer_map_sub(init_maps, type, get_first_element(Initializer)(init->list), offset);
        }
    }
    else
    {
        if(is_array(type) && (type->base->kind == TY_CHAR) && is_string(init->assign))
        {
            // initialize array of char type by string-literal
            Initializer *init_string = new_initializer();
            init_string->list = new_list(Initializer)();
            const char *content = init->assign->var->str->content;
            for(size_t i = 0; i < strlen(content) + 1; i++)
            {
                Initializer *init  = new_initializer();
                init->assign = new_node_constant(TY_INT, content[i]);
                add_list_entry_tail(Initializer)(init_string->list, init);
            }
            make_initializer_map_sub(init_maps, type, init_string, offset);
        }
        else if(is_pointer(type) && (type->base->kind == TY_CHAR) && is_string(init->assign))
        {
            // initialize string-literal
            add_list_entry_tail(InitializerMap)(init_maps, new_string_initializer_map(init->assign->var->name, offset));
        }
        else
        {
            // initialize scalar
            add_list_entry_tail(InitializerMap)(init_maps, new_initializer_map(init->assign, type->size, offset));
        }
    }

    return init_maps;
}


/*
append an uninitialized map from offset to initializer
*/
static List(InitializerMap) *append_uninitialized_map(List(InitializerMap) *init_maps, size_t size, size_t offset)
{
    if(size > 0)
    {
        add_list_entry_tail(InitializerMap)(init_maps, new_uninitialized_map(size, offset));
    }

    return init_maps;
}


/*
compare offset of two data segments
*/
static int compare_offset(const void *data1, const void *data2)
{
    size_t offset1 = ((const InitializerMap *)data1)->offset;
    size_t offset2 = ((const InitializerMap *)data2)->offset;

    if(offset1 < offset2)
    {
        return -1;
    }
    else if(offset1 > offset2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*
determine type by type-specifiers
*/
static Type *determine_type(const int *spec_list, Type *type, TypeQualifier qual)
{
    for(size_t i = 0; i < TYPE_SPECS_MAP_SIZE; i++)
    {
        bool equal = true;

        for(size_t j = 0; j < TYPESPEC_SIZE; j++)
        {
            if(TYPE_SPECS_MAP[i].spec_list[j] != spec_list[j])
            {
                equal = false;
                break;
            }
        }

        if(equal)
        {
            TypeKind type_kind = TYPE_SPECS_MAP[i].type_kind;
            switch(type_kind)
            {
            case TY_VOID:
            case TY_CHAR:
            case TY_SCHAR:
            case TY_UCHAR:
            case TY_SHORT:
            case TY_USHORT:
            case TY_INT:
            case TY_UINT:
            case TY_LONG:
            case TY_ULONG:
            case TY_BOOL:
                return new_type(type_kind, qual);

            case TY_STRUCT:
            case TY_UNION:
            case TY_ENUM:
            case TY_TYPEDEF:
                if(type->qual != qual)
                {
                    type = copy_type(type, type->qual | qual);
                }
                return type;

            default:
                break;
            }
        }
    }

    // invalid combination of type specifiers
    report_error(NULL, "invalid combination of type specifiers");
    return NULL;
}


/*
check if it is possible to determine type by type-specifiers
*/
static bool can_determine_type(const int *spec_list)
{
    for(size_t i = 0; i < TYPESPEC_SIZE; i++)
    {
        if(spec_list[i] > 0)
        {
            return true;
        }
    }

    return false;
}


/*
check if a given node is a string-literal
*/
static bool is_string(const Expression *expr)
{
    return (expr->str != NULL);
}


/*
peek declaration-specifiers
*/
bool peek_declaration_specifiers(void)
{
    return peek_storage_class_specifier() || peek_type_specifier() || peek_type_qualifier();
}


/*
peek a storage-class-specifier
*/
static bool peek_storage_class_specifier(void)
{
    return (
           peek_reserved("typedef")
        || peek_reserved("extern")
        || peek_reserved("static")
        || peek_reserved("auto")
        || peek_reserved("register")
    );
}


/*
peek a type-specifier
*/
static bool peek_type_specifier(void)
{
    return peek_reserved_type_specifier() || peek_user_type_specifier();
}


/*
peek a reserved type-specifier
*/
static bool peek_reserved_type_specifier(void)
{
    return (
           peek_reserved("void")
        || peek_reserved("char")
        || peek_reserved("short")
        || peek_reserved("int")
        || peek_reserved("long")
        || peek_reserved("signed")
        || peek_reserved("unsigned")
        || peek_reserved("_Bool")
    );
}


/*
peek a user-defined type-specifier
*/
static bool peek_user_type_specifier(void)
{
    return (
           peek_reserved("struct")
        || peek_reserved("union")
        || peek_reserved("enum")
        || peek_typedef_name()
    );
}


/*
peek a type-qualifier
*/
static bool peek_type_qualifier(void)
{
    return (
           peek_reserved("const")
        || peek_reserved("restrict")
        || peek_reserved("volatile")
    );
}


/*
peek a declarator
*/
static bool peek_declarator(void)
{
    Token *token;

    return peek_pointer() || peek_token(TK_IDENT, &token) || peek_reserved("(");
}


/*
peek a pointer
*/
static bool peek_pointer(void)
{
    return peek_reserved("*");
}


/*
peek a type name
*/
bool peek_type_name(void)
{
    return peek_type_specifier() || peek_type_qualifier();
}


/*
peek a typedef name
*/
static bool peek_typedef_name(void)
{
    bool peek = false;
    Token *token;
    if(peek_token(TK_IDENT, &token))
    {
        Identifier *ident = find_identifier(token);
        peek = ((ident != NULL) && (ident->type_def != NULL));
    }

    return peek;
}


/*
peek an abstract-declarator
*/
static bool peek_abstract_declarator(void)
{
    return peek_pointer() || peek_direct_abstract_declarator();
}


/*
peek a direct-abstract-declarator
*/
static bool peek_direct_abstract_declarator(void)
{
    return peek_reserved("[") || peek_reserved("(");
}


/*
peek a declarator-suffix
*/
static bool peek_declarator_suffix(void)
{
    return peek_reserved("[") || peek_reserved("(");
}


/*
peek a designator
*/
static bool peek_designator(void)
{
    return peek_reserved("[") || peek_reserved(".");
}


/*
add label for a block scope identifier for an object with static storage duration
*/
static char *add_block_scope_label(const char *name)
{
    static int lab_number = 0;
    lab_number++;

    // count digits
    int count = 0;
    for(int current = lab_number; current > 0; count++, current /= 10)
    {
        count++;
    }

    char *ident = calloc(strlen(name) + count + 1, sizeof(char)); // add 1 for '\n'
    sprintf(ident, "%s%s%d", name, STATIC_VARIABLE_PUNCTUATOR, lab_number);

    return ident;
}
