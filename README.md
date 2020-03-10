# 9cc compiler


## Usage
```
9cc [ <file> | -c <code> ]
```


## Syntax
```
prg ::= (gvar | func)*
gvar ::= type-spec declarator ("=" initializer) ";"
func ::= declaration-spec declarator "(" ("void" | parameter-list)? ")" compound-stmt
stmt ::= ident ":" stmt
       | "case" num ":" stmt
       | "default" ":" stmt
       | compound-stmt
       | expr? ";"
       | "if" "(" expr ")" stmt ("else" stmt)?
       | "switch" "(" expr ")" stmt
       | "while" "(" expr ")" stmt
       | "do" stmt "while" "(" expr ")" ";"
       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
       | "goto" ident ";"
       | "continue" ";"
       | "break" ";"
       | "return" expr ";"
compound-stmt ::= "{" (declaration | stmt)* "}"
expr ::= assign ("," assign)*
assign ::= conditional (assign-op assign)?
assign-op ::= "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|="
conditional ::= logical-or-expr ("?" expr ":" conditional)?
logical-or-expr ::= logical-and-expr (|| logical-and-expr)*
logical-and-expr ::= or-expr (&& or-expr)*
or-expr ::= xor-expr (| xor-expr)*
xor-expr ::= and-expr (^ and-expr)*
and-expr ::= equality (& equality)*
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
postfix ::= primary ("[" expr "]" | "(" arg-expr-list? ")" | "++" | "--" )*
arg-expr-list ::= assign ("," assign)*
primary ::= ident
          | num
          | str
          | "(" expr ")"
declaration ::= declaration-spec init-declarator-list ";"
declaration-spec ::= type-spec
type-spec ::= "void" | "char" | "short" | "int" | "long"
init-declarator-list ::= init-declarator ("," init-declarator)*
init-declarator ::= declarator ("=" initializer)?
declarator ::= pointer? direct-declarator
direct-declarator ::= ident | direct-declarator "[" num "]"
parameter-list ::= parameter-declaration ("," parameter-declaration)*
parameter-declaration ::= declaration-spec declarator
initializer ::= assign
type-name ::= type-spec pointer?
pointer ::= "*" "*"*
```


## Reference
[1] https://www.sigbus.info/compilerbook

[2] https://www.bottlecaps.de/rr/ui

[3] https://wiki.osdev.org/System_V_ABI