# 9cc compiler


## Usage
```
9cc [ <file> | -c <code> ]
```


## Syntax

### Expressions
```
primary ::= identifier
          | constant
          | string-literal
          | "(" expression ")"
          | generic-selection
generic-selection ::= "_Generic" "(" assign "," generic-assoc-list ")"
generic-assoc-list ::= generic-association ("," generic-association)*
generic-association ::= (type-name | "default") ":" assign

postfix ::= primary
          | postfix "[" expression "]"
          | postfix "(" arg-expr-list? ")"
          | postfix "." identifier
          | postfix "->" identifier
          | postfix "++"
          | postfix "--"
          | "(" type-name ")" "{" initializer-list ","? "}"
arg-expr-list ::= assign ("," assign)*

unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | "sizeof" unary
        | "sizeof" "(" type-name ")"
        | "_Alignof" "(" type-name ")"
unary-op ::= "&" | "*" | "+" | "-" | "~" | "!"

cast ::= ("(" type-name ")")? unary

multiplicative ::= cast ("*" cast | "/" cast | "%" cast)*

additive ::= multiplicative ("+" multiplicative | "-" multiplicative)*

shift ::=  additive ("<<" additive | ">>" additive)*

relational ::= shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*

equality ::= relational ("==" relational | "!=" relational)*

bitwise-and ::= equality (& equality)*

bitwise-xor ::= bitwise-and (^ bitwise-and)*

bitwise-or ::= bitwise-xor (| bitwise-xor)*

logical-and ::= bitwise-or (&& bitwise-or)*

logical-or ::= logical-and (|| logical-and)*

conditional ::= logical-or ("?" expression ":" conditional)?

assign ::= conditional (assign-op assign)?
assign-op ::= "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|="

expression ::= assign ("," assign)*

const-expression ::= conditional
```

# Declarations
```
declaration ::= declaration-specifiers init-declarator-list? ";"
              | static_assert-declaration
declaration-specifiers ::= (storage-class-specifier | type-specifier | type-qualifier | function-specifier | alignment-specifier)*
init-declarator-list ::= init-declarator ("," init-declarator)*
init-declarator ::= declarator ("=" initializer)?

storage-class-specifier ::= "typedef"
                          | "extern"
                          | "static"
                          | "_Thread_local"
                          | "auto"
                          | "register"

type-specifier ::= "void"
                 | "char"
                 | "short"
                 | "int"
                 | "long"
                 | "float"
                 | "double"
                 | "signed"
                 | "unsigned"
                 | "_Bool"
                 | atomic-type-specifier
                 | struct-or-union-specifier
                 | enum-specifier
                 | typedef-name
atomic-type-specifier ::= "_Atomic" "(" type-name ")"
struct-or-union-specifier ::= ("struct" | "union") identifier? "{" struct-declaration-list "}"
                            | ("struct" | "union") identifier
struct-declaration-list ::= struct-declaration struct-declaration*
struct-declaration ::= specifier-list (struct-declarator-list)? ";"
specifier-qualifier-list ::= (type-specifier | type-qualifier)*
struct-declarator-list ::= struct-declarator ("," struct-declarator)*
struct-declarator-list ::= declarator | declarator? ":" const-expression
enum-specifier ::= "enum" identifier? "{" enumerator-list (",")? "}"
                 | "enum" identifier
enumerator-list ::= enumerator ("," enumerator)*
enumerator ::= identifier ("=" const-expression)?

type-qualifier ::= "const"
                 | "restrict"
                 | "volatile"
                 | "_Atomic"

function-specifier ::= "inline" | "_Noreturn"

alignment-specifier ::= "_Alignas" "(" type-name | const-expression ")"

declarator ::= pointer? direct-declarator
direct-declarator ::= identifier
                    | "(" declarator ")"
                    | direct-declarator "[" const-expression* "]"
                    | direct-declarator "(" parameter-type-list? ")"
pointer ::= "*" ("*" | type-qualifier)*
parameter-type-list ::= parameter-list ("," "...")?
parameter-list ::= parameter-declaration ("," parameter-declaration)*
parameter-declaration ::= declaration-specifiers (declarator | abstract-declarator?)

type-name ::= specifier-qualifier-list abstract-declarator?
abstract-declarator ::= pointer | pointer? direct-abstract-declarator
direct-abstract-declarator ::= "(" abstract-declarator ")"
                             | direct-abstract-declarator? "[" const-expression "]"
                             | direct-abstract-declarator? "(" ("void" | parameter-type-list)? ")"

typedef-name ::= identifier

initializer ::= assign
              | "{" initializer-list ","? "}"
initializer-list ::= designation? initializer ("," designation? initializer)*
designation ::= designator-list "="
designator-list ::= designator designator*
designator ::= "[" const-expression "]" | "." identifier

static_assert-declaration ::= "_Static_assert" "(" const-expression "," string-literal ")"
```

# Statements
```
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
            | "for" "(" declaration expression? ";" expression? ")" statement
            | "goto" identifier ";"
            | "continue" ";"
            | "break" ";"
            | "return" expression? ";"
compound-statement ::= "{" (declaration | statement)* "}"
```

# External definitions
```
program ::= (declaration | function-def)*
function-def ::= declaration-specifiers declarator compound-statement
```


## Reference
[1] https://www.sigbus.info/compilerbook

[2] https://www.bottlecaps.de/rr/ui

[3] https://wiki.osdev.org/System_V_ABI

[4] https://github.com/hjl-tools/x86-psABI/wiki/X86-psABI
