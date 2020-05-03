/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser (syntax tree constructor)
*/

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// macro definition
#define INVALID_DECLSPEC (-1) // invalid declaration specifier
#define TYPESPEC_SIZE ((size_t)11) // number of type specifiers


// type definition
typedef enum {
    SC_TYPEDEF,  // "typdef"
    SC_EXTERN,   // "extern"
    SC_STATIC,   // "static"
    SC_AUTO,     // "auto"
    SC_REGISTER, // "register"
} StorageClassSpecifier;

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

typedef struct Identifier Identifier;
struct Identifier {
    Identifier *next; // next element
    const char *name; // identifier
    Variable *var;    // variable
    Enumerator *en;   // enumerator
    Type *type_def;   // type definition
    int depth;        // depth of scope
};

typedef struct Tag Tag;
struct Tag {
    Tag *next;        // next element
    const char *name; // name of tag
    Type *type;       // type 
    int depth;        // depth of scope
};

typedef struct {
    Identifier *ident_list; // list of ordinary identifiers visible in the current scope
    Tag *tag_list;          // list of tags visible in the current scope
    int depth;              // depth of the current scope
} Scope;

typedef struct Initializer Initializer;
struct Initializer {
    Initializer *next;  // next element
    Initializer *list;  // initializer-list
    Expression *assign; // assignment expression
};

// function prototype
static void program(void);
static void function_def(void);
static Type *declaration_specifiers(StorageClassSpecifier *sclass);
static Type *specifier_qualifier_list(void);
static Type *declarator(Type *type, Token **token, Variable **arg_vars);
static Type *direct_declarator(Type *type, Token **token, Variable **arg_vars);
static Type *pointer(Type *base);
static Type *parameter_type_list(Variable **arg_vars);
static Type *parameter_list(Variable **arg_vars);
static Type *parameter_declaration(Variable **arg_var);
static Type *type_name(void);
static Type *abstract_declarator(Type *type);
static Type *direct_abstract_declarator(Type *type);
static Type *declarator_suffixes(Type *type, Variable **arg_vars);
static StorageClassSpecifier storage_class_specifier(void);
static TypeSpecifier type_specifier(Type **type);
static TypeQualifier type_qualifier(void);
static Type *struct_or_union_specifier(void);
static Member *struct_declaration_list(void);
static Member *struct_declaration(void);
static Member *struct_declarator_list(Type *type);
static Type *enum_specifier(void);
static void enumerator_list(void);
static int enumerator(int val);
static Statement *statement(void);
static Statement *compound_statement(void);
static Statement *declaration(bool is_local);
static Statement *init_declarator_list(Type *type, StorageClassSpecifier sclass, bool is_local);
static Statement *init_declarator(Type *type, StorageClassSpecifier sclass, bool is_local);
static Initializer *initializer(void);
static Initializer *initializer_list(void);
static Statement *assign_initializer(Expression *expr, const Initializer *init);
static Statement *assign_zero_initializer(Expression *expr);
static DataSegment *make_data_segment(Type *type, const Initializer *init);
static Expression *expression(void);
static Expression *assign(void);
static Expression *conditional(void);
static Expression *logical_or(void);
static Expression *logical_and(void);
static Expression *bitwise_or(void);
static Expression *bitwise_xor(void);
static Expression *bitwise_and(void);
static Expression *equality(void);
static Expression *relational(void);
static Expression *shift(void);
static Expression *additive(void);
static Expression *multiplicative(void);
static Expression *cast(void);
static Expression *unary(void);
static Expression *postfix(void);
static Expression *arg_expr_list(void);
static Expression *primary(void);
static Expression *new_expression(ExpressionKind kind);
static Statement *new_statement(StatementKind kind);
static Expression *new_node_unary(ExpressionKind kind, Expression *lhs);
static Expression *new_node_binary(ExpressionKind kind, Expression *lhs, Expression *rhs);
static Expression *new_node_subscript(Expression *base, size_t index);
static Expression *new_node_member(Expression *expr, Member *member);
static Expression *new_node_integer(TypeKind kind, long value);
static Expression *apply_integer_promotion(Expression *expr);
static void apply_arithmetic_conversion(Expression *lhs, Expression *rhs);
static Variable *new_var(const char *name, Type *type, bool local);
static Variable *new_gvar(const Token *token, Type *type, bool entity);
static Variable *new_string(const Token *token);
static Variable *new_lvar(const Token *token, Type *type);
static Function *new_function(const Token *token, Type *type, Variable *args, Statement *body);
static Enumerator *new_enumerator(const char *name, int value);
static Identifier *push_identifier_scope(const char *name);
static Tag *push_tag_scope(const char *name);
static Scope enter_scope(void);
static void leave_scope(Scope scope);
static Identifier *find_identifier(const Token *token);
static Tag *find_tag(const Token *token);
static Initializer *new_initializer(void);
static DataSegment *new_data_segment(void);
static DataSegment *new_zero_data_segment(size_t size);
static long parse_integer_constant(const Token *token, TypeKind *kind);
static bool peek_declaration_specifiers(void);
static bool peek_storage_class_specifier(void);
static bool peek_type_specifier(void);
static bool peek_reserved_type_specifier(void);
static bool peek_user_type_specifier(void);
static bool peek_typedef_name(void);
static bool peek_type_qualifier(void);
static bool peek_declarator(void);
static bool peek_pointer(void);
static bool peek_type_name(void);
static bool peek_abstract_declarator(void);
static bool peek_direct_abstract_declarator(void);
static bool peek_declarator_suffix(void);
static bool peek_func(void);
static char *new_string_label(void);
static bool is_string(const Expression *expr);
static long const_expression(void);
static long evaluate(Expression *expr);
static size_t adjust_alignment(size_t target, size_t alignment);
static Type *determine_type(const int *spec_list, Type *type, TypeQualifier qual);
static bool can_determine_type(const int *spec_list);


// global variable
static int str_number = 0; // label number of string-literal
static Function *function_list = NULL; // list of functions
static Variable *gvar_list = NULL; // list of global variables
static Variable *lvar_list = NULL; // list of local variables of currently constructing function
static Statement *current_switch = NULL; // currently parsing switch statement
static Scope current_scope = {NULL, NULL, 0}; // current scope
static const size_t STACK_ALIGNMENT = 8; // alignment of function stack
static const struct {int spec_list[TYPESPEC_SIZE]; TypeKind type_kind;} TYPE_SPECS_MAP[] = {
    // synonym of 'void'
    {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, TY_VOID},    // void
    // synonym of 'char'
    {{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, TY_CHAR},    // char
    // synonym of 'signed char'
    {{0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0}, TY_CHAR},    // signed char
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
construct syntax tree
*/
void construct(Program *prog)
{
    program();
    prog->gvars = gvar_list;
    prog->funcs = function_list;
}


/*
make a program
```
program ::= (declaration | function-def)*
```
*/
static void program(void)
{
    Function func_head = {};
    function_list = &func_head;
    Variable gvar_head = {};
    gvar_list = &gvar_head;

    while(!at_eof())
    {
        if(peek_func())
        {
            // parse function
            function_def();
        }
        else
        {
            // parse global variable
            declaration(false);
        }
    }

    function_list = func_head.next;
    gvar_list = gvar_head.next;
}


/*
make a function definition
```
function-def ::= declaration-specifiers declarator compound-statement
```
*/
static void function_def(void)
{
    // clear list of local variables
    lvar_list = NULL;

    // save the current scope
    Scope scope = enter_scope();

    // parse declaration specifier and declarator
    StorageClassSpecifier sclass;
    Type *type = declaration_specifiers(&sclass);
    Token *token;
    Variable args_head = {};
    Variable *args = &args_head;
    type = declarator(type, &token, &args);
    args = args_head.next;

    // parse body
    Statement *body = compound_statement();

    // restore the scope
    leave_scope(scope);

    // make a new function
    new_function(token, type, args, body);
}


/*
make a declaration specifier
```
declaration-specifiers ::= (storage-class-specifier | type-specifier | type-qualifier)*
```
* This function sets INVALID_DECLSPEC to the argument 'sclass' if no storage-class specifier is given.
*/
static Type *declaration_specifiers(StorageClassSpecifier *sclass)
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
make a declarator
```
declarator ::= pointer? direct-declarator
```
*/
static Type *declarator(Type *type, Token **token, Variable **arg_vars)
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
static Type *direct_declarator(Type *type, Token **token, Variable **arg_vars)
{
    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_declarator())
        {
            // parse declarator
            Type *placeholder = new_type(TY_VOID, TQ_NONE);
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
static Type *parameter_type_list(Variable **arg_vars)
{
    Type *arg_types = parameter_list(arg_vars);

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
static Type *parameter_list(Variable **arg_vars)
{
    Type arg_types_head = {};
    Type *arg_types_cursor = &arg_types_head;
    Variable *arg_var;
    Variable *arg_vars_cursor = (arg_vars != NULL) ? *arg_vars : NULL;

    arg_types_cursor->next = parameter_declaration(&arg_var);
    arg_types_cursor = arg_types_cursor->next;
    if(arg_vars != NULL)
    {
        if(arg_var == NULL)
        {
            report_error(NULL, "parameter name omitted");
        }
        else
        {
            arg_vars_cursor->next = arg_var;
            arg_vars_cursor = arg_vars_cursor->next;
        }
    }

    while(true)
    {
        Token *token = get_token();
        if(consume_reserved(","))
        {
            if(!consume_reserved("..."))
            {
                arg_types_cursor->next = parameter_declaration(&arg_var);
                arg_types_cursor = arg_types_cursor->next;
                if(arg_vars != NULL)
                {
                    if(arg_var == NULL)
                    {
                        report_error(NULL, "parameter name omitted");
                    }
                    else
                    {
                        arg_vars_cursor->next = arg_var;
                        arg_vars_cursor = arg_vars_cursor->next;
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

    return arg_types_head.next;
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
static Type *type_name(void)
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
            Type *placeholder = new_type(TY_VOID, TQ_NONE);
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
static Type *declarator_suffixes(Type *type, Variable **arg_vars)
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
        Type *arg_types = new_type(TY_VOID, TQ_NONE);
        if(!consume_reserved(")"))
        {
            if(!consume_reserved("void"))
            {
                arg_types = parameter_type_list(arg_vars);
            }
            expect_reserved(")");
        }
        type = declarator_suffixes(type, arg_vars);
        type = new_type_function(type, arg_types);
    }

    return type;
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

        if((tag != NULL) && (tag->type->kind != type_kind) && (tag->depth == current_scope.depth))
        {
            report_error(token->str, "'%s' defined as wrong kind of tag", name);
        }

        if(tag == NULL)
        {
            // declaration of a new tag
            type = new_type(type_kind, TQ_NONE);
            push_tag_scope(name)->type = type;
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
            if((tag != NULL) && (tag->depth < current_scope.depth))
            {
                // make a new type since this is a declaration of a new tag in the current scope
                type = new_type(type_kind, TQ_NONE);
                push_tag_scope(name)->type = type;
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
        type->member = struct_declaration_list();

        if(spec_kind == TS_STRUCT)
        {
            // set offset of members and determine size and alignment of the structure type
            size_t offset = 0;
            size_t alignment = 0;
            for(Member *cursor = type->member; cursor != NULL; cursor = cursor->next)
            {
                cursor->offset = adjust_alignment(offset, cursor->type->align);
                offset = cursor->offset + cursor->type->size;
                if(alignment < cursor->type->align)
                {
                    alignment = cursor->type->align;
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
            for(Member *cursor = type->member; cursor != NULL; cursor = cursor->next)
            {
                // offset is always 0
                cursor->offset = 0;
                if(size < cursor->type->size)
                {
                    size = cursor->type->size;
                }
                if(alignment < cursor->type->align)
                {
                    alignment = cursor->type->align;
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
static Member *struct_declaration_list(void)
{
    Member *member = struct_declaration();
    Member *cursor = member;
    while(!peek_reserved("}"))
    {
        // move cursor until last element since a struct-declaration may have multiple declarators
        while(cursor->next != NULL)
        {
            cursor = cursor->next;
        }
        cursor->next = struct_declaration();
    }

    return member;
}


/*
make a struct-declaration
```
struct-declaration ::= specifier-qualifier-list struct-declarator-list ";"
```
*/
static Member *struct_declaration(void)
{
    Type *type = specifier_qualifier_list();
    Member *member = struct_declarator_list(type);
    expect_reserved(";");

    return member;
}


/*
make a struct-declarator-list
```
struct-declarator-list ::= declarator ("," declarator)*
```
*/
static Member *struct_declarator_list(Type *type)
{
    Token *token;
    Type *decl_type = declarator(type, &token, false);
    Member *member = new_member(token, decl_type);
    Member *cursor = member;
    while(consume_reserved(","))
    {
        decl_type = declarator(type, &token, false);
        cursor->next = new_member(token, decl_type);
        cursor = cursor->next;
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

        if((tag != NULL) && (tag->type->kind != TY_ENUM) && (tag->depth == current_scope.depth))
        {
            report_error(token->str, "'%s' defined as wrong kind of tag", name);
        }

        if(tag == NULL)
        {
            // declaration of a new tag
            type = new_type_enum();
            push_tag_scope(name)->type = type;
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
            if((tag != NULL) && (tag->depth < current_scope.depth))
            {
                // make a new type since this is a declaration of a new tag in the current scope
                type = new_type_enum();
                push_tag_scope(name)->type = type;
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
        enumerator_list();
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
static void enumerator_list(void)
{
    Token *token;

    int val = enumerator(0);
    while(consume_reserved(",") && peek_token(TK_IDENT, &token))
    {
        val = enumerator(val);
    }
}


/*
make an enumerator
```
enumerator ::= identifier ("=" const-expression)?
```
*/
static int enumerator(int value)
{
    Token *token = expect_identifier();
    if(consume_reserved("="))
    {
        value = const_expression();
    }
    new_enumerator(make_identifier(token), value);

    return value + 1;
}


/*
make a statement
```
statement ::= identifier ":" statement
            | "case" const-expression ":" statement
            | "default" ":" statement
            | compound-statement
            | expr? ";"
            | "if" "(" expression ")" statement ("else" statement)?
            | "switch" "(" expression ")" statement
            | "while" "(" expression ")" statement
            | "do" statement "while" "(" expression ")" ";"
            | "for" "(" expression? ";" expression? ";" expression? ")" statement
            | "goto" identifier ";"
            | "continue" ";"
            | "break" ";"
            | "return" expression ";"
```
*/
static Statement *statement(void)
{
    Statement *stmt;

    if(peek_reserved("{"))
    {
        // save the current scope
        Scope scope = enter_scope();

        stmt = new_statement(STMT_COMPOUND);
        stmt->body = compound_statement();

        // restore the scope
        leave_scope(scope);
    }
    else if(consume_reserved("break"))
    {
        stmt = new_statement(STMT_BREAK);
        expect_reserved(";");
    }
    else if(consume_reserved("case"))
    {
        // parse label expression
        long value = const_expression();
        expect_reserved(":");

        // parse statement for the case label
        stmt = new_statement(STMT_CASE);
        stmt->body = statement();

        // save the value of label expression and update node of currently parsing switch statement
        stmt->value = value;
        stmt->next_case = current_switch->next_case;
        current_switch->next_case = stmt;
    }
    else if(consume_reserved("continue"))
    {
        stmt = new_statement(STMT_CONTINUE);
        expect_reserved(";");
    }
    else if(consume_reserved("default"))
    {
        expect_reserved(":");

        // parse statement for the default label
        stmt = new_statement(STMT_CASE);
        stmt->body = statement();

        // update node of currently parsing switch statement
        current_switch->default_case = stmt;
    }
    else if(consume_reserved("do"))
    {
        stmt = new_statement(STMT_DO);

        // parse loop body
        stmt->body = statement();

        expect_reserved("while");
        expect_reserved("(");

        // parse loop condition
        stmt->cond = expression();

        expect_reserved(")");
        expect_reserved(";");
    }
    else if(consume_reserved("for"))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        stmt = new_statement(STMT_FOR);
        expect_reserved("(");

        // parse clause-1
        if(!consume_reserved(";"))
        {
            stmt->preexpr = expression();
            expect_reserved(";");
        }

        // parse expression-2
        if(!consume_reserved(";"))
        {
            stmt->cond = expression();
            expect_reserved(";");
        }

        // parse expression-3
        if(!consume_reserved(")"))
        {
            stmt->postexpr = expression();
            expect_reserved(")");
        }

        // parse loop body
        stmt->body = statement();
    }
    else if(consume_reserved("goto"))
    {
        stmt = new_statement(STMT_GOTO);
        stmt->ident = make_identifier(expect_identifier());
        expect_reserved(";");
    }
    else if(consume_reserved("if"))
    {
        stmt = new_statement(STMT_IF);
        expect_reserved("(");

        // parse condition
        stmt->cond = expression();

        expect_reserved(")");

        // parse statement in case of condition being true
        stmt->true_case = statement();

        // parse statement in case of condition being false
        if(consume_reserved("else"))
        {
            stmt->false_case = statement();
        }
    }
    else if(consume_reserved("return"))
    {
        stmt = new_statement(STMT_RETURN);
        if(!consume_reserved(";"))
        {
            // return statement with an expression
            stmt->expr = expression();
            expect_reserved(";");
        }
    }
    else if(consume_reserved("switch"))
    {
        // save node of previous switch statement
        Statement *prev_switch = current_switch;

        // parse controlling expression
        stmt = new_statement(STMT_SWITCH);
        expect_reserved("(");
        stmt->cond = expression();
        expect_reserved(")");

        // update node of currently parsing switch statement and parse body
        current_switch = stmt;
        stmt->body = statement();

        // restore node of previous switch statement
        current_switch = prev_switch;
    }
    else if(consume_reserved("while"))
    {
        stmt = new_statement(STMT_WHILE);
        expect_reserved("(");

        // parse loop condition
        stmt->cond = expression();

        expect_reserved(")");

        // parse loop body
        stmt->body = statement();
    }
    else if(consume_reserved(";"))
    {
        // null statement
        stmt = new_statement(STMT_NULL);
    }
    else
    {
        Token *saved_token = get_token();
        Token *token;
        if(consume_token(TK_IDENT, &token))
        {
            if(consume_reserved(":"))
            {
                // labeled statement
                stmt = new_statement(STMT_LABEL);
                stmt->body = statement();
                stmt->ident = make_identifier(token);
                goto statement_end;
            }
            else
            {
                // resume the token since it is not a label
                set_token(saved_token);
            }
        }

        // expression statement
        stmt = new_statement(STMT_EXPR);
        stmt->expr = expression();
        expect_reserved(";");
    }

statement_end:
    return stmt;
}


/*
make a compound statement
```
compound-statement ::= "{" (declaration | statement)* "}"
```
*/
static Statement *compound_statement(void)
{
    expect_reserved("{");

    // parse declaration and/or statement until reaching '}'
    Statement head = {};
    Statement *cursor = &head;
    while(!consume_reserved("}"))
    {
        if(peek_declaration_specifiers())
        {
            // declaration
            cursor->next = declaration(true);
        }
        else
        {
            // statement
            cursor->next = statement();
        }
        cursor = cursor->next;
    }

    return head.next;
}


/*
make a declaration
```
declaration ::= declaration-specifiers init-declarator-list? ";"
```
*/
static Statement *declaration(bool is_local)
{
    // parse declaration specifier
    StorageClassSpecifier sclass;
    Type *type = declaration_specifiers(&sclass);

    // parse init-declarator-list
    Statement *stmt = new_statement((sclass == SC_TYPEDEF) ? STMT_NULL : STMT_DECL);
    if(peek_declarator())
    {
        stmt->body = init_declarator_list(type, sclass, is_local);
    }

    expect_reserved(";");

    return stmt;
}


/*
make a init-declarator-list
```
init-declarator-list ::= init-declarator ("," init-declarator)*
```
*/
static Statement *init_declarator_list(Type *type, StorageClassSpecifier sclass, bool is_local)
{
    Statement *stmt = init_declarator(type, sclass, is_local);
    Statement *cursor = stmt;

    while(consume_reserved(","))
    {
        cursor->next = init_declarator(type, sclass, is_local);
        cursor = cursor->next;
    }

    return stmt;
}


/*
make a init-declarator-list
```
init-declarator ::= declarator ("=" initializer)?
```
*/
static Statement *init_declarator(Type *type, StorageClassSpecifier sclass, bool is_local)
{
    // parse declarator
    Token *token;
    type = declarator(type, &token, false);

    // check duplicated declaration
    Identifier *ident = find_identifier(token);
    if((ident != NULL) && (ident->depth == current_scope.depth))
    {
        report_error(token->str, "duplicated declaration of '%s'\n", make_identifier(token));
    }

    if(sclass == SC_TYPEDEF)
    {
        push_identifier_scope(make_identifier(token))->type_def = type;

        static Statement dummy_stmt;
        return &dummy_stmt;
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

        Statement *stmt = new_statement(STMT_DECL);
        stmt->var = expr->var;
        return stmt;
    }
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
            Member *member = expr->type->member;

            while((init_cursor != NULL) && (member != NULL))
            {
                Expression *dest = new_node_member(expr, member);
                stmt_cursor->next = assign_initializer(dest, init_cursor);
                stmt_cursor = stmt_cursor->next;
                init_cursor = init_cursor->next;
                member = member->next;
            }

            while(member != NULL)
            {
                // handle the remainder
                Expression *dest = new_node_member(expr, member);
                stmt_cursor->next = assign_zero_initializer(dest);
                stmt_cursor = stmt_cursor->next;
                member = member->next;
            }

            init_stmt->body = stmt_head.next;
        }
        else if(is_union(expr->type))
        {
            const Initializer *init_cursor = init->list;
            Expression *dest = new_node_member(expr, expr->type->member);
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
            char *content = init->assign->var->content;
            for(size_t i = 0; i < strlen(content) + 1; i++)
            {
                cursor->next = new_initializer();
                cursor = cursor->next;
                cursor->assign = new_node_integer(TY_INT, content[i]);
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
        for(Member *member = expr->type->member; member != NULL; member = member->next)
        {
            Expression *dest = new_node_member(expr, member);
            stmt_cursor->next = assign_zero_initializer(dest);
            stmt_cursor = stmt_cursor->next;
        }
        init_stmt->body = stmt_head.next;
    }
    else if(is_union(expr->type))
    {
        Expression *dest = new_node_member(expr, expr->type->member);
        init_stmt->body = assign_zero_initializer(dest);
    }
    else
    {
        init_stmt->body = new_statement(STMT_EXPR);
        init_stmt->body->expr = new_node_binary(EXPR_ASSIGN, expr, new_node_integer(TY_INT, 0));
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
            Member *member = type->member;

            while((init_cursor != NULL) && (member != NULL))
            {
                data_cursor->next = make_data_segment(member->type, init_cursor);
                while(data_cursor->next != NULL)
                {
                    data_cursor = data_cursor->next;
                }

                // fill padding by zero
                size_t start = member->offset + member->type->size;
                size_t end = (member->next == NULL ? type->size : member->next->offset);
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
                member = member->next;
            }

            // fill the remainder by zero
            if(member != NULL)
            {
                size_t remainder = type->size - member->offset;
                data_cursor->next = new_zero_data_segment(remainder);
                data_cursor = data_cursor->next;
            }

            data = data_head.next;
        }
        else if(is_union(type))
        {
            const Initializer *init_cursor = init->list;
            data = make_data_segment(type->member->type, init_cursor);
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
            char *content = init->assign->var->content;
            for(size_t i = 0; i < strlen(content) + 1; i++)
            {
                cursor->next = new_initializer();
                cursor = cursor->next;
                cursor->assign = new_node_integer(TY_INT, content[i]);
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
make an expression
```
expression ::= assign ("," assign)*
```
*/
static Expression *expression(void)
{
    Expression *node = assign();

    // parse tokens while finding an assignment expression
    while(true)
    {
        if(consume_reserved(","))
        {
            node = new_node_binary(EXPR_COMMA, node, assign());
        }
        else
        {
            return node;
        }
    }
}


/*
make an assignment expression
```
assign ::= conditional (assign-op assign)?
assign-op ::= "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|="
```
*/
static Expression *assign(void)
{
    Expression *node = conditional();

    // parse assignment
    if(consume_reserved("="))
    {
        node = new_node_binary(EXPR_ASSIGN, node, assign());
    }
    else if(consume_reserved("*="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment *=");
        }

        node = new_node_binary(EXPR_MUL_EQ, lhs, rhs);
    }
    else if(consume_reserved("/="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment /=");
        }

        node = new_node_binary(EXPR_DIV_EQ, lhs, rhs);
    }
    else if(consume_reserved("%="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment %=");
        }

        node = new_node_binary(EXPR_MOD_EQ, lhs, rhs);
    }
    else if(consume_reserved("+="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(EXPR_ADD_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
        {
            node = new_node_binary(EXPR_PTR_ADD_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }
    }
    else if(consume_reserved("-="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(EXPR_SUB_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
        {
            node = new_node_binary(EXPR_PTR_SUB_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }
    }
    else if(consume_reserved("<<="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment <<=");
        }

        node = new_node_binary(EXPR_LSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved(">>="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment >>=");
        }

        node = new_node_binary(EXPR_RSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved("&="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment &=");
        }

        node = new_node_binary(EXPR_AND_EQ, lhs, rhs);
    }
    else if(consume_reserved("^="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment ^=");
        }

        node = new_node_binary(EXPR_XOR_EQ, lhs, rhs);
    }
    else if(consume_reserved("|="))
    {
        Expression *lhs = node;
        Expression *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment |=");
        }

        node = new_node_binary(EXPR_OR_EQ, lhs, rhs);
    }

    return node;
}


/*
make a conditional expression
```
conditional ::= logical-or ("?" expression ":" conditional)?
```
*/
static Expression *conditional(void)
{
    Expression *node = logical_or();

    if(consume_reserved("?"))
    {
        Expression *ternary = new_expression(EXPR_COND);

        ternary->cond = node;
        ternary->lhs = expression();
        expect_reserved(":");
        ternary->rhs = conditional();

        // copy LHS and RHS since their types may not be modified
        Expression lhs = *ternary->lhs;
        Expression rhs = *ternary->rhs;
        if(is_integer(lhs.type) && is_integer(rhs.type))
        {
            apply_arithmetic_conversion(&lhs, &rhs);
        }
        ternary->type = lhs.type;

        return ternary;
    }
    else
    {
        return node;
    }
}


/*
make a logical OR expression
```
logical-or ::= logical-and (|| logical-and)*
```
*/
static Expression *logical_or(void)
{
    Expression *node = logical_and();

    // parse tokens while finding a bitwise logical AND expression
    while(true)
    {
        if(consume_reserved("||"))
        {
            node = new_node_binary(EXPR_LOG_OR, node, logical_and());
        }
        else
        {
            return node;
        }
    }
}


/*
make a logical AND expression
```
logical-and ::= bitwise-or (&& bitwise-or)*
```
*/
static Expression *logical_and(void)
{
    Expression *node = bitwise_or();

    // parse tokens while finding a bitwise inclusive OR expression
    while(true)
    {
        if(consume_reserved("&&"))
        {
            node = new_node_binary(EXPR_LOG_AND, node, bitwise_or());
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise inclusive OR expression
```
bitwise-or ::= bitwise-xor (| bitwise-xor)*
```
*/
static Expression *bitwise_or(void)
{
    Expression *node = bitwise_xor();

    // parse tokens while finding a bitwise exclusive OR expression
    while(true)
    {
        if(consume_reserved("|"))
        {
            node = new_node_binary(EXPR_BIT_OR, node, bitwise_xor());
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise exclusive OR expression
```
bitwise-xor ::= bitwise-and (^ bitwise-and)*
```
*/
static Expression *bitwise_xor(void)
{
    Expression *node = bitwise_and();

    // parse tokens while finding a bitwise AND expression
    while(true)
    {
        if(consume_reserved("^"))
        {
            node = new_node_binary(EXPR_BIT_XOR, node, bitwise_and());
        }
        else
        {
            return node;
        }
    }
}


/*
make a bitwise AND expression
```
bitwise-and ::= equality (& equality)*
```
*/
static Expression *bitwise_and(void)
{
    Expression *node = equality();

    // parse tokens while finding a equality expression
    while(true)
    {
        if(consume_reserved("&"))
        {
            node = new_node_binary(EXPR_BIT_AND, node, equality());
        }
        else
        {
            return node;
        }
    }
}


/*
make an equality expression
```
equality ::= relational ("==" relational | "!=" relational)*
```
*/
static Expression *equality(void)
{
    Expression *node = relational();

    // parse tokens while finding a relational expression
    while(true)
    {
        if(consume_reserved("=="))
        {
            node = new_node_binary(EXPR_EQ, node, relational());
        }
        else if(consume_reserved("!="))
        {
            node = new_node_binary(EXPR_NEQ, node, relational());
        }
        else
        {
            return node;
        }
    }
}


/*
make a relational expression
```
relational ::= shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
```
*/
static Expression *relational(void)
{
    Expression *node = shift();

    // parse tokens while finding a shift expression
    while(true)
    {
        if(consume_reserved("<"))
        {
            node = new_node_binary(EXPR_L, node, shift());
        }
        else if(consume_reserved("<="))
        {
            node = new_node_binary(EXPR_LEQ, node, shift());
        }
        else if(consume_reserved(">"))
        {
            node = new_node_binary(EXPR_L, shift(), node);
        }
        else if(consume_reserved(">="))
        {
            node = new_node_binary(EXPR_LEQ, shift(), node);
        }
        else
        {
            return node;
        }
    }
}


/*
make a shift expression
```
shift ::=  additive ("<<" additive | ">>" additive)*
```
*/
static Expression *shift(void)
{
    Expression *node = additive();

    // parse tokens while finding an additive expression
    while(true)
    {
        if(consume_reserved("<<"))
        {
            Expression *lhs = node;
            Expression *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_LSHIFT, lhs, rhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator <<");
            }
        }
        else if(consume_reserved(">>"))
        {
            Expression *lhs = node;
            Expression *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_RSHIFT, lhs, rhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator >>");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make an additive expression
```
additive ::= multiplicative ("+" multiplicative | "-" multiplicative)*
```
*/
static Expression *additive(void)
{
    Expression *node = multiplicative();

    // parse tokens while finding a multiplicative expression
    while(true)
    {
        if(consume_reserved("+"))
        {
            Expression *lhs = node;
            Expression *rhs = multiplicative();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_ADD, lhs, rhs);
            }
            else if(is_pointer_or_array(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_PTR_ADD, lhs, rhs);
            }
            else if(is_integer(lhs->type) && is_pointer_or_array(rhs->type))
            {
                node = new_node_binary(EXPR_PTR_ADD, rhs, lhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator +");
            }
        }
        else if(consume_reserved("-"))
        {
            Expression *lhs = node;
            Expression *rhs = multiplicative();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_SUB, lhs, rhs);
            }
            else if(is_pointer_or_array(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(EXPR_PTR_SUB, lhs, rhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator -");
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a multiplicative expression
```
multiplicative ::= cast ("*" cast | "/" cast | "%" cast)*
```
*/
static Expression *multiplicative(void)
{
    Expression *node = cast();

    // parse tokens while finding a cast expression
    while(true)
    {
        if(consume_reserved("*"))
        {
            node = new_node_binary(EXPR_MUL, node, cast());
        }
        else if(consume_reserved("/"))
        {
            node = new_node_binary(EXPR_DIV, node, cast());
        }
        else if(consume_reserved("%"))
        {
            node = new_node_binary(EXPR_MOD, node, cast());
        }
        else
        {
            return node;
        }
    }
}


/*
make a cast expression
```
cast ::= ("(" type-name ")")? unary
```
*/
static Expression *cast(void)
{
    Expression *node;

    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_type_name())
        {
            Type *type = type_name();
            expect_reserved(")");
            node = new_expression(EXPR_CAST);
            node->lhs = unary();
            node->type = type;
            goto cast_end;
        }
        else
        {
            set_token(saved_token);
        }
    }

    node = unary();

cast_end:
    return node;
}


/*
make an unary expression
```
unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | "sizeof" unary
        | "sizeof" "(" type-name ")"
unary-op ::= "&" | "*" | "+" | "-" | "~" | "!"
```
*/
static Expression *unary(void)
{
    Expression *node;

    if(consume_reserved("sizeof"))
    {
        // The type of the result of 'sizeof' operator is 'size_t'.
        // This implementation regards 'size_t' as 'unsigned long'.
        Token *saved_token = get_token();
        if(consume_reserved("("))
        {
            if(peek_type_name())
            {
                Type *type = type_name();
                node = new_node_integer(TY_ULONG, type->size);
                expect_reserved(")");
                goto unary_end;
            }
            else
            {
                set_token(saved_token);
            }
        }

        Expression *operand = unary();
        node = new_node_integer(TY_ULONG, operand->type->size);
    }
    else if(consume_reserved("++"))
    {
        Expression *operand = unary();

        if(is_integer(operand->type))
        {
            node = new_node_binary(EXPR_ADD_EQ, operand, new_node_integer(TY_INT, 1));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(EXPR_PTR_ADD_EQ, operand, new_node_integer(TY_INT, 1));
        }
        else
        {
            report_error(NULL, "bad operand for prefix increment operator ++");
        }
    }
    else if(consume_reserved("--"))
    {
        Expression *operand = unary();

        if(is_integer(operand->type))
        {
            node = new_node_binary(EXPR_SUB_EQ, operand, new_node_integer(TY_INT, 1));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(EXPR_PTR_SUB_EQ, operand, new_node_integer(TY_INT, 1));
        }
        else
        {
            report_error(NULL, "bad operand for prefix decrement operator --");
        }
    }
    else if(consume_reserved("&"))
    {
        node = new_node_unary(EXPR_ADDR, unary());
    }
    else if (consume_reserved("*"))
    {
        node = new_node_unary(EXPR_DEREF, unary());
    }
    else if(consume_reserved("+"))
    {
        node = apply_integer_promotion(unary());
    }
    else if(consume_reserved("-"))
    {
        node = new_node_binary(EXPR_SUB, new_node_integer(TY_INT, 0), apply_integer_promotion(unary()));
    }
    else if (consume_reserved("~"))
    {
        node = new_node_unary(EXPR_COMPL, apply_integer_promotion(unary()));
    }
    else if (consume_reserved("!"))
    {
        node = new_node_unary(EXPR_NEG, unary());
    }
    else
    {
        node = postfix();
    }

unary_end:
    return node;
}


/*
make a postfix expression
```
postfix ::= primary
          | postfix "[" expression "]"
          | postfix "(" arg-expr-list? ")"
          | postfix "." identifier
          | postfix "->" identifier
          | postfix "++"
          | postfix "--"
```
*/
static Expression *postfix(void)
{
    Expression *node = primary();

    // parse tokens while finding a postfix operator
    while(true)
    {
        if(consume_reserved("["))
        {
            // array subscripting
            Expression *lhs;
            Expression *index = expression();

            if(is_pointer_or_array(node->type) && is_integer(index->type))
            {
                lhs = new_node_binary(EXPR_PTR_ADD, node, index);
            }
            else if(is_integer(node->type) && is_pointer_or_array(index->type))
            {
                lhs = new_node_binary(EXPR_PTR_ADD, index, node);
            }
            else
            {
                report_error(NULL, "bad operand for [] operator\n");
            }

            node = new_node_unary(EXPR_DEREF, lhs);
            expect_reserved("]");
        }
        else if(consume_reserved("("))
        {
            // function call
            if((node->type->base == NULL) || (node->type->base->kind != TY_FUNC))
            {
                report_error(NULL, "expected function");
            }

            Expression *func_node = new_expression(EXPR_FUNC);
            if(!consume_reserved(")"))
            {
                func_node->args = arg_expr_list();
                expect_reserved(")");
            }
            func_node->type = node->type->base->base; // dereference pointer and get type of return value
            func_node->ident = node->ident;
            node = func_node;
        }
        else if(consume_reserved("."))
        {
            // access to member (by value)
            Token *token = expect_identifier();
            Expression *struct_node = node;
            node = new_node_member(struct_node, find_member(token, struct_node->type));
        }
        else if(consume_reserved("->"))
        {
            // access to member (by pointer)
            Token *token = expect_identifier();
            Expression *struct_node = new_node_unary(EXPR_DEREF, node);
            node = new_node_member(struct_node, find_member(token, struct_node->type));
        }
        else if(consume_reserved("++"))
        {
            // postfix increment
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix increment operator ++\n");
            }
            node = new_node_unary(EXPR_POST_INC, node);
        }
        else if(consume_reserved("--"))
        {
            // postfix decrement
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix decrement operator --\n");
            }
            node = new_node_unary(EXPR_POST_DEC, node);
        }
        else
        {
            return node;
        }
    }
}


/*
make an argument expression list
```
arg-expr-list ::= assign ("," assign)*
```
*/
static Expression *arg_expr_list(void)
{
    Expression *arg;
    Expression *cursor = NULL;

    arg = assign();
    arg->next = cursor;
    cursor = arg;

    // parse arguments
    while(consume_reserved(","))
    {
        // append the argument at the head in order to push arguments in reverse order when generating assembler code
        arg = assign();
        arg->next = cursor;
        cursor = arg;
    }

    return cursor;
}


/*
make a primary expression
```
primary ::= identifier
          | integer-constant
          | string-literal
          | "(" expression ")"
```
*/
static Expression *primary(void)
{
    // expression in brackets
    if(consume_reserved("("))
    {
        Expression *node = expression();
 
        expect_reserved(")");
 
        return node;
    }

    // identifier
    Token *token;
    if(consume_token(TK_IDENT, &token))
    {
        Identifier *ident = find_identifier(token);
        if(ident != NULL)
        {
            if(ident->var != NULL)
            {
                // variable
                Variable *var = ident->var;
                if(var->type->kind == TY_FUNC)
                {
                    Expression *node = new_expression(EXPR_FUNC);
                    node->type = new_type_pointer(var->type);
                    node->ident = make_identifier(token);
                    return node;
                }
                else
                {
                    Expression *node = new_expression(EXPR_VAR);
                    node->type = var->type;
                    node->var = var;
                    return node;
                }
            }
            else if(ident->en != NULL)
            {
                // enumeration
                return new_node_integer(TY_INT, ident->en->value);
            }
        }

        if(peek_reserved("("))
        {
            // implicitly assume that the token denotes a function which returns int
            Expression *node = new_expression(EXPR_FUNC);
            node->type = new_type_pointer(new_type_function(new_type(TY_INT, TQ_NONE), new_type(TY_VOID, TQ_NONE)));
            node->ident = make_identifier(token);
#if(WARN_IMPLICIT_DECLARATION_OF_FUNCTION == ENABLED)
            report_warning(token->str, "implicit declaration of function '%s'\n", make_identifier(token));
#endif /* WARN_IMPLICIT_DECLARATION_OF_FUNCTION */
            return node;
        }

        report_error(token->str, "undefined identifier '%s'", make_identifier(token));
    }

    // string-literal
    if(consume_token(TK_STR, &token))
    {
        Expression *node = new_expression(EXPR_VAR);
        node->var = new_string(token);
        node->type = node->var->type;
        return node;
    }

    // integer-constant
    token = expect_integer_constant();
    TypeKind kind;
    long value = parse_integer_constant(token, &kind);
    return new_node_integer(kind, value);
}


/*
make a new expression
*/
static Expression *new_expression(ExpressionKind kind)
{
    Expression *node = calloc(1, sizeof(Expression));
    node->next = NULL;
    node->kind = kind;
    node->lhs = NULL;
    node->rhs = NULL;
    node->type = NULL;
    node->value = 0;
    node->var = NULL;
    node->cond = NULL;
    node->ident = NULL;
    node->args = NULL;

    return node;
}


/*
make a new statement
*/
static Statement *new_statement(StatementKind kind)
{
    Statement *node = calloc(1, sizeof(Statement));
    node->next = NULL;
    node->kind = kind;
    node->value = 0;
    node->cond = NULL;
    node->preexpr = NULL;
    node->postexpr = NULL;
    node->body = NULL;
    node->ident = NULL;

    return node;
}


/*
make a new node for unary operations
*/
static Expression *new_node_unary(ExpressionKind kind, Expression *lhs)
{
    Expression *node = new_expression(kind);
    node->lhs = lhs;

    switch(kind)
    {
    case EXPR_ADDR:
        node->type = new_type_pointer(lhs->type);
        break;

    case EXPR_DEREF:
        node->type = lhs->type->base;
        break;

    case EXPR_POST_INC:
    case EXPR_POST_DEC:
        node->type = lhs->type;
        break;

    case EXPR_NEG:
        node->type = new_type(TY_INT, TQ_NONE);
        break;

    case EXPR_COMPL:
    default:
        break;
    }

    return node;
}


/*
make a new node for binary operations
*/
static Expression *new_node_binary(ExpressionKind kind, Expression *lhs, Expression *rhs)
{
    Expression *node = new_expression(kind);

    switch(kind)
    {
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    case EXPR_L:
    case EXPR_LEQ:
    case EXPR_BIT_AND:
    case EXPR_BIT_XOR:
    case EXPR_BIT_OR:
        apply_arithmetic_conversion(lhs, rhs);
        node->type = lhs->type;
        break;

    case EXPR_LSHIFT:
    case EXPR_RSHIFT:
        lhs = apply_integer_promotion(lhs);
        rhs = apply_integer_promotion(rhs);
        node->type = lhs->type;
        break;

    case EXPR_EQ:
    case EXPR_NEQ:
        if(is_integer(lhs->type) && is_integer(rhs->type))
        {
            apply_arithmetic_conversion(lhs, rhs);
        }
        node->type = new_type(TY_INT, TQ_NONE);
        break;

    case EXPR_PTR_ADD:
    case EXPR_PTR_SUB:
        node->type = lhs->type;
        break;

    case EXPR_ASSIGN:
        if(is_array(rhs->type))
        {
            // convert from array to pointer
            node->type = new_type_pointer(rhs->type->base);
        }
        else
        {
            node->type = lhs->type;
        }
        break;

    case EXPR_ADD_EQ:
    case EXPR_PTR_ADD_EQ:
    case EXPR_SUB_EQ:
    case EXPR_PTR_SUB_EQ:
    case EXPR_MUL_EQ:
    case EXPR_DIV_EQ:
    case EXPR_MOD_EQ:
    case EXPR_LSHIFT_EQ:
    case EXPR_RSHIFT_EQ:
    case EXPR_AND_EQ:
    case EXPR_XOR_EQ:
    case EXPR_OR_EQ:
        node->type = lhs->type;
        break;

    case EXPR_COMMA:
        node->type = rhs->type;
        break;

    case EXPR_LOG_AND:
    case EXPR_LOG_OR:
    default:
        node->type = new_type(TY_INT, TQ_NONE);
        break;
    }

    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}


/*
make a new node for subscripting
*/
static Expression *new_node_subscript(Expression *base, size_t index)
{
    Expression *addr = new_node_binary(EXPR_PTR_ADD, base, new_node_integer(TY_ULONG, index));
    Expression *dest = new_node_unary(EXPR_DEREF, addr);

    return dest;
}


/*
make a new node for members of structure or union
*/
static Expression *new_node_member(Expression *lhs, Member *member)
{
    Expression *node = new_expression(EXPR_MEMBER);
    node->member = member;
    node->type = member->type;
    node->lhs = lhs;

    return node;
}


/*
make a new node for integer-constant
*/
static Expression *new_node_integer(TypeKind kind, long value)
{
    Expression *node = new_expression(EXPR_CONST);
    node->type = new_type(kind, TQ_NONE);
    node->value = value;

    return node;
}


/*
apply integer promotion
* In this implementation, integer promotions convert 'char' and 'short' (regardless of sign) to 'int' because
    * The size of 'char' is 1 byte.
    * The size of 'short' is 2 bytes.
    * The size of 'int' is 4 bytes.
    * Therefore, 'int' can represent 'char', 'unsigned char', 'short' and 'unsigned short'.
*/
static Expression *apply_integer_promotion(Expression *expr)
{
    if((expr->type->kind == TY_CHAR)
    || (expr->type->kind == TY_UCHAR)
    || (expr->type->kind == TY_SHORT)
    || (expr->type->kind == TY_USHORT))
    {
        expr->type = new_type(TY_INT, TQ_NONE);
    }

    return expr;
}


/*
apply usual arithmetic conversion
*/
static void apply_arithmetic_conversion(Expression *lhs, Expression *rhs)
{
    // perform integer promotions on both operands at first
    lhs = apply_integer_promotion(lhs);
    rhs = apply_integer_promotion(rhs);

    if(lhs->type->kind == rhs->type->kind)
    {
        // If both operands have the same type, then no further conversion is needed.
    }
    else
    {
        if((is_signed(lhs->type) && is_signed(rhs->type)) || (is_unsigned(lhs->type) && is_unsigned(rhs->type)))
        {
            // Otherwise, if both operands have signed integer types or both have unsigned integer types, the operand with the type of lesser integer conversion rank is converted to the type of the operand with greater rank.
            if(lhs->type->kind < rhs->type->kind)
            {
                lhs->type = rhs->type;
            }
            else
            {
                rhs->type = lhs->type;
            }
        }
        else
        {
            // Otherwise, if the operand that has unsigned integer type has rank greater or equal to the rank of the type of the other operand, then the operand with signed integer type is converted to the type of the operand with unsigned integer type.
            if(is_unsigned(lhs->type) && is_signed(rhs->type) && (get_conversion_rank(lhs->type) >= get_conversion_rank(rhs->type)))
            {
                rhs->type = lhs->type;
            }
            else if(is_unsigned(rhs->type) && is_signed(lhs->type) && (get_conversion_rank(rhs->type) >= get_conversion_rank(lhs->type)))
            {
                lhs->type = rhs->type;
            }
            // Otherwise, if the type of the operand with signed integer type can represent all of the values of the type of the operand with unsigned integer type, then the operand with unsigned integer type is converted to the type of the operand with signed integer type.
            // In this implementation, this rule can be checked only by comparing integer conversion ranks of operands.
            else if(is_signed(lhs->type) && is_unsigned(rhs->type) && (get_conversion_rank(lhs->type) > get_conversion_rank(rhs->type)))
            {
                rhs->type = lhs->type;
            }
            else if(is_signed(rhs->type) && is_unsigned(lhs->type) && (get_conversion_rank(rhs->type) > get_conversion_rank(lhs->type)))
            {
                lhs->type = rhs->type;
            }
            // Otherwise, both operands are converted to the unsigned integer type corresponding to the type of the operand with signed integer type.
            else
            {
                if(is_signed(lhs->type))
                {
                    lhs->type = discard_sign(lhs->type);
                    rhs->type = lhs->type;
                }
                else
                {
                    rhs->type = discard_sign(rhs->type);
                    lhs->type = rhs->type;
                }
            }
        }
    }
}


/*
make a new variable
*/
static Variable *new_var(const char *name, Type *type, bool local)
{
    Variable *var = calloc(1, sizeof(Variable));
    var->next = NULL;
    var->name = name;
    var->type = type;
    var->init = NULL;
    var->local = local;
    var->offset = 0;
    var->content = NULL;
    var->entity = false;

    push_identifier_scope(var->name)->var = var;

    return var;
}


/*
make a new global variable
*/
static Variable *new_gvar(const Token *token, Type *type, bool entity)
{
    Variable *gvar = new_var(make_identifier(token), type, false);
    gvar->entity = entity;
    gvar_list->next = gvar;
    gvar_list = gvar;

    return gvar;
}


/*
make a new string-literal
* String-literal is regarded as a global variable.
*/
static Variable *new_string(const Token *token)
{
    char *label = new_string_label();
    Variable *gvar = new_var(label, new_type_array(new_type(TY_CHAR, TQ_CONST), token->len + 1), false);
    gvar->content = calloc(token->len + 1, sizeof(char));
    strncpy(gvar->content, token->str, token->len);
    gvar->data = new_data_segment();
    gvar->data->label = label;
    gvar->entity = true;
    gvar_list->next = gvar;
    gvar_list = gvar;

    return gvar;
}


/*
make a new local variable
*/
static Variable *new_lvar(const Token *token, Type *type)
{
    Variable *lvar = new_var(make_identifier(token), type, true);
    lvar->next = lvar_list;
    lvar_list = lvar;

    return lvar;
}


/*
make a new function
*/
static Function *new_function(const Token *token, Type *type, Variable *args, Statement *body)
{
    Function *new_func = calloc(1, sizeof(Function));
    new_func->name = make_identifier(token);
    new_func->type = new_type_function(type->base, type->args);
    new_func->body = body;

    // set offset of arguments and local variables and accumulate stack size
    size_t offset = 0;
    if(type->args->kind != TY_VOID)
    {
        for(Variable *arg = args; arg != NULL; arg = arg->next)
        {
            offset = adjust_alignment(offset, arg->type->align);
            offset += arg->type->size;
            arg->offset = offset;
        }
    }
    for(Variable *lvar = lvar_list; lvar != NULL; lvar = lvar->next)
    {
        offset = adjust_alignment(offset, lvar->type->align);
        offset += lvar->type->size;
        lvar->offset = offset;
    }
    new_func->stack_size = adjust_alignment(offset, STACK_ALIGNMENT);;

    // save list of arguments and list of local variables
    new_func->args = args;
    new_func->locals = lvar_list;

    // update list of functions
    function_list->next = new_func;
    function_list = new_func;

    return new_func;
}


/*
make a new label for string-literal
*/
static char *new_string_label(void)
{
    // A label for string-literal is of the form "LS<number>", so the length of buffer should be more than 2 + 10 + 1.
    char *label = calloc(15, sizeof(char));

    sprintf(label, "Lstring%d", str_number);
    str_number++;

    return label;
}


/*
check if a given node is a string-literal
*/
static bool is_string(const Expression *expr)
{
    return (expr->var != NULL) && (expr->var->content != NULL);
}


/*
make a new enumerator
*/
static Enumerator *new_enumerator(const char *name, int value)
{
    Enumerator *en = calloc(1, sizeof(Enumerator));
    en->name = name;
    en->value = value;

    push_identifier_scope(en->name)->en = en;

    return en;
}


/*
push an identifier to the current scope
*/
static Identifier *push_identifier_scope(const char *name)
{
    Identifier *ident = calloc(1, sizeof(Identifier));
    ident->next = current_scope.ident_list;
    ident->name = name;
    ident->var = NULL;
    ident->en = NULL;
    ident->depth = current_scope.depth;
    current_scope.ident_list = ident;

    return ident;
}


/*
push a tag to the current scope
*/
static Tag *push_tag_scope(const char *name)
{
    Tag *tag = calloc(1, sizeof(Tag));
    tag->next = current_scope.tag_list;
    tag->name = name;
    tag->type = NULL;
    tag->depth = current_scope.depth;
    current_scope.tag_list = tag;

    return tag;
}


/*
enter a new scope
*/
static Scope enter_scope(void)
{
    current_scope.depth++;
    return current_scope;
}


/*
leave the current scope
*/
static void leave_scope(Scope scope)
{
    current_scope = scope;
    current_scope.depth--;
}


/*
find an ordinary identifier in the current scope
* If there exists a visible identifier with a given token, this function returns the identifier.
* Otherwise, it returns NULL.
*/
static Identifier *find_identifier(const Token *token)
{
    // search list of ordinary identifiers visible in the current scope
    for(Identifier *ident = current_scope.ident_list; ident != NULL; ident = ident->next)
    {
        if((strlen(ident->name) == token->len) && (strncmp(token->str, ident->name, token->len) == 0))
        {
            return ident;
        }
    }

    return NULL;
}


/*
find a tag in the current scope
* If there exists a visible tag with a given token, this function returns the tag.
* Otherwise, it returns NULL.
*/
static Tag *find_tag(const Token *token)
{
    // search list of tags visible in the current scope
    for(Tag *tag = current_scope.tag_list; tag != NULL; tag = tag->next)
    {
        if((strlen(tag->name) == token->len) && (strncmp(token->str, tag->name, token->len) == 0))
        {
            return tag;
        }
    }

    return NULL;
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
static DataSegment *new_data_segment(void)
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
parse an integer-constant
*/
static long parse_integer_constant(const Token *token, TypeKind *kind)
{
    long value = strtol(token->str, NULL, 10);
    if(errno != ERANGE)
    {
        if((INT_MIN <= value) && (value <= INT_MAX))
        {
            *kind = TY_INT;
        }
        else
        {
            *kind = TY_LONG;
        }
    }
    else
    {
        report_warning(token->str, "integer constant is too large");
        *kind = TY_LONG;
    }

    return value;
}


/*
peek declaration-specifiers
*/
static bool peek_declaration_specifiers(void)
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
static bool peek_type_name(void)
{
    return peek_type_specifier() || peek_type_qualifier();
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
peek a function
*/
static bool peek_func(void)
{
    // save the currently parsing token and the current scope
    // since the parser twice reads declaration specifiers and/or declarators in external definitions
    Token *saved_token = get_token();
    Scope scope = current_scope;

    // parse declaration specifier and declarator
    StorageClassSpecifier sclass;
    Type *type = declaration_specifiers(&sclass);
    Token *token;
    type = declarator(type, &token, NULL);

    // check if a compound statement follows
    bool is_func = consume_reserved("{");

    // restore the saved token and scope
    set_token(saved_token);
    current_scope = scope;

    if(is_func)
    {
        // make a function declarator
        new_gvar(token, new_type_function(type->base, type->args), false);
    }

    return is_func;
}


/*
make a constant expression
```
const-expression ::= conditional
```
*/
static long const_expression(void)
{
    return evaluate(conditional());
}


/*
evaluate a node
*/
static long evaluate(Expression *expr)
{
    long result = 0;

    switch(expr->kind)
    {
    case EXPR_COMPL:
        result = ~evaluate(expr);
        break;

    case EXPR_NEG:
        result = !evaluate(expr);
        break;

    case EXPR_ADD:
        result = evaluate(expr->lhs) + evaluate(expr->rhs);
        break;

    case EXPR_SUB:
        result = evaluate(expr->lhs) - evaluate(expr->rhs);
        break;

    case EXPR_MUL:
        result = evaluate(expr->lhs) * evaluate(expr->rhs);
        break;

    case EXPR_DIV:
        result = evaluate(expr->lhs) / evaluate(expr->rhs);
        break;

    case EXPR_MOD:
        result = evaluate(expr->lhs) % evaluate(expr->rhs);
        break;

    case EXPR_LSHIFT:
        result = evaluate(expr->lhs) << evaluate(expr->rhs);
        break;

    case EXPR_RSHIFT:
        result = evaluate(expr->lhs) >> evaluate(expr->rhs);
        break;

    case EXPR_EQ:
        result = (evaluate(expr->lhs) == evaluate(expr->rhs));
        break;

    case EXPR_NEQ:
        result = (evaluate(expr->lhs) != evaluate(expr->rhs));
        break;

    case EXPR_L:
        result = (evaluate(expr->lhs) < evaluate(expr->rhs));
        break;

    case EXPR_LEQ:
        result = (evaluate(expr->lhs) <= evaluate(expr->rhs));
        break;

    case EXPR_BIT_AND:
        result = (evaluate(expr->lhs) & evaluate(expr->rhs));
        break;

    case EXPR_BIT_XOR:
        result = (evaluate(expr->lhs) ^ evaluate(expr->rhs));
        break;

    case EXPR_BIT_OR:
        result = (evaluate(expr->lhs) | evaluate(expr->rhs));
        break;

    case EXPR_LOG_AND:
        result = (evaluate(expr->lhs) && evaluate(expr->rhs));
        break;

    case EXPR_LOG_OR:
        result = (evaluate(expr->lhs) || evaluate(expr->rhs));
        break;

    case EXPR_CONST:
        result = expr->value;
        break;

    default:
        report_error(NULL, "cannot evaluate");
        break;
    }

    return result;
}


/*
adjust alignment
*/
static size_t adjust_alignment(size_t target, size_t alignment)
{
    return (target + (alignment - 1)) & ~(alignment - 1);
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
                type->qual = qual;
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
