# 9cc compiler


## Usage
```
9cc [ <file> | -c <code> ]
```


## Syntax
```
prg ::= (gvar | func)*
gvar ::= type-spec declarator ("=" initializer) ";"
func ::= type-spec declarator "(" (type-spec declarator ("," type-spec declarator)*)? ")" "{" stmt* "}"
stmt ::= ident ":" stmt
       | "case" num ":" stmt
       | "default" ":" stmt
       | declaration
       | "{" stmt* "}"
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
declaration ::= type-spec declarator ("=" initializer)? ";"
type-spec ::= "void" | "char" | "short" | "int" | "long"
declarator ::= pointer? direct-declarator
direct-declarator ::= ident | direct-declarator "[" num "]"
initializer ::= assign
type-name ::= type-spec pointer?
pointer ::= "*" "*"*
```


## Reference
[1] https://www.sigbus.info/compilerbook

[2] https://www.bottlecaps.de/rr/ui

[3] https://wiki.osdev.org/System_V_ABI