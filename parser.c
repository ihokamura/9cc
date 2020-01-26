/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* parser (syntax tree constructor)
*/

#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// function prototype
static void prg(void);
static GVar *gvar(const Token *token, GVar *cur_gvar, Type *base);
static Function *func(const Token *token, Function *cur_func, Type *base);
static bool peek_typename(void);
static Type *declarator(Type *type, Token **token);
static Type *type_spec(void);
static Type *type_suffix(Type *type);
static Node *stmt(void);
static Node *declaration(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *postfix(void);
static Node *primary(void);
static Node *new_node(NodeKind kind);
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static Node *new_node_func(const Token *token);
static GVar *new_gvar(const Token *token, GVar *cur_gvar, Type *type);
static GVar *get_gvar(const Token *token);
static LVar *new_lvar(const Token *token, LVar *cur_lvar, Type *type);
static LVar *get_lvar(const Token *token);
static Function *new_function(const Token *token, Function *cur_func, Type *base);
static Function *get_function(const Token *token);


// global variable
static GVar *gvar_list; // list of global variables
static Function *function_list; // list of functions
static Function *current_function; // currently constructing function


/*
construct syntax tree
*/
void construct(Program *program)
{
    prg();
    program->gvars = gvar_list;
    program->funcs = function_list;
}


/*
make a program
```
prg ::= (gvar | func)*
```
*/
static void prg(void)
{
    GVar gvar_head = {};
    GVar *gvar_cursor = &gvar_head;
    gvar_list = &gvar_head;

    Function func_head = {};
    Function *func_cursor = &func_head;
    function_list = &func_head;

    while(!at_eof())
    {
        // parse identifier and base type
        Type *base = type_spec();
        Token *token;

        base = declarator(base, &token);
        if(consume_reserved("("))
        {
            // parse function
            func_cursor = func(token, func_cursor, base);
        }
        else
        {
            // parse global variable
            gvar_cursor = gvar(token, gvar_cursor, base);
        }
    }

    gvar_list = gvar_head.next;
    function_list = func_head.next;
}


/*
make a global variable
```
gvar ::= type-spec declarator ";"
```
*/
static GVar *gvar(const Token *token, GVar *cur_gvar, Type *base)
{
    expect_reserved(";");

    return new_gvar(token, cur_gvar, base);
}


/*
make a function
```
func ::= type-spec declarator "(" (type-spec declarator ("," type-spec declarator)*)? ")" "{" stmt* "}"
```
*/
static Function *func(const Token *token, Function *cur_func, Type *base)
{
    // make a new function
    current_function = new_function(token, cur_func, base);

    // parse arguments
    Type *arg_type = type_spec();
    if(arg_type != NULL)
    {
        Token *arg_token;

        arg_type = declarator(arg_type, &arg_token);
        current_function->args[0] = new_lvar(arg_token, NULL, arg_type);
        current_function->argc++;

        for(size_t i = 1; (i < ARG_REGISTERS_SIZE) && consume_reserved(","); i++)
        {
            arg_type = type_spec();
            if(arg_type == NULL)
            {
                report_error(NULL, "expected type specifier\n");
            }

            arg_type = declarator(arg_type, &arg_token);
            current_function->args[i] = new_lvar(arg_token, NULL, arg_type);
            current_function->argc++;
        }
    }
    expect_reserved(")");

    // parse body
    Node head = {};
    Node *cursor = &head;

    expect_reserved("{");
    while(!consume_reserved("}"))
    {
        cursor->next = stmt();
        cursor = cursor->next;
    }
    current_function->body = head.next;

    return current_function;
}


/*
peek a type name
*/
static bool peek_typename(void)
{
    return (
           peek_reserved("char")
        || peek_reserved("int")
    );
}


/*
make a declarator
```
declarator ::= "*"* ident type-suffix
```
*/
static Type *declarator(Type *type, Token **token)
{
    // consume pointers
    while(consume_reserved("*"))
    {
        type = new_type_pointer(type);
    }

    // consume identifier
    *token = consume_ident();
    if(*token == NULL)
    {
        return NULL;
    }

    // consume type-suffix
    type = type_suffix(type);

    return type;
}


/*
make a type-spec
```
type-spec ::= "char" | "int"
```
*/
static Type *type_spec(void)
{
    if(consume_reserved("char"))
    {
        return new_type(TY_CHAR);
    }
    else if(consume_reserved("int"))
    {
        return new_type(TY_INT);
    }
    else
    {
        return NULL;
    }
}


/*
make a type-suffix
```
type-suffix ::= ("[" num "]" | type-suffix)?
```
*/
static Type *type_suffix(Type *type)
{
    if(!consume_reserved("["))
    {
        return type;
    }

    size_t size = expect_number();
    expect_reserved("]");
    type = type_suffix(type);
    type = new_type_array(type, size);

    return type;
}


/*
make a statement
```
stmt ::= declaration |
         expr ";" |
         "{" stmt* "}" |
         "do" stmt "while" "(" expr ")" ";" |
         "for" "(" expr? ";" expr? ";" expr? ")" stmt |
         "if" "(" expr ")" stmt ("else" stmt)? |
         "return" expr ";" |
         "while" "(" expr ")" stmt
```
*/
static Node *stmt(void)
{
    Node *node;

    if(consume_reserved("{"))
    {
        // parse statements until reaching '}'
        Node head = {};
        Node *cursor = &head;
        while(!consume_reserved("}"))
        {
            cursor->next = stmt();
            cursor = cursor->next;
        }

        node = new_node(ND_BLOCK);
        node->body = head.next;

        return node;
    }
    if(consume_reserved("do"))
    {
        node = new_node(ND_DO);

        // parse loop body
        node->lhs = stmt();

        expect_reserved("while");
        expect_reserved("(");

        // parse loop condition
        node->cond = expr();

        expect_reserved(")");
        expect_reserved(";");

        return node;
    }
    else if(consume_reserved("for"))
    {
        // for statement should be of the form `for(clause-1; expression-2; expression-3) statement`
        // clause-1, expression-2 and/or expression-3 may be empty.
        Node *node = new_node(ND_FOR);
        expect_reserved("(");

        // parse clause-1
        if(!consume_reserved(";"))
        {
            node->preexpr = expr();
            expect_reserved(";");
        }

        // parse expression-2
        if(!consume_reserved(";"))
        {
            node->cond = expr();
            expect_reserved(";");
        }

        // parse expression-3
        if(!consume_reserved(")"))
        {
            node->postexpr = expr();
            expect_reserved(")");
        }

        // parse loop body
        node->lhs = stmt();

        return node;
    }
    else if(consume_reserved("if"))
    {
        node = new_node(ND_IF);
        expect_reserved("(");

        // parse condition
        node->cond = expr();

        expect_reserved(")");

        // parse statement in case of condition being true
        node->lhs = stmt();

        // parse statement in case of condition being false
        if(consume_reserved("else"))
        {
            node->rhs = stmt();
        }

        return node;
    }
    else if(consume_reserved("return"))
    {
        node = new_node(ND_RETURN);
        node->lhs = expr();
        expect_reserved(";");

        return node;
    }
    else if(consume_reserved("while"))
    {
        node = new_node(ND_WHILE);
        expect_reserved("(");

        // parse loop condition
        node->cond = expr();

        expect_reserved(")");

        // parse loop body
        node->lhs = stmt();

        return node;
    }
    else
    {
        if(peek_typename())
        {
            // declaration
            node = declaration();
            expect_reserved(";");

            return node;
        }
        else
        {
            // expression statement
            node = expr();
            expect_reserved(";");

            return node;
        }
    }
}


/*
make a declaration
```
declaration ::= type-spec declarator ";"
```
*/
static Node *declaration(void)
{
    // parse type-specifier
    Type *type = type_spec();

    // parse declarator
    Token *token;
    type = declarator(type, &token);

    if(get_lvar(token) != NULL)
    {
        report_error(token->str, "duplicated declaration of '%s'\n", make_ident(token));
    }

    current_function->locals = new_lvar(token, current_function->locals, type);

    Node *node = new_node(ND_DECL);
    node->lvar = current_function->locals;

    return node;
}


/*
make an expression
```
expr ::= assign
```
*/
static Node *expr(void)
{
    return assign();
}


/*
make an assignment expression
```
assign ::= equality ("=" assign)?
```
*/
static Node *assign(void)
{
    Node *node = equality();

    // parse assignment
    if(consume_reserved("="))
    {
        node = new_node_binary(ND_ASSIGN, node, assign());
    }

    return node;
}


/*
make an equality
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
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
```
*/
static Node *relational(void)
{
    Node *node = add();

    // parse tokens while finding an addition term
    while(true)
    {
        if(consume_reserved("<"))
        {
            node = new_node_binary(ND_L, node, add());
        }
        else if(consume_reserved("<="))
        {
            node = new_node_binary(ND_LEQ, node, add());
        }
        else if(consume_reserved(">"))
        {
            node = new_node_binary(ND_L, add(), node);
        }
        else if(consume_reserved(">="))
        {
            node = new_node_binary(ND_LEQ, add(), node);
        }
        else
        {
            return node;
        }
    }
}


/*
make an addition term
```
add ::= mul ("+" mul | "-" mul)*
```
*/
static Node *add(void)
{
    Node *node = mul();

    // parse tokens while finding a term
    while(true)
    {
        if(consume_reserved("+"))
        {
            Node *lhs = node;
            Node *rhs = mul();

            if(is_pointer(lhs) && !is_pointer(rhs))
            {
                node = new_node_binary(ND_PTR_ADD, lhs, rhs);
            }
            else if(!is_pointer(lhs) && is_pointer(rhs))
            {
                node = new_node_binary(ND_PTR_ADD, rhs, lhs);
            }
            else
            {
                node = new_node_binary(ND_ADD, lhs, rhs);
            }
        }
        else if(consume_reserved("-"))
        {
            Node *lhs = node;
            Node *rhs = mul();

            if(is_pointer(lhs) && !is_pointer(rhs))
            {
                node = new_node_binary(ND_PTR_SUB, lhs, rhs);
            }
            else
            {
                node = new_node_binary(ND_SUB, lhs, rhs);
            }
        }
        else
        {
            return node;
        }
    }
}


/*
make a multiplication term
```
mul ::= unary ("*" unary | "/" unary)*
```
*/
static Node *mul(void)
{
    Node *node = unary();

    // parse tokens while finding an unary
    while(true)
    {
        if(consume_reserved("*"))
        {
            node = new_node_binary(ND_MUL, node, unary());
        }
        else if(consume_reserved("/"))
        {
            node = new_node_binary(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}


/*
make an unary
```
unary ::= postfix | ("&" | "*" | "+" | "-")? unary | sizeof unary
```
*/
static Node *unary(void)
{
    Node *node;

    if(consume_reserved("sizeof"))
    {
        Node *operand = unary();
        node = new_node_num(operand->type->size);
    }
    else if(consume_reserved("&"))
    {
        node = new_node(ND_ADDR);
        node->lhs = unary();
        node->type = new_type_pointer(node->lhs->type);
    }
    else if (consume_reserved("*"))
    {
        node = new_node(ND_DEREF);
        node->lhs = unary();
        node->type = node->lhs->type->base;
    }
    else if(consume_reserved("+"))
    {
        node = unary();
    }
    else if(consume_reserved("-"))
    {
        node = new_node_binary(ND_SUB, new_node_num(0), unary());
    }
    else
    {
        node = postfix();
    }

    return node;
}


/*
make a postfix
```
postfix ::= primary ("[" expr "]")*
```
*/
static Node *postfix(void)
{
    Node *node = primary();

    while(consume_reserved("["))
    {
        Node *lhs;
        Node *index = expr();

        if(is_pointer(node) && !is_pointer(index))
        {
            lhs = new_node_binary(ND_PTR_ADD, node, index);
        }
        else if(!is_pointer(node) && is_pointer(index))
        {
            lhs = new_node_binary(ND_PTR_ADD, index, node);
        }
        else
        {
            report_error(NULL, "bad operand for [] operator\n");
        }

        node = new_node(ND_DEREF);
        node->lhs = lhs;
        node->type = node->lhs->type->base;
        expect_reserved("]");
    }

    return node;
}


/*
make a primary
```
primary ::= num | ident ("(" (expr ("," expr)*)? ")")? | "(" expr ")"
```
*/
static Node *primary(void)
{
    // expression in brackets
    if(consume_reserved("("))
    {
        Node *node = expr();
 
        expect_reserved(")");
 
        return node;
    }

    // identifier
    Token *ident = consume_ident();
    if(ident != NULL)
    {
        if(consume_reserved("("))
        {
            // function
            Node *node = new_node_func(ident);
            if(consume_reserved(")"))
            {
                for(size_t i = 0; i < sizeof(node->args) / sizeof(node->args[0]); i++)
                {
                    node->args[i] = NULL;
                }
            }
            else
            {
                // arguments
                node->args[0] = expr();
                for(size_t i = 1; (i < sizeof(node->args) / sizeof(node->args[0])) && consume_reserved(","); i++)
                {
                    node->args[i] = expr();
                }
                expect_reserved(")");
            }

            return node;
        }
        else
        {
            // variable
            GVar *gvar = get_gvar(ident);
            if(gvar != NULL)
            {
                Node *node = new_node(ND_GVAR);
                node->type = gvar->type;
                node->gvar = gvar;
                return node;
            }

            LVar *lvar = get_lvar(ident);
            if(lvar != NULL)
            {
                Node *node = new_node(ND_LVAR);
                node->type = lvar->type;
                node->lvar = lvar;
                return node;
            }

            report_error(ident->str, "undefined variable '%s'", make_ident(ident));
        }
    }

    // number
    return new_node_num(expect_number());
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
    node->val = 0;
    node->lvar = NULL;
    node->cond = NULL;
    node->preexpr = NULL;
    node->postexpr = NULL;
    node->body = NULL;
    node->ident = NULL;
    for(size_t i = 0; i < sizeof(node->args) / sizeof(node->args[0]); i++)
    {
        node->args[i] = NULL;
    }

    return node;
}


/*
make a new node for binary operations
*/
static Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;

    switch(kind)
    {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NEQ:
    case ND_L:
    case ND_LEQ:
        node->type = new_type(TY_INT);
        break;

    case ND_PTR_ADD:
    case ND_PTR_SUB:
        node->type = lhs->type;
        break;

    case ND_ASSIGN:
        if(rhs->type->kind == TY_ARRAY)
        {
            // convert from array to pointer
            node->type = new_type_pointer(rhs->type->base);
        }
        else
        {
            node->type = rhs->type;
        }
        break;

    default:
        node->type = new_type(TY_INT);
        break;
    }

    return node;
}


/*
make a new node for number
*/
static Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->type = new_type(TY_INT);
    node->val = val;

    return node;
}


/*
make a new node for function call
*/
static Node *new_node_func(const Token *token)
{
    Type *type;
    Function *func = get_function(token);
    if(func == NULL)
    {
        // implicitly assume that the function returns int
        type = new_type(TY_INT);
        report_warning(token->str, "implicit declaration of function '%s'\n", make_ident(token));
    }
    else
    {
        type = func->type;
    }

    Node *node = new_node(ND_FUNC);
    node->type = type;
    node->ident = make_ident(token);

    return node;
}


/*
make a new global variable
*/
static GVar *new_gvar(const Token *token, GVar *cur_gvar, Type *type)
{
    GVar *gvar = calloc(1, sizeof(GVar));
    gvar->name = make_ident(token);
    gvar->type = type;
    cur_gvar->next = gvar;

    return gvar;
}


/*
get an existing global variable
* If there exists a global variable with a given token, this function returns the variable.
* Otherwise, it returns NULL.
*/
static GVar *get_gvar(const Token *token)
{
    // search list of gocal variables
    for(GVar *gvar = gvar_list->next; gvar != NULL; gvar = gvar->next)
    {
        if((strlen(gvar->name) == token->len) && (strncmp(token->str, gvar->name, token->len) == 0))
        {
            return gvar;
        }
    }

    return NULL;
}


/*
make a new local variable
*/
static LVar *new_lvar(const Token *token, LVar *cur_lvar, Type *type)
{
    current_function->stack_size += type->size;

    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = cur_lvar;
    lvar->str = token->str;
    lvar->len = token->len;
    lvar->offset = current_function->stack_size;
    lvar->type = type;

    return lvar;
}


/*
get a function argument or an existing local variable
* If there exists a function argument or a local variable with a given token, this function returns the variable.
* Otherwise, it returns NULL.
*/
static LVar *get_lvar(const Token *token)
{
    // search list of function arguments
    for(size_t i = 0; (i < ARG_REGISTERS_SIZE) && (current_function->args[i] != NULL); i++)
    {
        LVar *lvar = current_function->args[i];
        if((lvar->len == token->len) && (strncmp(token->str, lvar->str, token->len) == 0))
        {
            return lvar;
        }
    }

    // search list of local variables
    for(LVar *lvar = current_function->locals; lvar != NULL; lvar = lvar->next)
    {
        if((lvar->len == token->len) && (strncmp(token->str, lvar->str, token->len) == 0))
        {
            return lvar;
        }
    }

    return NULL;
}


/*
make a new function
*/
static Function *new_function(const Token *token, Function *cur_func, Type *base)
{
    Function *new_func = calloc(1, sizeof(Function));

    // initialize the name
    new_func->name = make_ident(token);

    // initialize arguments
    for(size_t i = 0; i < ARG_REGISTERS_SIZE; i++)
    {
        new_func->args[i] = NULL;
    }
    new_func->argc = 0;

    // initialize type of return value
    new_func->type = base;

    // initialize function body
    new_func->body = NULL;

    // initialize list of local variables
    new_func->locals = NULL;

    // initialize stack size
    new_func->stack_size = 0;

    // update list of functions
    cur_func->next = new_func;

    return new_func;
}


/*
get an existing function
* If there exists a function with a given token, this function returns the function.
* Otherwise, it returns NULL.
*/
static Function *get_function(const Token *token)
{
    // search list of function
    for(Function *func = function_list->next; func != NULL; func = func->next)
    {
        if((strlen(func->name) == token->len) && (strncmp(token->str, func->name, token->len) == 0))
        {
            return func;
        }
    }

    return NULL;
}