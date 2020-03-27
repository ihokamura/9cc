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
#define SPEC_LIST_SIZE ((size_t)7) // number of valid specifiers


// type definition
typedef enum {
    SP_VOID,     // "void"
    SP_CHAR,     // "char"
    SP_SHORT,    // "short"
    SP_INT,      // "int"
    SP_LONG,     // "long"
    SP_SIGNED,   // "signed"
    SP_UNSIGNED, // "unsigned"
    SP_INVALID,  // invalid specifier
} SpecifierKind;

typedef struct VarScope VarScope;
struct VarScope {
    VarScope *next; // next element
    Variable *var;  // visible variable
    int depth;      // depth of scope
};


// function prototype
static void program(void);
static void function_def(void);
static Type *declaration_specifiers(void);
static Type *declarator(Type *type, Token **token);
static Type *parameter_list(Variable **arg_vars);
static Type *parameter_declaration(Variable **arg_var);
static Type *type_name(void);
static SpecifierKind type_specifier(void);
static Type *pointer(Type *base);
static Type *direct_declarator(Type *type, Token **token);
static Node *statement(void);
static Node *compound_statement(void);
static Node *declaration(bool is_local);
static Node *init_declarator_list(Type *type, bool is_local);
static Node *init_declarator(Type *type, bool is_local);
static Node *initializer(void);
static Node *expression(void);
static Node *assign(void);
static Node *conditional(void);
static Node *logical_or(void);
static Node *logical_and(void);
static Node *bitwise_or(void);
static Node *bitwise_xor(void);
static Node *bitwise_and(void);
static Node *equality(void);
static Node *relational(void);
static Node *shift(void);
static Node *additive(void);
static Node *multiplicative(void);
static Node *cast(void);
static Node *unary(void);
static Node *postfix(void);
static Node *arg_expr_list(void);
static Node *primary(void);
static Node *new_node(NodeKind kind);
static Node *new_node_unary(NodeKind kind, Node *lhs);
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_integer(IntegerConstant val);
static Node *apply_integer_promotion(Node *node);
static void apply_arithmetic_conversion(Node *lhs, Node *rhs);
static Variable *new_var(char *name, Type *type, bool local);
static Variable *new_gvar(const Token *token, Type *type, bool entity);
static Variable *new_string(const Token *token);
static Variable *new_lvar(const Token *token, Type *type);
static Function *new_function(const Token *token, Type *type, Node *body);
static VarScope *enter_scope(void);
static void leave_scope(VarScope *scope);
static void push_scope(Variable *var);
static VarScope *find_scope(const Token *token);
static Type *determine_type(void);
static IntegerConstant parse_integer_constant(const Token *token);
static void clear_current_spec_list(void);
static bool peek_type_specifier(void);
static bool peek_func(void);
static char *new_string_label(void);
static IntegerConstant const_expression(void);
static IntegerConstant evaluate(Node *node);
static bool is_zero(IntegerConstant val);


// global variable
static int str_number = 0; // label number of string-literal
static Function *function_list; // list of functions
static Variable *gvar_list = NULL; // list of global variables
static Variable *args_list = NULL; // list of arguments of currently constructing function
static Variable *lvar_list = NULL; // list of local variables of currently constructing function
static const size_t stack_alignment_size = 8; // alignment size of function stack
static Node *current_switch = NULL; // currently parsing switch statement
static VarScope *var_scope = NULL; // list of variables visible in the current scope
static int scope_depth = 0; // depth of the current scope
static int current_spec_list[SPEC_LIST_SIZE] = {}; // list of currently parsing specifiers
static const struct {int spec_list[SPEC_LIST_SIZE]; TypeKind type_kind;} TYPE_SPECS_MAP[] = {
    // synonym of 'void'
    {{1, 0, 0, 0, 0, 0, 0}, TY_VOID},   // void
    // synonym of 'char'
    {{0, 1, 0, 0, 0, 0, 0}, TY_CHAR},   // char
    // synonym of 'signed char'
    {{0, 1, 0, 0, 0, 1, 0}, TY_CHAR},   // signed char
    // synonym of 'unsigned char'
    {{0, 1, 0, 0, 0, 0, 1}, TY_UCHAR},  // unsigned char
    // synonym of 'short'
    {{0, 0, 1, 0, 0, 0, 0}, TY_SHORT},  // short
    {{0, 0, 1, 0, 0, 1, 0}, TY_SHORT},  // signed short
    {{0, 0, 1, 1, 0, 0, 0}, TY_SHORT},  // short int
    {{0, 0, 1, 1, 0, 1, 0}, TY_SHORT},  // signed short int
    // synonym of 'unsigned short'
    {{0, 0, 1, 0, 0, 0, 1}, TY_USHORT}, // unsigned short
    {{0, 0, 1, 1, 0, 0, 1}, TY_USHORT}, // unsigned short int
    // synonym of 'int'
    {{0, 0, 0, 1, 0, 0, 0}, TY_INT},    // int
    {{0, 0, 0, 0, 0, 1, 0}, TY_INT},    // signed
    {{0, 0, 0, 1, 0, 1, 0}, TY_INT},    // signed int
    // synonym of 'unsigned'
    {{0, 0, 0, 0, 0, 0, 1}, TY_UINT},   // unsigned
    {{0, 0, 0, 1, 0, 0, 1}, TY_UINT},   // unsigned int
    // synonym of 'long'
    {{0, 0, 0, 0, 1, 0, 0}, TY_LONG},   // long
    {{0, 0, 0, 0, 1, 1, 0}, TY_LONG},   // signed long
    {{0, 0, 0, 1, 1, 0, 0}, TY_LONG},   // long int
    {{0, 0, 0, 1, 1, 1, 0}, TY_LONG},   // signed long int
    // synonym of 'unsigned long'
    {{0, 0, 0, 0, 1, 0, 1}, TY_ULONG},  // unsigned long
    {{0, 0, 0, 1, 1, 0, 1}, TY_ULONG},  // unsigned long int
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

    // parse declaration specifier and declarator
    Type *type = declaration_specifiers();
    Token *token;
    type = declarator(type, &token);

    // make a function declarator
    new_gvar(token, new_type_function(type->base, type->args), false);

    // parse body
    Node *body = compound_statement();

    // make a new function
    new_function(token, type, body);
}


/*
make a declaration specifier
```
declaration-specifiers ::= type-specifier type-specifier*
```
*/
static Type *declaration_specifiers(void)
{
    clear_current_spec_list();
    current_spec_list[type_specifier()]++;
    while(peek_type_specifier())
    {
        current_spec_list[type_specifier()]++;
    }

    return determine_type();
}



/*
make a declarator
```
declarator ::= pointer? direct-declarator
```
*/
static Type *declarator(Type *type, Token **token)
{
    if(peek_reserved("*"))
    {
        type = pointer(type);
    }

    type = direct_declarator(type, token);

    return type;
}


/*
make a parameterlist
```
parameter-list ::= parameter-declaration ("," parameter-declaration)*
```
*/
static Type *parameter_list(Variable **arg_vars)
{
    Type *arg_types;
    Type arg_types_head = {};
    Type *arg_types_cursor = &arg_types_head;
    Variable arg_vars_head = {};
    Variable *arg_vars_cursor = &arg_vars_head;

    arg_types_cursor->next = parameter_declaration(&arg_vars_cursor->next);
    arg_types_cursor = arg_types_cursor->next;
    arg_vars_cursor = arg_vars_cursor->next;

    while(consume_reserved(","))
    {
        arg_types_cursor->next = parameter_declaration(&arg_vars_cursor->next);
        arg_types_cursor = arg_types_cursor->next;
        arg_vars_cursor = arg_vars_cursor->next;
    }

    arg_types = arg_types_head.next;
    *arg_vars = arg_vars_head.next;

    return arg_types;
}


/*
make a parameter declaration
```
parameter-declaration ::= declaration-specifiers declarator
```
*/
static Type *parameter_declaration(Variable **arg_var)
{
    Type *arg_type = declaration_specifiers();
    Token *arg_token;

    arg_type = declarator(arg_type, &arg_token);
    *arg_var = new_var(make_identifier(arg_token), arg_type, true);

    return arg_type;
}


/*
make a type name
```
type-name ::= type-specifier type-specifier* pointer?
```
*/
static Type *type_name(void)
{
    clear_current_spec_list();
    current_spec_list[type_specifier()]++;
    while(peek_type_specifier())
    {
        current_spec_list[type_specifier()]++;
    }
    Type *type = determine_type();

    if(peek_reserved("*"))
    {
        type = pointer(type);
    }

    return type;
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
```
*/
static SpecifierKind type_specifier(void)
{
    if(consume_reserved("void"))
    {
        return SP_VOID;
    }
    else if(consume_reserved("char"))
    {
        return SP_CHAR;
    }
    else if(consume_reserved("short"))
    {
        return SP_SHORT;
    }
    else if(consume_reserved("int"))
    {
        return SP_INT;
    }
    else if(consume_reserved("long"))
    {
        return SP_LONG;
    }
    else if(consume_reserved("signed"))
    {
        return SP_SIGNED;
    }
    else if(consume_reserved("unsigned"))
    {
        return SP_UNSIGNED;
    }
    else
    {
        report_error(NULL, "invalid type specifier\n");
        return SP_INVALID;
    }
}


/*
make a pointer
```
pointer ::= "*" "*"*
```
*/
static Type *pointer(Type *base)
{
    Type *type = base;

    expect_reserved("*");
    type = new_type_pointer(type);

    while(consume_reserved("*"))
    {
        type = new_type_pointer(type);
    }

    return type;
}


/*
make a direct declarator
```
direct-declarator ::= identifier
                    | direct-declarator "[" const-expression "]"
                    | direct-declarator "(" ("void" | parameter-list)? ")"
```
*/
static Type *direct_declarator(Type *type, Token **token)
{
    if(consume_token(TK_IDENT, token))
    {
        type = direct_declarator(type, token);
    }
    else
    {
        if(consume_reserved("["))
        {
            IntegerConstant len = const_expression();
            size_t len_val;
            switch(len.kind)
            {
            case TY_ULONG:
                len_val = len.ulong_val;
                break;

            case TY_LONG:
                len_val = len.long_val;
                break;

            case TY_UINT:
                len_val = len.uint_val;
                break;

            default:
                len_val = len.int_val;
                break;
            }

            expect_reserved("]");
            type = direct_declarator(type, token);
            type = new_type_array(type, len_val);
        }
        else if(consume_reserved("("))
        {
            // parse arguments
            Type *arg_types = new_type(TY_VOID);
            Variable *arg_vars = NULL;
            if(!consume_reserved(")"))
            {
                if(!consume_reserved("void"))
                {
                    arg_types = parameter_list(&arg_vars);
                }
                expect_reserved(")");
            }

            type = direct_declarator(type, token);
            type = new_type_function(type, arg_types);
            args_list = arg_vars;
        }
    }

    return type;
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
static Node *statement(void)
{
    Node *node;

    if(peek_reserved("{"))
    {
        node = new_node(ND_BLOCK);
        node->body = compound_statement();
    }
    else if(consume_reserved("break"))
    {
        node = new_node(ND_BREAK);
        expect_reserved(";");
    }
    else if(consume_reserved("case"))
    {
        // parse label expression
        IntegerConstant val = const_expression();
        expect_reserved(":");

        // parse statement for the case label
        node = new_node(ND_CASE);
        node->lhs = statement();

        // save the value of label expression and update node of currently parsing switch statement
        node->val = val;
        node->next_case = current_switch->next_case;
        current_switch->next_case = node;
    }
    else if(consume_reserved("continue"))
    {
        node = new_node(ND_CONTINUE);
        expect_reserved(";");
    }
    else if(consume_reserved("default"))
    {
        expect_reserved(":");

        // parse statement for the default label
        node = new_node(ND_CASE);
        node->lhs = statement();

        // update node of currently parsing switch statement
        current_switch->default_case = node;
    }
    else if(consume_reserved("do"))
    {
        node = new_node(ND_DO);

        // parse loop body
        node->lhs = statement();

        expect_reserved("while");
        expect_reserved("(");

        // parse loop condition
        node->cond = expression();

        expect_reserved(")");
        expect_reserved(";");
    }
    else if(consume_reserved("for"))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        node = new_node(ND_FOR);
        expect_reserved("(");

        // parse clause-1
        if(!consume_reserved(";"))
        {
            node->preexpr = expression();
            expect_reserved(";");
        }

        // parse expression-2
        if(!consume_reserved(";"))
        {
            node->cond = expression();
            expect_reserved(";");
        }

        // parse expression-3
        if(!consume_reserved(")"))
        {
            node->postexpr = expression();
            expect_reserved(")");
        }

        // parse loop body
        node->lhs = statement();
    }
    else if(consume_reserved("goto"))
    {
        node = new_node(ND_GOTO);
        node->ident = make_identifier(expect_identifier());
        expect_reserved(";");
    }
    else if(consume_reserved("if"))
    {
        node = new_node(ND_IF);
        expect_reserved("(");

        // parse condition
        node->cond = expression();

        expect_reserved(")");

        // parse statement in case of condition being true
        node->lhs = statement();

        // parse statement in case of condition being false
        if(consume_reserved("else"))
        {
            node->rhs = statement();
        }
    }
    else if(consume_reserved("return"))
    {
        node = new_node(ND_RETURN);
        if(!consume_reserved(";"))
        {
            // return statement with an expression
            node->lhs = expression();
            expect_reserved(";");
        }
    }
    else if(consume_reserved("switch"))
    {
        // save node of previous switch statement
        Node *prev_switch = current_switch;

        // parse controlling expression
        node = new_node(ND_SWITCH);
        expect_reserved("(");
        node->cond = expression();
        expect_reserved(")");

        // update node of currently parsing switch statement and parse body
        current_switch = node;
        node->lhs = statement();

        // restore node of previous switch statement
        current_switch = prev_switch;
    }
    else if(consume_reserved("while"))
    {
        node = new_node(ND_WHILE);
        expect_reserved("(");

        // parse loop condition
        node->cond = expression();

        expect_reserved(")");

        // parse loop body
        node->lhs = statement();
    }
    else if(consume_reserved(";"))
    {
        // null statement
        node = new_node(ND_NULL);
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
                node = new_node(ND_LABEL);
                node->lhs = statement();
                node->ident = make_identifier(token);
                goto statement_end;
            }
            else
            {
                // resume the token since it is not a label
                set_token(saved_token);
            }
        }

        // expression statement
        node = expression();
        expect_reserved(";");
    }

statement_end:
    return node;
}


/*
make a compound statement
```
compound-statement ::= "{" (declaration | statement)* "}"
```
*/
static Node *compound_statement(void)
{
    expect_reserved("{");

    // save the current scope
    VarScope *scope = enter_scope();

    // parse declaration and/or statement until reaching '}'
    Node head = {};
    Node *cursor = &head;
    while(!consume_reserved("}"))
    {
        if(peek_type_specifier())
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

    // restore the scope
    leave_scope(scope);

    return head.next;
}


/*
make a declaration
```
declaration ::= declaration-specifiers init-declarator-list ";"
```
*/
static Node *declaration(bool is_local)
{
    // parse declaration specifier
    Type *type = declaration_specifiers();

    // parse init-declarator-list
    Node *node = new_node(ND_DECL);
    node->body = init_declarator_list(type, is_local);

    expect_reserved(";");

    return node;
}


/*
make a init-declarator-list
```
init-declarator-list ::= init-declarator ("," init-declarator)*
```
*/
static Node *init_declarator_list(Type *type, bool is_local)
{
    Node *node = init_declarator(type, is_local);
    Node *cursor = node;

    while(consume_reserved(","))
    {
        cursor->next = init_declarator(type, is_local);
        cursor = cursor->next;
    }

    return node;
}


/*
make a init-declarator-list
```
init-declarator ::= declarator ("=" initializer)?
```
*/
static Node *init_declarator(Type *type, bool is_local)
{
    // parse declarator
    Token *token;
    type = declarator(type, &token);

    // check duplicated declaration
    VarScope *scope = find_scope(token);
    if((scope != NULL) && (scope->depth == scope_depth))
    {
        report_error(token->str, "duplicated declaration of '%s'\n", make_identifier(token));
    }

    // make a new node for variable
    Node *node = new_node(ND_VAR);
    node->type = type;
    if(is_local)
    {
        node->var = new_lvar(token, type);

        // parse initializer
        if(consume_reserved("="))
        {
            node->var->init = initializer();
        }
    }
    else
    {
        bool emit = (type->kind != TY_FUNC);
        node->var = new_gvar(token, type, emit);

        // parse initializer
        if(consume_reserved("="))
        {
            node->var->init = initializer();
        }
    }

    return node;
}


/*
make an initializer
```
initializer ::= assign
```
*/
static Node *initializer(void)
{
    return assign();
}


/*
make an expression
```
expression ::= assign ("," assign)*
```
*/
static Node *expression(void)
{
    Node *node = assign();

    // parse tokens while finding an assignment expression
    while(true)
    {
        if(consume_reserved(","))
        {
            node = new_node_binary(ND_COMMA, node, assign());
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
static Node *assign(void)
{
    Node *node = conditional();

    // parse assignment
    if(consume_reserved("="))
    {
        node = new_node_binary(ND_ASSIGN, node, assign());
    }
    else if(consume_reserved("*="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment *=");
        }

        node = new_node_binary(ND_MUL_EQ, lhs, rhs);
    }
    else if(consume_reserved("/="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment /=");
        }

        node = new_node_binary(ND_DIV_EQ, lhs, rhs);
    }
    else if(consume_reserved("%="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment %=");
        }

        node = new_node_binary(ND_MOD_EQ, lhs, rhs);
    }
    else if(consume_reserved("+="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(ND_ADD_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
        {
            node = new_node_binary(ND_PTR_ADD_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment +=");
        }
    }
    else if(consume_reserved("-="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }

        if(is_integer(lhs->type))
        {
            node = new_node_binary(ND_SUB_EQ, lhs, rhs);
        }
        else if(is_pointer(lhs->type))
        {
            node = new_node_binary(ND_PTR_SUB_EQ, lhs, rhs);
        }
        else
        {
            report_error(NULL, "bad operand for compound assignment -=");
        }
    }
    else if(consume_reserved("<<="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment <<=");
        }

        node = new_node_binary(ND_LSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved(">>="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment >>=");
        }

        node = new_node_binary(ND_RSHIFT_EQ, lhs, rhs);
    }
    else if(consume_reserved("&="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment &=");
        }

        node = new_node_binary(ND_AND_EQ, lhs, rhs);
    }
    else if(consume_reserved("^="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment ^=");
        }

        node = new_node_binary(ND_XOR_EQ, lhs, rhs);
    }
    else if(consume_reserved("|="))
    {
        Node *lhs = node;
        Node *rhs = assign();

        if(!is_integer(lhs->type) || !is_integer(rhs->type))
        {
            report_error(NULL, "bad operand for compound assignment |=");
        }

        node = new_node_binary(ND_OR_EQ, lhs, rhs);
    }

    return node;
}


/*
make a conditional expression
```
conditional ::= logical-or ("?" expression ":" conditional)?
```
*/
static Node *conditional(void)
{
    Node *node = logical_or();

    if(consume_reserved("?"))
    {
        Node *ternary = new_node(ND_COND);

        ternary->cond = node;
        ternary->lhs = expression();
        expect_reserved(":");
        ternary->rhs = conditional();

        // copy LHS and RHS since their types may not be modified
        Node lhs = *ternary->lhs;
        Node rhs = *ternary->rhs;
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
static Node *logical_or(void)
{
    Node *node = logical_and();

    // parse tokens while finding a bitwise logical AND expression
    while(true)
    {
        if(consume_reserved("||"))
        {
            node = new_node_binary(ND_LOG_OR, node, logical_and());
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
static Node *logical_and(void)
{
    Node *node = bitwise_or();

    // parse tokens while finding a bitwise inclusive OR expression
    while(true)
    {
        if(consume_reserved("&&"))
        {
            node = new_node_binary(ND_LOG_AND, node, bitwise_or());
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
static Node *bitwise_or(void)
{
    Node *node = bitwise_xor();

    // parse tokens while finding a bitwise exclusive OR expression
    while(true)
    {
        if(consume_reserved("|"))
        {
            node = new_node_binary(ND_BIT_OR, node, bitwise_xor());
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
static Node *bitwise_xor(void)
{
    Node *node = bitwise_and();

    // parse tokens while finding a bitwise AND expression
    while(true)
    {
        if(consume_reserved("^"))
        {
            node = new_node_binary(ND_BIT_XOR, node, bitwise_and());
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
static Node *bitwise_and(void)
{
    Node *node = equality();

    // parse tokens while finding a equality expression
    while(true)
    {
        if(consume_reserved("&"))
        {
            node = new_node_binary(ND_BIT_AND, node, equality());
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
static Node *equality(void)
{
    Node *node = relational();

    // parse tokens while finding a relational expression
    while(true)
    {
        if(consume_reserved("=="))
        {
            node = new_node_binary(ND_EQ, node, relational());
        }
        else if(consume_reserved("!="))
        {
            node = new_node_binary(ND_NEQ, node, relational());
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
static Node *relational(void)
{
    Node *node = shift();

    // parse tokens while finding a shift expression
    while(true)
    {
        if(consume_reserved("<"))
        {
            node = new_node_binary(ND_L, node, shift());
        }
        else if(consume_reserved("<="))
        {
            node = new_node_binary(ND_LEQ, node, shift());
        }
        else if(consume_reserved(">"))
        {
            node = new_node_binary(ND_L, shift(), node);
        }
        else if(consume_reserved(">="))
        {
            node = new_node_binary(ND_LEQ, shift(), node);
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
static Node *shift(void)
{
    Node *node = additive();

    // parse tokens while finding an additive expression
    while(true)
    {
        if(consume_reserved("<<"))
        {
            Node *lhs = node;
            Node *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_LSHIFT, lhs, rhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator <<");
            }
        }
        else if(consume_reserved(">>"))
        {
            Node *lhs = node;
            Node *rhs = additive();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_RSHIFT, lhs, rhs);
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
static Node *additive(void)
{
    Node *node = multiplicative();

    // parse tokens while finding a multiplicative expression
    while(true)
    {
        if(consume_reserved("+"))
        {
            Node *lhs = node;
            Node *rhs = multiplicative();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_ADD, lhs, rhs);
            }
            else if(is_pointer_or_array(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_PTR_ADD, lhs, rhs);
            }
            else if(is_integer(lhs->type) && is_pointer_or_array(rhs->type))
            {
                node = new_node_binary(ND_PTR_ADD, rhs, lhs);
            }
            else
            {
                report_error(NULL, "bad operand for binary operator +");
            }
        }
        else if(consume_reserved("-"))
        {
            Node *lhs = node;
            Node *rhs = multiplicative();

            if(is_integer(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_SUB, lhs, rhs);
            }
            else if(is_pointer_or_array(lhs->type) && is_integer(rhs->type))
            {
                node = new_node_binary(ND_PTR_SUB, lhs, rhs);
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
static Node *multiplicative(void)
{
    Node *node = cast();

    // parse tokens while finding a cast expression
    while(true)
    {
        if(consume_reserved("*"))
        {
            node = new_node_binary(ND_MUL, node, cast());
        }
        else if(consume_reserved("/"))
        {
            node = new_node_binary(ND_DIV, node, cast());
        }
        else if(consume_reserved("%"))
        {
            node = new_node_binary(ND_MOD, node, cast());
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
static Node *cast(void)
{
    Node *node;

    Token *saved_token = get_token();
    if(consume_reserved("("))
    {
        if(peek_type_specifier())
        {
            Type *type = type_name();
            expect_reserved(")");
            node = new_node(ND_CAST);
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
static Node *unary(void)
{
    Node *node;

    if(consume_reserved("sizeof"))
    {
        // The type of the result of 'sizeof' operator is 'size_t'.
        // This implementation regards 'size_t' as 'unsigned long'.
        Token *saved_token = get_token();
        if(consume_reserved("("))
        {
            if(peek_type_specifier())
            {
                Type *type = type_name();
                node = new_node_integer((IntegerConstant){.kind = TY_ULONG, .ulong_val = type->size});
                expect_reserved(")");
                goto unary_end;
            }
            else
            {
                set_token(saved_token);
            }
        }

        Node *operand = unary();
        node = new_node_integer((IntegerConstant){.kind = TY_ULONG, .ulong_val = operand->type->size});
    }
    else if(consume_reserved("++"))
    {
        Node *operand = unary();

        if(is_integer(operand->type))
        {
            node = new_node_binary(ND_ADD_EQ, operand, new_node_integer((IntegerConstant){.kind = TY_INT, .int_val = 1}));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(ND_PTR_ADD_EQ, operand, new_node_integer((IntegerConstant){.kind = TY_INT, .int_val = 1}));
        }
        else
        {
            report_error(NULL, "bad operand for prefix increment operator ++");
        }
    }
    else if(consume_reserved("--"))
    {
        Node *operand = unary();

        if(is_integer(operand->type))
        {
            node = new_node_binary(ND_SUB_EQ, operand, new_node_integer((IntegerConstant){.kind = TY_INT, .int_val = 1}));
        }
        else if(is_pointer(operand->type))
        {
            node = new_node_binary(ND_PTR_SUB_EQ, operand, new_node_integer((IntegerConstant){.kind = TY_INT, .int_val = 1}));
        }
        else
        {
            report_error(NULL, "bad operand for prefix decrement operator --");
        }
    }
    else if(consume_reserved("&"))
    {
        node = new_node_unary(ND_ADDR, unary());
    }
    else if (consume_reserved("*"))
    {
        node = new_node_unary(ND_DEREF, unary());
    }
    else if(consume_reserved("+"))
    {
        node = apply_integer_promotion(unary());
    }
    else if(consume_reserved("-"))
    {
        node = new_node_binary(ND_SUB, new_node_integer((IntegerConstant){.kind = TY_INT, .int_val = 0}), apply_integer_promotion(unary()));
    }
    else if (consume_reserved("~"))
    {
        node = new_node_unary(ND_COMPL, apply_integer_promotion(unary()));
    }
    else if (consume_reserved("!"))
    {
        node = new_node_unary(ND_NEG, unary());
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
postfix ::= primary ("[" expression "]" | "(" arg-expr-list? ")" | "++" | "--" )*
```
*/
static Node *postfix(void)
{
    Node *node = primary();

    // parse tokens while finding a postfix operator
    while(true)
    {
        if(consume_reserved("["))
        {
            // array subscripting
            Node *lhs;
            Node *index = expression();

            if(is_pointer_or_array(node->type) && is_integer(index->type))
            {
                lhs = new_node_binary(ND_PTR_ADD, node, index);
            }
            else if(is_integer(node->type) && is_pointer_or_array(index->type))
            {
                lhs = new_node_binary(ND_PTR_ADD, index, node);
            }
            else
            {
                report_error(NULL, "bad operand for [] operator\n");
            }

            node = new_node_unary(ND_DEREF, lhs);
            expect_reserved("]");
        }
        else if(consume_reserved("("))
        {
            // function call
            if((node->type->base == NULL) || (node->type->base->kind != TY_FUNC))
            {
                report_error(NULL, "expected function");
            }

            Node *func_node = new_node(ND_FUNC);
            if(!consume_reserved(")"))
            {
                func_node->args = arg_expr_list();
                expect_reserved(")");
            }
            func_node->type = node->type->base->base; // dereference pointer and get type of return value
            func_node->ident = node->ident;
            node = func_node;
        }
        else if(consume_reserved("++"))
        {
            // postfix increment
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix increment operator ++\n");
            }
            node = new_node_unary(ND_POST_INC, node);
        }
        else if(consume_reserved("--"))
        {
            // postfix decrement
            if(!(is_integer(node->type) || is_pointer(node->type)))
            {
                report_error(NULL, "bad operand for postfix decrement operator --\n");
            }
            node = new_node_unary(ND_POST_DEC, node);
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
static Node *arg_expr_list(void)
{
    Node *arg;
    Node *cursor = NULL;

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
static Node *primary(void)
{
    // expression in brackets
    if(consume_reserved("("))
    {
        Node *node = expression();
 
        expect_reserved(")");
 
        return node;
    }

    // identifier
    Token *token;
    if(consume_token(TK_IDENT, &token))
    {
        // search variable
        VarScope *scope = find_scope(token);
        if(scope != NULL)
        {
            Variable *var = scope->var;
            if(var->type->kind == TY_FUNC)
            {
                Node *node = new_node(ND_FUNC);
                node->type = new_type_pointer(var->type);
                node->ident = make_identifier(token);
                return node;
            }
            else
            {
                Node *node = new_node(ND_VAR);
                node->type = var->type;
                node->var = var;
                return node;
            }
        }

        if(peek_reserved("("))
        {
            // implicitly assume that the token denotes a function which returns int
            Node *node = new_node(ND_FUNC);
            node->type = new_type_pointer(new_type_function(new_type(TY_INT), NULL));
            node->ident = make_identifier(token);
#if(WARN_IMPLICIT_DECLARATION_OF_FUNCTION == ENABLED)
            report_warning(token->str, "implicit declaration of function '%s'\n", make_identifier(token));
#endif /* WARN_IMPLICIT_DECLARATION_OF_FUNCTION */
            return node;
        }

        report_error(token->str, "undefined variable '%s'", make_identifier(token));
    }

    // string-literal
    if(consume_token(TK_STR, &token))
    {
        Node *node = new_node(ND_VAR);
        node->var = new_string(token);
        node->type = node->var->type;
        return node;
    }

    // integer-constant
    token = expect_integer_constant();
    return new_node_integer(parse_integer_constant(token));
}


/*
make a new node
*/
static Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->next = NULL;
    node->kind = kind;
    node->lhs = NULL;
    node->rhs = NULL;
    node->type = NULL;
    node->val = (IntegerConstant){.kind = TY_INT, .int_val = 0};
    node->var = NULL;
    node->cond = NULL;
    node->preexpr = NULL;
    node->postexpr = NULL;
    node->body = NULL;
    node->ident = NULL;
    node->args = NULL;

    return node;
}


/*
make a new node for unary operations
*/
static Node *new_node_unary(NodeKind kind, Node *lhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;

    switch(kind)
    {
    case ND_ADDR:
        node->type = new_type_pointer(lhs->type);
        break;

    case ND_DEREF:
        node->type = lhs->type->base;
        break;

    case ND_POST_INC:
    case ND_POST_DEC:
        node->type = lhs->type;
        break;

    case ND_NEG:
        node->type = new_type(TY_INT);
        break;

    case ND_COMPL:
    default:
        break;
    }

    return node;
}


/*
make a new node for binary operations
*/
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);

    switch(kind)
    {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_MOD:
    case ND_L:
    case ND_LEQ:
    case ND_BIT_AND:
    case ND_BIT_XOR:
    case ND_BIT_OR:
        apply_arithmetic_conversion(lhs, rhs);
        node->type = lhs->type;
        break;

    case ND_LSHIFT:
    case ND_RSHIFT:
        lhs = apply_integer_promotion(lhs);
        rhs = apply_integer_promotion(rhs);
        node->type = lhs->type;
        break;

    case ND_EQ:
    case ND_NEQ:
        if(is_integer(lhs->type) && is_integer(rhs->type))
        {
            apply_arithmetic_conversion(lhs, rhs);
        }
        node->type = new_type(TY_INT);
        break;

    case ND_PTR_ADD:
    case ND_PTR_SUB:
        node->type = lhs->type;
        break;

    case ND_ASSIGN:
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

    case ND_ADD_EQ:
    case ND_PTR_ADD_EQ:
    case ND_SUB_EQ:
    case ND_PTR_SUB_EQ:
    case ND_MUL_EQ:
    case ND_DIV_EQ:
    case ND_MOD_EQ:
    case ND_LSHIFT_EQ:
    case ND_RSHIFT_EQ:
    case ND_AND_EQ:
    case ND_XOR_EQ:
    case ND_OR_EQ:
        node->type = lhs->type;
        break;

    case ND_COMMA:
        node->type = rhs->type;
        break;

    case ND_LOG_AND:
    case ND_LOG_OR:
    default:
        node->type = new_type(TY_INT);
        break;
    }

    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}


/*
make a new node for integer-constant
*/
static Node *new_node_integer(IntegerConstant val)
{
    Node *node = new_node(ND_CONST);
    node->val = val;

    switch(val.kind)
    {
    case TY_INT:
    case TY_UINT:
        node->type = new_type(TY_INT);
        break;

    case TY_LONG:
    case TY_ULONG:
        node->type = new_type(TY_LONG);
        break;

    default:
        node->type = new_type(TY_INT);
        break;
    }

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
static Node *apply_integer_promotion(Node *node)
{
    if((node->type->kind == TY_CHAR)
    || (node->type->kind == TY_UCHAR)
    || (node->type->kind == TY_SHORT)
    || (node->type->kind == TY_USHORT))
    {
        node->type = new_type(TY_INT);
    }

    return node;
}


/*
apply usual arithmetic conversion
*/
static void apply_arithmetic_conversion(Node *lhs, Node *rhs)
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
static Variable *new_var(char *name, Type *type, bool local)
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

    push_scope(var);

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
    Variable *gvar = new_var(new_string_label(), new_type_array(new_type(TY_CHAR), token->len + 1), false);
    gvar->content = calloc(token->len + 1, sizeof(char));
    strncpy(gvar->content, token->str, token->len);
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
static Function *new_function(const Token *token, Type *type, Node *body)
{
    Function *new_func = calloc(1, sizeof(Function));
    new_func->name = make_identifier(token);
    new_func->type = new_type_function(type->base, type->args);
    new_func->body = body;

    // accumulate stack size and set offset of arguments and local variables
    size_t stack_size = 0;
    if(type->args->kind != TY_VOID)
    {
        for(Variable *arg = args_list; arg != NULL; arg = arg->next)
        {
            stack_size += arg->type->size;
            arg->offset = stack_size;
        }
    }
    for(Variable *arg = lvar_list; arg != NULL; arg = arg->next)
    {
        stack_size += arg->type->size;
        arg->offset = stack_size;
    }

    // align stack size
    new_func->stack_size = (stack_size + (stack_alignment_size - 1)) & ~(stack_alignment_size - 1);

    // save list of arguments and list of local variables
    new_func->args = args_list;
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
push a variable to the current scope
*/
static void push_scope(Variable *var)
{
    VarScope *scope = calloc(1, sizeof(VarScope));
    scope->next = var_scope;
    scope->var = var;
    scope->depth = scope_depth;
    var_scope = scope;
}


/*
enter a new scope
*/
static VarScope *enter_scope(void)
{
    scope_depth++;
    return var_scope;
}


/*
leave the current scope
*/
static void leave_scope(VarScope *scope)
{
    scope_depth--;
    var_scope = scope;
}


/*
find a variable in the current scope
* If there exists a visible variable with a given token, this function returns the scope.
* Otherwise, it returns NULL.
*/
static VarScope *find_scope(const Token *token)
{
    // search list of variables visible in the current scope
    for(VarScope *scope = var_scope; scope != NULL; scope = scope->next)
    {
        if((strlen(scope->var->name) == token->len) && (strncmp(token->str, scope->var->name, token->len) == 0))
        {
            return scope;
        }
    }

    return NULL;
}


/*
parse an integer-constant
*/
static IntegerConstant parse_integer_constant(const Token *token)
{
    long long_val = strtol(token->str, NULL, 10);
    if(errno != ERANGE)
    {
        if((INT_MIN <= long_val) && (long_val <= INT_MAX))
        {
            return (IntegerConstant){.kind = TY_INT, .int_val = long_val};
        }
        else
        {
            return (IntegerConstant){.kind = TY_LONG, .long_val = long_val};
        }
    }
    else
    {
        report_warning(token->str, "integer constant is too large");
        return (IntegerConstant){.kind = TY_LONG, .long_val = long_val};
    }
}


/*
determine type from list of specifiers
*/
static Type *determine_type(void)
{
    for(size_t i = 0; i < TYPE_SPECS_MAP_SIZE; i++)
    {
        bool equal = true;

        for(size_t j = 0; j < SPEC_LIST_SIZE; j++)
        {
            if(TYPE_SPECS_MAP[i].spec_list[j] != current_spec_list[j])
            {
                equal = false;
                break;
            }
        }

        if(equal)
        {
            return new_type(TYPE_SPECS_MAP[i].type_kind);
        }
    }

    report_error(NULL, "invalid specifier");
    return NULL;
}


/*
clear list of currently parsing specifiers
*/
static void clear_current_spec_list(void)
{
    for(size_t i = 0; i < SPEC_LIST_SIZE; i++)
    {
        current_spec_list[i] = 0;
    }
}


/*
peek a type-specifier
*/
static bool peek_type_specifier(void)
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
peek a function
*/
static bool peek_func(void)
{
    // save the currently parsing token
    Token *saved_token = get_token();

    // parse declaration specifier and declarator
    Type *base = declaration_specifiers();
    Token *token;
    base = declarator(base, &token);

    // check if a compound statement follows
    bool is_func = consume_reserved("{");

    // resume the saved token
    set_token(saved_token);

    return is_func;
}


/*
make a constant expression
```
const-expression ::= conditional
```
*/
static IntegerConstant const_expression(void)
{
    return evaluate(conditional());
}


/*
evaluate a node
*/
static IntegerConstant evaluate(Node *node)
{
    IntegerConstant result = {.kind = TY_INT, .int_val = 0};

    switch(node->kind)
    {
    case ND_COMPL:
    {
        node = apply_integer_promotion(node);
        result.kind = node->type->kind;
        IntegerConstant operand = evaluate(node);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = ~operand.ulong_val;
            break;

        case TY_LONG:
            result.long_val = ~operand.long_val;
            break;

        case TY_UINT:
            result.uint_val = ~operand.uint_val;
            break;

        default:
            result.int_val = ~operand.int_val;
            break;
        }
        break;
    }

    case ND_NEG:
    {
        node = apply_integer_promotion(node);
        result.kind = TY_INT;
        result.int_val = !is_zero(evaluate(node));
        break;
    }

    case ND_ADD:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val + rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val + rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val + rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val + rhs.int_val;
            break;
        }
        break;
    }

    case ND_SUB:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val - rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val - rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val - rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val - rhs.int_val;
            break;
        }
        break;
    }

    case ND_MUL:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val * rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val * rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val * rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val * rhs.int_val;
            break;
        }
        break;
    }

    case ND_DIV:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val / rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val / rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val / rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val / rhs.int_val;
            break;
        }
        break;
    }

    case ND_MOD:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val % rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val % rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val % rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val % rhs.int_val;
            break;
        }
        break;
    }

    case ND_LSHIFT:
    {
        node->lhs = apply_integer_promotion(node->lhs);
        node->rhs = apply_integer_promotion(node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(lhs.kind)
        {
        case TY_ULONG:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.ulong_val = lhs.ulong_val << rhs.ulong_val;
                break;

            case TY_LONG:
                result.ulong_val = lhs.ulong_val << rhs.long_val;
                break;

            case TY_UINT:
                result.ulong_val = lhs.ulong_val << rhs.uint_val;
                break;

            default:
                result.ulong_val = lhs.ulong_val << rhs.int_val;
                break;
            }
            break;

        case TY_LONG:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.long_val = lhs.long_val << rhs.ulong_val;
                break;

            case TY_LONG:
                result.long_val = lhs.long_val << rhs.long_val;
                break;

            case TY_UINT:
                result.long_val = lhs.long_val << rhs.uint_val;
                break;

            default:
                result.long_val = lhs.long_val << rhs.int_val;
                break;
            }
            break;

        case TY_UINT:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.uint_val = lhs.uint_val << rhs.ulong_val;
                break;

            case TY_LONG:
                result.uint_val = lhs.uint_val << rhs.long_val;
                break;

            case TY_UINT:
                result.uint_val = lhs.uint_val << rhs.uint_val;
                break;

            default:
                result.uint_val = lhs.uint_val << rhs.int_val;
                break;
            }
            break;

        default:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.int_val = lhs.int_val << rhs.ulong_val;
                break;

            case TY_LONG:
                result.int_val = lhs.int_val << rhs.long_val;
                break;

            case TY_UINT:
                result.int_val = lhs.int_val << rhs.uint_val;
                break;

            default:
                result.int_val = lhs.int_val << rhs.int_val;
                break;
            }
            break;
        }
    }

    case ND_RSHIFT:
    {
        node->lhs = apply_integer_promotion(node->lhs);
        node->rhs = apply_integer_promotion(node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(lhs.kind)
        {
        case TY_ULONG:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.ulong_val = lhs.ulong_val >> rhs.ulong_val;
                break;

            case TY_LONG:
                result.ulong_val = lhs.ulong_val >> rhs.long_val;
                break;

            case TY_UINT:
                result.ulong_val = lhs.ulong_val >> rhs.uint_val;
                break;

            default:
                result.ulong_val = lhs.ulong_val >> rhs.int_val;
                break;
            }
            break;

        case TY_LONG:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.long_val = lhs.long_val >> rhs.ulong_val;
                break;

            case TY_LONG:
                result.long_val = lhs.long_val >> rhs.long_val;
                break;

            case TY_UINT:
                result.long_val = lhs.long_val >> rhs.uint_val;
                break;

            default:
                result.long_val = lhs.long_val >> rhs.int_val;
                break;
            }
            break;

        case TY_UINT:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.uint_val = lhs.uint_val >> rhs.ulong_val;
                break;

            case TY_LONG:
                result.uint_val = lhs.uint_val >> rhs.long_val;
                break;

            case TY_UINT:
                result.uint_val = lhs.uint_val >> rhs.uint_val;
                break;

            default:
                result.uint_val = lhs.uint_val >> rhs.int_val;
                break;
            }
            break;

        default:
            switch(rhs.kind)
            {
            case TY_ULONG:
                result.int_val = lhs.int_val >> rhs.ulong_val;
                break;

            case TY_LONG:
                result.int_val = lhs.int_val >> rhs.long_val;
                break;

            case TY_UINT:
                result.int_val = lhs.int_val >> rhs.uint_val;
                break;

            default:
                result.int_val = lhs.int_val >> rhs.int_val;
                break;
            }
            break;
        }
    }

    case ND_EQ:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = TY_INT;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(lhs.kind)
        {
        case TY_ULONG:
            result.int_val = (lhs.ulong_val == rhs.ulong_val);
            break;

        case TY_LONG:
            result.int_val = (lhs.long_val == rhs.long_val);
            break;

        case TY_UINT:
            result.int_val = (lhs.uint_val == rhs.uint_val);
            break;

        default:
            result.int_val = (lhs.int_val == rhs.int_val);
            break;
        }
        break;
    }

    case ND_NEQ:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = TY_INT;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(lhs.kind)
        {
        case TY_ULONG:
            result.int_val = (lhs.ulong_val != rhs.ulong_val);
            break;

        case TY_LONG:
            result.int_val = (lhs.long_val != rhs.long_val);
            break;

        case TY_UINT:
            result.int_val = (lhs.uint_val != rhs.uint_val);
            break;

        default:
            result.int_val = (lhs.int_val != rhs.int_val);
            break;
        }
        break;
    }

    case ND_L:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = TY_INT;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(lhs.kind)
        {
        case TY_ULONG:
            result.int_val = (lhs.ulong_val < rhs.ulong_val);
            break;

        case TY_LONG:
            result.int_val = (lhs.long_val < rhs.long_val);
            break;

        case TY_UINT:
            result.int_val = (lhs.uint_val < rhs.uint_val);
            break;

        default:
            result.int_val = (lhs.int_val < rhs.int_val);
            break;
        }
        break;
    }

    case ND_LEQ:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = TY_INT;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(lhs.kind)
        {
        case TY_ULONG:
            result.int_val = (lhs.ulong_val <= rhs.ulong_val);
            break;

        case TY_LONG:
            result.int_val = (lhs.long_val <= rhs.long_val);
            break;

        case TY_UINT:
            result.int_val = (lhs.uint_val <= rhs.uint_val);
            break;

        default:
            result.int_val = (lhs.int_val <= rhs.int_val);
            break;
        }
        break;
    }

    case ND_BIT_AND:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val & rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val & rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val & rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val & rhs.int_val;
            break;
        }
        break;
    }

    case ND_BIT_XOR:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val ^ rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val ^ rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val ^ rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val ^ rhs.int_val;
            break;
        }
        break;
    }

    case ND_BIT_OR:
    {
        apply_arithmetic_conversion(node->lhs, node->rhs);
        result.kind = node->lhs->type->kind;
        IntegerConstant lhs = evaluate(node->lhs);
        IntegerConstant rhs = evaluate(node->rhs);
        switch(result.kind)
        {
        case TY_ULONG:
            result.ulong_val = lhs.ulong_val | rhs.ulong_val;
            break;

        case TY_LONG:
            result.long_val = lhs.long_val | rhs.long_val;
            break;

        case TY_UINT:
            result.uint_val = lhs.uint_val | rhs.uint_val;
            break;

        default:
            result.int_val = lhs.int_val | rhs.int_val;
            break;
        }
        break;
    }

    case ND_LOG_AND:
        result.kind = TY_INT;
        result.int_val = !is_zero(evaluate(node->lhs)) && !is_zero(evaluate(node->rhs));
        break;

    case ND_LOG_OR:
        result.kind = TY_INT;
        result.int_val = !is_zero(evaluate(node->lhs)) || !is_zero(evaluate(node->rhs));
        break;

    case ND_CONST:
        result = node->val;
        break;

    default:
        report_error(NULL, "cannot evaluate");
        break;
    }

    return result;
}


/*
check if a given integer-constant is equal to zero
*/
static bool is_zero(IntegerConstant val)
{
    switch(val.kind)
    {
    case TY_ULONG:
        return (val.ulong_val == 0);

    case TY_LONG:
        return (val.long_val == 0);

    case TY_UINT:
        return (val.uint_val == 0);

    default:
        return (val.int_val == 0);
    }
}
