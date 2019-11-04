/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// kind of token
typedef enum {
    TK_RESERVED, // operator
    TK_NUM,      // integer
    TK_EOF,      // end of input
} TokenKind;

// kind of node in AST(abstract syntax tree)
typedef enum {
    ND_ADD, // addition(+)
    ND_SUB, // subtraction(-)
    ND_MUL, // multiplication(*)
    ND_DIV, // division(/)
    ND_EQ,  // equal to(==)
    ND_NEQ, // not equal to(!=)
    ND_L,   // less than(<)
    ND_LEQ, // less than or equal to(<=)
    ND_NUM, // integer
} NodeKind;

// structure for token
typedef struct Token Token;
struct Token {
    TokenKind kind; // kind of token
    Token *next;    // next input token
    int val;        // value of token (only for TK_NUM)
    char *str;      // token string
    int len;        // length of token string
};

// structure for node in AST
typedef struct Node Node;
struct Node {
    NodeKind kind; // kind of node
    Node *lhs;     // left hand side
    Node *rhs;     // right hand side
    int val;       // value of node (only for ND_NUM)
};


// currently parsing token
Token *token;
// input expression
char *user_input;


// function prototype
Node *expr(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);


/*
report an error
* This function never returns.
*/
void error_at(char *loc, const char *fmt, ...)
{
    // emphasize the position where the error is detected
    int pos = (loc - user_input) / sizeof(user_input[0]);
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s ^", pos, "");

    // print the error message
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    exit(1);
}


/*
consume a operator
* If the next token is a given operator, this function parses the token and returns true.
* Otherwise, it returns false.
*/
bool consume(const char *op)
{
    if(
        (token->kind != TK_RESERVED) || 
        (token->len != strlen(op)) || 
        (memcmp(token->str, op, token->len) != 0)
        )
    {
        return false;
    }

    token = token->next;

    return true;
}


/*
parse an operator
* If the next token is a given operator, this function parses the token.
* Otherwise, it reports an error.
*/
void expect(const char *op)
{
    if(
        (token->kind != TK_RESERVED) || 
        (token->len != strlen(op)) || 
        (memcmp(token->str, op, token->len) != 0)
        )
    {
        error_at(token->str, "not '%s'.", op);
    }

    token = token->next;
}


/*
parse a number
* If the next token is a number, this function parses the token and returns the value.
* Otherwise, it reports an error.
*/
int expect_number(void)
{
    if(token->kind != TK_NUM)
    {
        error_at(token->str, "not a number.");
    }

    int val = token->val;

    token = token->next;

    return val;
}


/*
make a new token and concatenate it to the current token
*/
Token *new_token(TokenKind kind, Token *cur_tok, char *str, int len)
{
    Token *new_tok = calloc(1, sizeof(Token));

    new_tok->kind = kind;
    new_tok->str = str;
    new_tok->len = len;
    cur_tok->next = new_tok;

    return new_tok;
}


/*
tokenize a given string
*/
Token *tokenize(char *str)
{
    Token head;
    head.next = NULL;

    Token *current = &head;

    while(*str)
    {
        // ignore space
        if(isspace(*str))
        {
            str++;
            continue;
        }

        // parse an operator
        if(
            (memcmp(str, "==", 2) == 0) || 
            (memcmp(str, "!=", 2) == 0) || 
            (memcmp(str, "<=", 2) == 0) || 
            (memcmp(str, ">=", 2) == 0)
            )
        {
            current = new_token(TK_RESERVED, current, str, 2);
            str += 2;
            continue;
        }
        if(
            (*str == '+') || 
            (*str == '-') || 
            (*str == '*') || 
            (*str == '/') || 
            (*str == '(') || 
            (*str == ')') || 
            (*str == '<') || 
            (*str == '>')
            )
        {
            current = new_token(TK_RESERVED, current, str, 1);
            str++;
            continue;
        }

        // parse a number
        if(isdigit(*str))
        {
            current = new_token(TK_NUM, current, str, 0);
            current->val = strtol(str, &str, 10);
            continue;
        }

        // Other characters are not accepted as a token.
        error_at(token->str, "cannot tokenize.");
    }

    new_token(TK_EOF, current, str, 0);

    return head.next;
}


/*
make a new node for non-number
*/
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}


/*
make a new node for number
*/
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->val = val;

    return node;
}


/*
make an expression
* expr = equality
*/
Node *expr(void)
{
    return equality();
}


/*
make an equality
* equality = relational ("==" relational | "!=" relational)*
*/
Node *equality(void)
{
    Node *node = relational();

    // parse tokens while finding a relational expression
    while(true)
    {
        if(consume("=="))
        {
            node = new_node(ND_EQ, node, relational());
        }
        else if(consume("!="))
        {
            node = new_node(ND_NEQ, node, relational());
        }
        else
        {
            return node;
        }
    }
}


/*
make a relational expression
* relational = add ("<" add | "<=" add | ">" add | ">=" add)*
*/
Node *relational(void)
{
    Node *node = add();

    // parse tokens while finding an addition term
    while(true)
    {
        if(consume("<"))
        {
            node = new_node(ND_L, node, add());
        }
        else if(consume("<="))
        {
            node = new_node(ND_LEQ, node, add());
        }
        else if(consume(">"))
        {
            node = new_node(ND_L, add(), node);
        }
        else if(consume(">="))
        {
            node = new_node(ND_LEQ, add(), node);
        }
        else
        {
            return node;
        }
    }
}


/*
make an addition term
* add = mul ("+" mul | "-" mul)*
*/
Node *add(void)
{
    Node *node = mul();

    // parse tokens while finding a term
    while(true)
    {
        if(consume("+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume("-"))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}


/*
make a multiplication term
* mul = unary ("*" unary | "/" unary)*
*/
Node *mul(void)
{
    Node *node = unary();

    // parse tokens while finding a primary
    while(true)
    {
        if(consume("*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if(consume("/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}


/*
make an unary
* unary = ("+" | "-")? primary
*/
Node *unary(void)
{
    // parse sign
    if(consume("+"))
    {
        return primary();
    }
    else if(consume("-"))
    {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    else
    {
        return primary();
    }
}


/*
make a primary
* primary = num | "(" expr ")"
*/
Node *primary(void)
{
    if(consume("("))
    {
        // expression in brackets
        Node *node = expr();
 
        expect(")");
 
        return node;
    }

    // number
    return new_node_num(expect_number());
}


/*
generate assembler code which emulates stack machine
*/
void gen(const Node *node)
{
    if(node->kind == ND_NUM)
    {
        printf("  push %d\n", node->val);

        return;
    }

    // compile LHS and RHS
    gen(node->lhs);
    gen(node->rhs);

    // pop RHS to rdi and LHS to rax
    printf("  pop rdi\n");
    printf("  pop rax\n");

    // execute operation
    switch(node->kind)
    {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;

        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;

        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;

        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;

        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzx rax, al\n");
            break;

        case ND_NEQ:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzx rax, al\n");
            break;

        case ND_L:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzx rax, al\n");
            break;

        case ND_LEQ:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzx rax, al\n");
            break;

        default:
            break;
    }

    // push return value
    printf("  push rax\n");
}


/*
main function of 9cc
*/
int main(int argc, char **argv)
{
    // check arguments
    if(argc != 2)
    {
        fprintf(stderr, "wrong number of arguments.\n");
        return 1;
    }

    // save input
    user_input = argv[1];
    // parse input
    token = tokenize(user_input);
    // construct syntax tree
    Node *root = expr();

    // use Intel syntax
    printf(".intel_syntax noprefix\n");

    // start main function
    printf(".global _main\n");
    printf("_main:\n");

    // output assembler according to the syntax tree
    gen(root);

    // pop return value from stack
    printf("  pop rax\n");

    // exit main function
    printf("  ret\n");

    return 0;
}