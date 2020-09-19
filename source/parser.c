/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* top level parser
* management of identifiers
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declaration.h"
#include "generator.h"
#include "parser.h"
#include "statement.h"
#include "tokenizer.h"
#include "type.h"

// definition of list operations
#include "list.h"
define_list_operations(Function)
define_list_operations(Identifier)
define_list_operations(StringLiteral)
define_list_operations(Tag)
define_list_operations(Variable)

// function prototype
static Function *new_function(const Token *token, Type *type, StorageClassSpecifier sclass, List(Variable) *args, List(Statement) *body);
static char *new_string_label(void);
static void program(void);
static void function_def(void);
static bool peek_func(void);


// global variable
const size_t STACK_ALIGNMENT = 8; // alignment of function stack
const size_t REGISTER_SAVE_AREA_SIZE = 48; // size of register save area
static int str_number = 0; // label number of string-literal
static List(StringLiteral) *str_list = NULL; // list of string-literals
static List(Function) *function_list = NULL; // list of functions
static List(Variable) *gvar_list = NULL; // list of global variables
static List(Variable) *lvar_list = NULL; // list of local variables of currently constructing function
static Scope current_scope = {NULL, NULL, NULL, NULL, 0}; // current scope


/*
make a new variable
*/
Variable *new_var(const char *name, Type *type, size_t align, StorageClassSpecifier sclass, bool local)
{
    Variable *var = calloc(1, sizeof(Variable));
    var->name = name;
    var->type = type;
    var->inits = NULL;
    var->str = NULL;
    var->align = align;
    var->offset = 0;
    var->sclass = sclass;
    var->local = local;
    var->entity = false;

    push_identifier_scope(var->name)->var = var;

    return var;
}


/*
make a new global variable
*/
Variable *new_gvar(const char *name, Type *type, size_t align, StorageClassSpecifier sclass, bool entity)
{
    Variable *gvar = new_var(name, type, align, sclass, false);
    gvar->entity = entity;
    add_list_entry_tail(Variable)(gvar_list, gvar);

    return gvar;
}


/*
make a new local variable
*/
Variable *new_lvar(const char *name, Type *type, size_t align, StorageClassSpecifier sclass)
{
    Variable *lvar = new_var(name, type, align, sclass, true);
    add_list_entry_head(Variable)(lvar_list, lvar);

    return lvar;
}


/*
make a new string-literal
* String-literal is regarded as a global variable.
*/
StringLiteral *new_string(const Token *token)
{
    // search existing string-literals
    for_each_entry(StringLiteral, cursor, str_list)
    {
        StringLiteral *str = get_element(StringLiteral)(cursor);
        if((strlen(str->content) == token->len) && (strncmp(str->content, token->str, token->len) == 0))
        {
            return str;
        }
    }

    // make a string-literal (an array of char terminated by '\0')
    char *content = calloc(token->len + 1, sizeof(char));
    strncpy(content, token->str, token->len);

    char *label = new_string_label();
    Type *type = new_type_array(new_type(TY_CHAR, TQ_NONE), token->len + 1);

    Variable *gvar = new_var(label, type, ALIGNOF_PTR, SC_STATIC, false);
    gvar->str = calloc(1, sizeof(StringLiteral));
    gvar->inits = new_list(InitializerMap)();
    add_list_entry_tail(InitializerMap)(gvar->inits, new_string_initializer_map(label, 0));
    gvar->entity = true;
    add_list_entry_tail(Variable)(gvar_list, gvar);

    StringLiteral *str = gvar->str;
    str->content = content;
    str->var = gvar;
    add_list_entry_tail(StringLiteral)(str_list, str);

    return str;
}


/*
make a new function
*/
static Function *new_function(const Token *token, Type *type, StorageClassSpecifier sclass, List(Variable) *args, List(Statement) *body)
{
    Function *new_func = calloc(1, sizeof(Function));
    new_func->name = make_identifier(token);
    new_func->type = type;
    new_func->body = body;
    new_func->sclass = sclass;

    // set offset of arguments and local variables and accumulate stack size
    size_t offset = (type->variadic ? REGISTER_SAVE_AREA_SIZE + STACK_ALIGNMENT : 0);
    for_each_entry(Variable, cursor, args)
    {
        Variable *arg = get_element(Variable)(cursor);
        offset = adjust_alignment(offset, arg->align);
        offset += (is_struct_or_union(arg->type) ? adjust_alignment(arg->type->size, STACK_ALIGNMENT) : arg->type->size);
        arg->offset = offset;
    }
    for_each_entry(Variable, cursor, lvar_list)
    {
        Variable *lvar = get_element(Variable)(cursor);
        offset = adjust_alignment(offset, lvar->align);
        offset += lvar->type->size;
        lvar->offset = offset;
    }
    new_func->stack_size = adjust_alignment(offset, STACK_ALIGNMENT);

    // save list of arguments and list of local variables
    new_func->args = args;
    new_func->locals = lvar_list;

    // update list of functions
    add_list_entry_tail(Function)(function_list, new_func);

    return new_func;
}


/*
make a new label for string-literal
*/
static char *new_string_label(void)
{
    // A label for string-literal is of the form "Lstring<number>", so the length of buffer should be more than 7 + 10.
    char *label = calloc(18, sizeof(char));

    sprintf(label, "Lstring%d", str_number);
    str_number++;

    return label;
}


/*
make a program
```
program ::= (declaration | function-def)*
```
*/
static void program(void)
{
    str_list = new_list(StringLiteral)();
    function_list = new_list(Function)();
    gvar_list = new_list(Variable)();
    current_scope.ident_list = new_list(Identifier)();
    current_scope.tag_list = new_list(Tag)();

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
    lvar_list = new_list(Variable)();

    // save the current scope
    Scope scope = enter_scope();

    // parse declaration specifier and declarator
    StorageClassSpecifier sclass;
    Type *type = declaration_specifiers(NULL, &sclass);
    Token *token;
    List(Variable) *args = new_list(Variable)();
    type = declarator(type, &token, &args);

    // parse body
    List(Statement) *body = compound_statement();

    // restore the scope
    leave_scope(scope);

    // make a new function
    new_function(token, type, sclass, args, body);
}


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
push an identifier to the current scope
*/
Identifier *push_identifier_scope(const char *name)
{
    Identifier *ident = calloc(1, sizeof(Identifier));
    ident->name = name;
    ident->var = NULL;
    ident->en = NULL;
    ident->type_def = NULL;
    char *punctuator = strstr(name, STATIC_VARIABLE_PUNCTUATOR);
    ident->len = ((punctuator == NULL) ? strlen(name) : punctuator - name);
    ident->depth = current_scope.depth;
    add_list_entry_head(Identifier)(current_scope.ident_list, ident);

    return ident;
}


/*
push a tag to the current scope
*/
Tag *push_tag_scope(const char *name, Type *type)
{
    Tag *tag = calloc(1, sizeof(Tag));
    tag->name = name;
    tag->type = type;
    tag->len = strlen(name);
    tag->depth = current_scope.depth;
    add_list_entry_head(Tag)(current_scope.tag_list, tag);

    return tag;
}


/*
enter a new scope
*/
Scope enter_scope(void)
{
    current_scope.depth++;
    current_scope.ident_head = get_first_entry(Identifier)(current_scope.ident_list);
    current_scope.tag_head = get_first_entry(Tag)(current_scope.tag_list);
    return current_scope;
}


/*
leave the current scope
*/
void leave_scope(Scope scope)
{
    current_scope = scope;
    set_first_entry(Identifier)(current_scope.ident_list, current_scope.ident_head);
    set_first_entry(Tag)(current_scope.tag_list, current_scope.tag_head);
    current_scope.depth--;
}


/*
get depth of the current scope
*/
int get_current_scope_depth(void)
{
    return current_scope.depth;
}


/*
find an ordinary identifier in the current scope
* If there exists a visible identifier with a given token, this function returns the identifier.
* Otherwise, it returns NULL.
*/
Identifier *find_identifier(const Token *token)
{
    // search list of ordinary identifiers visible in the current scope
    for_each_entry(Identifier, cursor, current_scope.ident_list)
    {
        Identifier *ident = get_element(Identifier)(cursor);
        if((ident->len == token->len) && (strncmp(token->str, ident->name, token->len) == 0))
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
Tag *find_tag(const Token *token)
{
    // search list of tags visible in the current scope
    for_each_entry(Tag, cursor, current_scope.tag_list)
    {
        Tag *tag = get_element(Tag)(cursor);
        if((tag->len == token->len) && (strncmp(token->str, tag->name, token->len) == 0))
        {
            return tag;
        }
    }

    return NULL;
}


/*
peek a function
*/
static bool peek_func(void)
{
    // save the currently parsing token and the current scope
    // since the parser twice reads declaration specifiers and/or declarators in external definitions
    Token *saved_token = get_token();
    Scope scope = enter_scope();

    // parse declaration specifier and declarator
    size_t align;
    StorageClassSpecifier sclass;
    Type *type = declaration_specifiers(&align, &sclass);
    Token *token;
    type = declarator(type, &token, NULL);

    // check if a compound statement follows
    bool is_func = consume_reserved("{");

    // restore the saved token and scope
    set_token(saved_token);
    leave_scope(scope);

    if(is_func)
    {
        // make a function declarator
        new_gvar(make_identifier(token), type, type->align, sclass, false);
    }

    return is_func;
}
