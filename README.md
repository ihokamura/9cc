# 9cc compiler


## Usage
```
9cc [ <file> | -c <code> ]
```


## Syntax
```
program ::= (declaration | function-def)*
function-def ::= declaration-specifiers declarator compound-statement
statement ::= identifier ":" statement
            | "case" const-expression ":" statement
            | "default" ":" statement
            | compound-statement
            | expression? ";"
            | "if" "(" expression ")" statement ("else" statement)?
            | "switch" "(" expression ")" statement
            | "while" "(" expression ")" statement
            | "do" statement "while" "(" expression ")" ";"
            | "for" "(" expression? ";" expression? ";" expression? ")" statement
            | "goto" identifier ";"
            | "continue" ";"
            | "break" ";"
            | "return" expression ";"
compound-statement ::= "{" (declaration | statement)* "}"
expression ::= assign ("," assign)*
assign ::= conditional (assign-op assign)?
assign-op ::= "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|="
conditional ::= logical-or ("?" expression ":" conditional)?
logical-or ::= logical-and (|| logical-and)*
logical-and ::= bitwise-or (&& bitwise-or)*
bitwise-or ::= bitwise-xor (| bitwise-xor)*
bitwise-xor ::= bitwise-and (^ bitwise-and)*
bitwise-and ::= equality (& equality)*
equality ::= relational ("==" relational | "!=" relational)*
relational ::= shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
shift ::=  additive ("<<" additive | ">>" additive)*
additive ::= multiplicative ("+" multiplicative | "-" multiplicative)*
multiplicative ::= cast ("*" cast | "/" cast | "%" cast)*
cast ::= ("(" type-name ")")? unary
unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | "sizeof" unary
        | "sizeof" "(" type-name ")"
unary-op ::= "&" | "*" | "+" | "-" | "~" | "!"
postfix ::= primary
          | postfix "[" expression "]"
          | postfix "(" arg-expr-list? ")"
          | postfix "." identifier
          | postfix "->" identifier
          | postfix "++"
          | postfix "--"
arg-expr-list ::= assign ("," assign)*
primary ::= identifier
          | integer-constant
          | string-literal
          | "(" expression ")"
const-expression ::= conditional
declaration ::= declaration-specifiers init-declarator-list? ";"
declaration-specifiers ::= (storage-class-specifier | type-specifier)*
storage-class-specifier ::= "typedef"
                          | "extern"
                          | "static"
                          | "auto"
                          | "register"
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
struct-or-union-specifier ::= ("struct" | "union") identifier? "{" struct-declaration-list "}"
                            | ("struct" | "union") identifier
struct-declaration-list ::= struct-declaration struct-declaration*
struct-declaration ::= specifier-list struct-declarator-list ";"
specifier-list ::= type-specifier*
struct-declarator-list ::= declarator ("," declarator)*
enum-specifier ::= "enum" identifier? "{" enumerator-list (",")? "}"
                 | "enum" identifier
enumerator-list ::= enumerator ("," enumerator)*
enumerator ::= identifier ("=" const-expression)?
init-declarator-list ::= init-declarator ("," init-declarator)*
init-declarator ::= declarator ("=" initializer)?
declarator ::= pointer? direct-declarator
direct-declarator ::= identifier
                    | direct-declarator "[" const-expression "]"
                    | direct-declarator "(" ("void" | parameter-list)? ")"
parameter-list ::= parameter-declaration ("," parameter-declaration)*
parameter-declaration ::= declaration-specifiers declarator
initializer ::= assign
type-name ::= specifier-list pointer?
pointer ::= "*" "*"*
```


## Reference
[1] https://www.sigbus.info/compilerbook

[2] https://www.bottlecaps.de/rr/ui

[3] https://wiki.osdev.org/System_V_ABI