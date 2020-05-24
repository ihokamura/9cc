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


// macro
#define INVALID_DECLSPEC (-1) // invalid declaration specifier
#define TYPESPEC_SIZE ((size_t)11) // number of type specifiers


// type definition
typedef enum {
    TS_VOID,     // "void"
    TS_CHAR,     // "char"
    TS_SHORT,    // "short"
    TS_INT,      // "int"
    TS_LONG,     // "long"
    TS_SIGNED,   // "signed"
    TS_UNSIGNED, // "unsigned"
    TS_STRUCT,   // structure
    TS_UNION,    // union
    TS_ENUM,     // enumeration
    TS_TYPEDEF,  // typedef name
} TypeSpecifier;

typedef struct Initializer Initializer;
struct Initializer {
    Initializer *next;  // next element
    Initializer *list;  // initializer-list
    Expression *assign; // assignment expression
};


// function prototype
static Declaration *new_declaration(Variable *var);
static Member *new_enumerator(const char *name, int value);
static Initializer *new_initializer(void);
static DataSegment *new_zero_data_segment(size_t size);
static Declaration *init_declarator_list(Type *type, StorageClassSpecifier sclass, bool is_local);
static Declaration *init_declarator(Type *type, StorageClassSpecifier sclass, bool is_local);
static StorageClassSpecifier storage_class_specifier(void);
static TypeSpecifier type_specifier(Type **type);
static Type *struct_or_union_specifier(void);
static List(Member) *struct_declaration_list(void);
static ListEntry(Member) *struct_declaration(void);
static Type *specifier_qualifier_list(void);
static ListEntry(Member) *struct_declarator_list(Type *type);
static Type *enum_specifier(void);
static List(Member) *enumerator_list(void);
static Member *enumerator(int val);
static TypeQualifier type_qualifier(void);
static Type *direct_declarator(Type *type, Token **token, List(Variable) **arg_vars);
static Type *pointer(Type *base);
static List(Type) *parameter_type_list(List(Variable) **arg_vars);
static List(Type) *parameter_list(List(Variable) **arg_vars);
static Type *parameter_declaration(Variable **arg_var);
static Type *abstract_declarator(Type *type);
static Type *direct_abstract_declarator(Type *type);
static Type *declarator_suffixes(Type *type, List(Variable) **arg_vars);
static Initializer *initializer(void);
static Initializer *initializer_list(void);
static Statement *assign_initializer(Expression *expr, const Initializer *init);
static Statement *assign_zero_initializer(Expression *expr);
static DataSegment *make_data_segment(Type *type, const Initializer *init);
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


// global variable
static const struct {int spec_list[TYPESPEC_SIZE]; TypeKind type_kind;} TYPE_SPECS_MAP[] = {
    // synonym of 'void'
    {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, TY_VOID},    // void
    // synonym of 'char'
    {{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, TY_CHAR},    // char
    // synonym of 'signed char'
    {{0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0}, TY_SCHAR},   // signed char
    // synonym of 'unsigned char'
    {{0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0}, TY_UCHAR},   // unsigned char
    // synonym of 'short'
    {{0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, TY_SHORT},   // short
    {{0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0}, TY_SHORT},   // signed short
    {{0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0}, TY_SHORT},   // short int
    {{0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0}, TY_SHORT},   // signed short int
    // synonym of 'unsigned short'
    {{0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0}, TY_USHORT},  // unsigned short
    {{0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0}, TY_USHORT},  // unsigned short int
    // synonym of 'int'
    {{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, TY_INT},     // int
    {{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, TY_INT},     // signed
    {{0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0}, TY_INT},     // signed int
    // synonym of 'unsigned'
    {{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, TY_UINT},    // unsigned
    {{0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}, TY_UINT},    // unsigned int
    // synonym of 'long'
    {{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, TY_LONG},    // long
    {{0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}, TY_LONG},    // signed long
    {{0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0}, TY_LONG},    // long int
    {{0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}, TY_LONG},    // signed long int
    // synonym of 'unsigned long'
    {{0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0}, TY_ULONG},   // unsigned long
    {{0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0}, TY_ULONG},   // unsigned long int
    // other type specifiers
    {{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, TY_STRUCT},  // structure
    {{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, TY_UNION},   // union
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, TY_ENUM},    // enumeration
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, TY_TYPEDEF}, // typedef name
}; // map from list of specifiers to kind of type
static const size_t TYPE_SPECS_MAP_SIZE = sizeof(TYPE_SPECS_MAP) / sizeof(TYPE_SPECS_MAP[0]); // size of map from list of specifiers to kind of type


/*
make a new declaration
*/
static Declaration *new_declaration(Variable *var)
{
    Declaration *decl = calloc(1, sizeof(Declaration));
    decl->next = NULL;
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
    init->next = NULL;
    init->list = NULL;
    init->assign = NULL;

    return init;
}


/*
make a new data segment
*/
DataSegment *new_data_segment(void)
{
    DataSegment *data = calloc(1, sizeof(DataSegment));
    data->next = NULL;
    data->label = NULL;
    data->size = 0;
    data->value = 0;
    data->zero = false;

    return data;
}


/*
make a new zero-valued data segment
*/
static DataSegment *new_zero_data_segment(size_t size)
{
    DataSegment *data = new_data_segment();
    data->size = size;
    data->zero = true;

    return data;
}


/*
make a declaration
```
declaration ::= declaration-specifiers init-declarator-list? ";"
```
*/
Statement *declaration(bool is_local)
{
    // parse declaration specifier
    StorageClassSpecifier sclass;
    Type *type = declaration_specifiers(&sclass);

    // parse init-declarator-list
    Statement *stmt = new_statement(STMT_DECL);
    if(peek_declarator())
    {
        stmt->decl = init_declarator_list(type, sclass, is_local);
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
    int spec_list[TYPESPEC_SIZE] = {};
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

        if(peek_type_specifier())
        {
            spec_list[type_specifier(&type)]++;
            continue;
        }
    }

    return determine_type(spec_list, type, qual);
}


/*
make a init-declarator-list
```
init-declarator-list ::= init-declarator ("," init-declarator)*
```
*/
static Declaration *init_declarator_list(Type *type, StorageClassSpecifier sclass, bool is_local)
{
    Declaration *decl = init_declarator(type, sclass, is_local);
    Declaration *cursor = decl;

    while(consume_reserved(","))
    {
        cursor->next = init_declarator(type, sclass, is_local);
        cursor = cursor->next;
    }

    return decl;
}


/*
make a init-declarator-list
```
init-declarator ::= declarator ("=" initializer)?
```
*/
static Declaration *init_declarator(Type *type, StorageClassSpecifier sclass, bool is_local)
{
    // parse declarator
    Token *token;
    type = declarator(type, &token, false);

    // check duplicated declaration
    Identifier *ident = find_identifier(token);
    if((ident != NULL) && (ident->depth == get_current_scope_depth()))
    {
        report_error(token->str, "duplicated declaration of '%s'\n", make_identifier(token));
    }

    if(sclass == SC_TYPEDEF)
    {
        // make a dummy declaration
        push_identifier_scope(make_identifier(token))->type_def = type;
        return new_declaration(NULL);
    }
    else
    {
        // make a new node for variable
        Expression *expr = new_expression(EXPR_VAR);
        expr->type = type;

        if(is_local)
        {
            expr->var = new_lvar(token, type);

            // parse initializer
            if(consume_reserved("="))
            {
                expr->var->init = assign_initializer(expr, initializer());
            }
        }
        else
        {
            bool emit = (type->kind != TY_FUNC);
            expr->var = new_gvar(token, type, emit);

            // parse initializer
            if(consume_reserved("="))
            {
                expr->var->data = make_data_segment(expr->var->type, initializer());
            }
            else
            {
                expr->var->data = new_zero_data_segment(type->size);
            }
        }

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
            *type = copy_type(ident->type_def);
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
    ListEntry(Member) *member = struct_declaration();
    ListEntry(Member) *cursor = member;
    while(!peek_reserved("}"))
    {
        // move cursor until last element since a struct-declaration may have multiple declarators
        while(cursor->next != NULL)
        {
            cursor = cursor->next;
        }
        cursor->next = struct_declaration();
    }

    while(cursor->next != NULL)
    {
        cursor = cursor->next;
    }

    List(Member) *list = new_list(Member)();
    list->head->next = member;
    member->prev = list->head;
    list->tail->prev = cursor;
    cursor->next = list->tail;

    return list;
}


/*
make a struct-declaration
```
struct-declaration ::= specifier-qualifier-list struct-declarator-list ";"
```
*/
static ListEntry(Member) *struct_declaration(void)
{
    Type *type = specifier_qualifier_list();
    ListEntry(Member) *member = struct_declarator_list(type);
    expect_reserved(";");

    return member;
}


/*
make a specifier-qualifier-list
```
specifier-qualifier-list ::= (type-specifier | type-qualifier)*
```
*/
static Type *specifier_qualifier_list(void)
{
    int spec_list[TYPESPEC_SIZE] = {};
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

        if(peek_type_specifier())
        {
            spec_list[type_specifier(&type)]++;
            continue;
        }
    }

    return determine_type(spec_list, type, qual);
}


/*
make a struct-declarator-list
```
struct-declarator-list ::= declarator ("," declarator)*
```
*/
static ListEntry(Member) *struct_declarator_list(Type *type)
{
    Token *token;
    Type *decl_type = declarator(type, &token, false);
    ListEntry(Member) *member = new_list_entry(Member)(new_member(make_identifier(token), decl_type));
    ListEntry(Member) *cursor = member;
    while(consume_reserved(","))
    {
        decl_type = declarator(type, &token, false);
        cursor = add_entry_tail(Member)(cursor, new_member(make_identifier(token), decl_type));
    }

    return member;
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
        if(consume_reserved(","))
        {
            // do nothing (only consume the trailing ",")
        }
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
                    | direct-declarator "(" ("void" | parameter-type-list)? ")"
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
static List(Type) *parameter_type_list(List(Variable) **arg_vars)
{
    List(Type) *arg_types = parameter_list(arg_vars);

    if(consume_reserved(","))
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
    List(Type) *arg_types = new_list(Type)();
    Variable *arg_var;

    add_list_entry_tail(Type)(arg_types, parameter_declaration(&arg_var));
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
                add_list_entry_tail(Type)(arg_types, parameter_declaration(&arg_var));
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

    return arg_types;
}


/*
make a parameter declaration
```
parameter-declaration ::= declaration-specifiers (declarator | abstract-declarator?)
```
*/
static Type *parameter_declaration(Variable **arg_var)
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

    if(arg_token != NULL)
    {
        *arg_var = new_var(make_identifier(arg_token), arg_type, true);
    }
    else
    {
        *arg_var = NULL;
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
            type = declarator_suffixes(type, false);

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

    type = declarator_suffixes(type, false);

direct_abstract_declarator_end:
    return type;
}


/*
make a declarator-suffixes
```
declarator-suffixes ::= ("[" const-expression* "]" | "(" ("void" | parameter-type-list)? ")")*
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
        if(consume_reserved(")"))
        {
            arg_types = new_list(Type)();
            add_list_entry_tail(Type)(arg_types, new_type(TY_VOID, TQ_NONE));
        }
        else if(consume_reserved("void"))
        {
            arg_types = new_list(Type)();
            add_list_entry_tail(Type)(arg_types, new_type(TY_VOID, TQ_NONE));
            expect_reserved(")");
        }
        else
        {
            arg_types = parameter_type_list(arg_vars);
            expect_reserved(")");
        }

        type = declarator_suffixes(type, arg_vars);
        type = new_type_function(type, arg_types);
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
        if(consume_reserved(","))
        {
            // do nothing (only consume the trailing ",")
        }
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
initializer-list ::= initializer ("," initializer)*
```
*/
static Initializer *initializer_list(void)
{
    Initializer head = {};
    Initializer *cursor = &head;

    cursor->next = initializer();
    cursor = cursor->next;

    while(true)
    {
        Token *token = get_token();
        if(consume_reserved(","))
        {
            if(!consume_reserved("}"))
            {
                cursor->next = initializer();
                cursor = cursor->next;
                continue;
            }
            else
            {
                set_token(token);
            }
        }
        break;
    }

    return head.next;
}


/*
assign initial value to object
*/
static Statement *assign_initializer(Expression *expr, const Initializer *init)
{
    Statement *init_stmt = new_statement(STMT_COMPOUND);

    if(init->assign == NULL)
    {
        if(is_array(expr->type))
        {
            Statement stmt_head = {};
            Statement *stmt_cursor = &stmt_head;
            const Initializer *init_cursor = init->list;

            if(expr->type->complete)
            {
                size_t index = 0;
                while((init_cursor != NULL) && (index < expr->type->len))
                {
                    Expression *dest = new_node_subscript(expr, index);
                    stmt_cursor->next = assign_initializer(dest, init_cursor);
                    stmt_cursor = stmt_cursor->next;
                    init_cursor = init_cursor->next;
                    index++;
                }

                while(index < expr->type->len)
                {
                    // handle the remainder
                    Expression *dest = new_node_subscript(expr, index);
                    stmt_cursor->next = assign_zero_initializer(dest);
                    stmt_cursor = stmt_cursor->next;
                    index++;
                }
            }
            else
            {
                // determine length of array by initializer-list
                size_t len = 0;
                while(init_cursor != NULL)
                {
                    Expression *dest = new_node_subscript(expr, len);
                    stmt_cursor->next = assign_initializer(dest, init_cursor);
                    stmt_cursor = stmt_cursor->next;
                    init_cursor = init_cursor->next;
                    len++;
                }
                expr->type->size = expr->type->base->size * len;
                expr->type->len = len;
                expr->type->complete = true;
            }

            init_stmt->body = stmt_head.next;
        }
        else if(is_struct(expr->type))
        {
            Statement stmt_head = {};
            Statement *stmt_cursor = &stmt_head;
            const Initializer *init_cursor = init->list;
            ListEntry(Member) *memb_cursor = get_first_entry(Member)(expr->type->members);

            while((init_cursor != NULL) && !end_iteration(Member)(expr->type->members, memb_cursor))
            {
                Member *member = get_element(Member)(memb_cursor);
                Expression *dest = new_node_member(expr, member);
                stmt_cursor->next = assign_initializer(dest, init_cursor);
                stmt_cursor = stmt_cursor->next;
                init_cursor = init_cursor->next;
                memb_cursor = memb_cursor->next;
            }

            while(!end_iteration(Member)(expr->type->members, memb_cursor))
            {
                // handle the remainder
                Member *member = get_element(Member)(memb_cursor);
                Expression *dest = new_node_member(expr, member);
                stmt_cursor->next = assign_zero_initializer(dest);
                stmt_cursor = stmt_cursor->next;
                memb_cursor = memb_cursor->next;
            }

            init_stmt->body = stmt_head.next;
        }
        else if(is_union(expr->type))
        {
            const Initializer *init_cursor = init->list;
            Member *member = get_element(Member)(get_first_entry(Member)(expr->type->members));
            Expression *dest = new_node_member(expr, member);
            init_stmt->body = assign_initializer(dest, init_cursor);
        }
        else if(init->list->assign != NULL)
        {
            // The initializer for a scalar may be enclosed in braces.
            init_stmt->body = new_statement(STMT_EXPR);
            init_stmt->body->expr = new_node_binary(EXPR_ASSIGN, expr, init->list->assign);
        }
        else
        {
            report_error(NULL, "invalid initializer");
        }
    }
    else
    {
        if(is_array(expr->type) && (expr->type->base->kind == TY_CHAR) && is_string(init->assign))
        {
            // initialize array of char type by string-literal
            Initializer head = {};
            Initializer *cursor = &head;
            const char *content = init->assign->var->str->content;
            for(size_t i = 0; i < strlen(content) + 1; i++)
            {
                cursor->next = new_initializer();
                cursor = cursor->next;
                cursor->assign = new_node_constant(TY_INT, content[i]);
            }

            Initializer *init_string = new_initializer();
            init_string->list = head.next;
            init_stmt->body = assign_initializer(expr, init_string);
        }
        else
        {
            init_stmt->body = new_statement(STMT_EXPR);
            init_stmt->body->expr = new_node_binary(EXPR_ASSIGN, expr, init->assign);
        }
    }

    return init_stmt;
}


/*
assign zero to object
*/
static Statement *assign_zero_initializer(Expression *expr)
{
    Statement *init_stmt = new_statement(STMT_COMPOUND);

    if(is_array(expr->type))
    {
        Statement stmt_head = {};
        Statement *stmt_cursor = &stmt_head;
        for(size_t index = 0; index < expr->type->len; index++)
        {
            Expression *dest = new_node_subscript(expr, index);
            stmt_cursor->next = assign_zero_initializer(dest);
            stmt_cursor = stmt_cursor->next;
        }
    }
    else if(is_struct(expr->type))
    {
        Statement stmt_head = {};
        Statement *stmt_cursor = &stmt_head;
        for_each_entry(Member, cursor, expr->type->members)
        {
            Member *member = get_element(Member)(cursor);
            Expression *dest = new_node_member(expr, member);
            stmt_cursor->next = assign_zero_initializer(dest);
            stmt_cursor = stmt_cursor->next;
        }
        init_stmt->body = stmt_head.next;
    }
    else if(is_union(expr->type))
    {
        Member *member = get_element(Member)(get_first_entry(Member)(expr->type->members));
        Expression *dest = new_node_member(expr, member);
        init_stmt->body = assign_zero_initializer(dest);
    }
    else
    {
        init_stmt->body = new_statement(STMT_EXPR);
        init_stmt->body->expr = new_node_binary(EXPR_ASSIGN, expr, new_node_constant(TY_INT, 0));
    }

    return init_stmt;
}


/*
make list of contents for data segment of global variable
*/
static DataSegment *make_data_segment(Type *type, const Initializer *init)
{
    DataSegment *data = NULL;

    if(init->assign == NULL)
    {
        if(is_array(type))
        {
            DataSegment data_head = {};
            DataSegment *data_cursor = &data_head;
            const Initializer *init_cursor = init->list;

            if(type->complete)
            {
                size_t index = 0;
                while((init_cursor != NULL) && (index < type->len))
                {
                    data_cursor->next = make_data_segment(type->base, init_cursor);
                    while(data_cursor->next != NULL)
                    {
                        data_cursor = data_cursor->next;
                    }
                    init_cursor = init_cursor->next;
                    index++;
                }

                // fill the remainder by zero
                size_t remainder = (type->len - index) * type->base->size;
                if(remainder > 0)
                {
                    data_cursor->next = new_zero_data_segment(remainder);
                    data_cursor = data_cursor->next;
                }
            }
            else
            {
                // determine length of array by initializer-list
                size_t len = 0;
                while(init_cursor != NULL)
                {
                    data_cursor->next = make_data_segment(type->base, init_cursor);
                    while(data_cursor->next != NULL)
                    {
                        data_cursor = data_cursor->next;
                    }
                    init_cursor = init_cursor->next;
                    len++;
                }
                type->size = type->base->size * len;
                type->len = len;
                type->complete = true;
            }

            data = data_head.next;
        }
        else if(is_struct(type))
        {
            DataSegment data_head = {};
            DataSegment *data_cursor = &data_head;
            const Initializer *init_cursor = init->list;
            ListEntry(Member) *memb_cursor = get_first_entry(Member)(type->members);

            while((init_cursor != NULL) && (!end_iteration(Member)(type->members, memb_cursor)))
            {
                Member *member = get_element(Member)(memb_cursor);
                data_cursor->next = make_data_segment(member->type, init_cursor);
                while(data_cursor->next != NULL)
                {
                    data_cursor = data_cursor->next;
                }

                // fill padding by zero
                size_t start = member->offset + member->type->size;
                size_t end = (end_iteration(Member)(type->members, memb_cursor->next) ? type->size : get_element(Member)(memb_cursor->next)->offset);
                size_t padding_size = end - start;
                if(padding_size > 0)
                {
                    DataSegment *data_zero = new_data_segment();
                    data_zero->size = padding_size;
                    data_zero->zero = true;
                    data_cursor->next = data_zero;
                    data_cursor = data_cursor->next;
                }
                init_cursor = init_cursor->next;
                memb_cursor = memb_cursor->next;
            }

            // fill the remainder by zero
            if(!end_iteration(Member)(type->members, memb_cursor))
            {
                Member *member = get_element(Member)(memb_cursor);
                size_t remainder = type->size - member->offset;
                data_cursor->next = new_zero_data_segment(remainder);
                data_cursor = data_cursor->next;
            }

            data = data_head.next;
        }
        else if(is_union(type))
        {
            const Initializer *init_cursor = init->list;
            Member *member = get_element(Member)(get_first_entry(Member)(type->members));
            data = make_data_segment(member->type, init_cursor);
        }
        else if(init->list->assign != NULL)
        {
            // The initializer for a scalar may be enclosed in braces.
            data = make_data_segment(type, init->list);
        }
        else
        {
            data = new_data_segment();
            data->size = type->size;
            data->zero = true;
        }
    }
    else
    {
        if(is_array(type) && (type->base->kind == TY_CHAR) && is_string(init->assign))
        {
            // initialize array of char type by string-literal
            Initializer head = {};
            Initializer *cursor = &head;
            const char *content = init->assign->var->str->content;
            for(size_t i = 0; i < strlen(content) + 1; i++)
            {
                cursor->next = new_initializer();
                cursor = cursor->next;
                cursor->assign = new_node_constant(TY_INT, content[i]);
            }

            Initializer *init_string = new_initializer();
            init_string->list = head.next;

            data = make_data_segment(type, init_string);
        }
        else if(is_pointer(type) && (type->base->kind == TY_CHAR) && is_string(init->assign))
        {
            // initialize string-literal
            data = new_data_segment();
            data->label = init->assign->var->name;
        }
        else
        {
            data = new_data_segment();
            data->size = type->size;
            data->value = evaluate(init->assign);
        }
    }

    return data;
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
                return new_type(type_kind, qual);

            case TY_STRUCT:
            case TY_UNION:
            case TY_ENUM:
            case TY_TYPEDEF:
                type->qual |= qual;
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
