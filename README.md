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
declaration ::= type-spec declarator ("=" initializer)? ";"
initializer ::= assign
expr ::= assign
assign ::= xor-expr (assign-op assign)?
assign-op ::= "=" | "+=" | "-=" | "*=" | "/="
xor-expr ::= and-expr (^ and-expr)*
and-expr ::= equality (& equality)*
equality ::= relational ("==" relational | "!=" relational)*
relational ::= shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
shift ::=  add ("<<" add | ">>" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary | "%" unary)*
unary ::= postfix
        | ("++" | "--") unary
        | unary-op unary
        | sizeof unary
unary-op ::= "&" | "*" | "+" | "-"
postfix ::= primary ("[" expr "]")* ("++" | "--" )?
primary ::= ident ("(" (assign ("," assign)*)? ")")?
          | num
          | str
          | "(" expr ")"
type-spec ::= "char" | "short" | "int" | "long"
declarator ::= "*"* ident type-suffix
type-suffix ::= ("[" num "]" | type-suffix)?
```


## Reference
[1] https://www.sigbus.info/compilerbook

[2] https://www.bottlecaps.de/rr/ui

[3] https://wiki.osdev.org/System_V_ABI