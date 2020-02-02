# 9cc compiler


## Usage
```
9cc [ <file> | -c <code> ]
```


## Syntax
```
prg ::= (gvar | func)*
gvar ::= type-spec declarator ";"
func ::= type-spec declarator "(" (type-spec declarator ("," type-spec declarator)*)? ")" "{" stmt* "}"
stmt ::= declaration | expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "do" stmt "while" "(" expr ")" ";" | "for" "(" expr? ";" expr? ";" expr? ")" stmt | "{" stmt* "}"
declaration ::= type-spec declarator ";"
expr ::= assign
assign ::= equality ("=" assign)?
equality ::= relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= postfix | ("&" | "*" | "+" | "-")? unary | sizeof unary
postfix ::= primary ("[" expr "]")?
primary ::= num | str | ident ("(" (assign ("," assign)*)? ")")? | "(" expr ")"
type-spec ::= "char" | "int"
declarator ::= "*"* ident type-suffix
type-suffix ::= ("[" num "]" | type-suffix)?
```


## Reference
[1] https://www.sigbus.info/compilerbook

[2] https://www.bottlecaps.de/rr/ui

[3] https://wiki.osdev.org/System_V_ABI