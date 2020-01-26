# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
[2] https://www.bottlecaps.de/rr/ui

## Syntax
```
prg ::= (gvar | func)*
gvar ::= "int" declarator ";"
func ::= "int" declarator "(" ("int" declarator ("," "int" declarator)*)? ")" "{" stmt* "}"
stmt ::= declaration | expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "do" stmt "while" "(" expr ")" ";" | "for" "(" expr? ";" expr? ";" expr? ")" stmt | "{" stmt* "}"
declaration ::= "int" declarator ";"
expr ::= assign
assign ::= equality ("=" assign)?
equality ::= relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= postfix | ("&" | "*" | "+" | "-")? unary | sizeof unary
postfix ::= primary ("[" expr "]")?
primary ::= num | ident ("(" (expr ("," expr)*)? ")")? | "(" expr ")"
declarator ::= "*"* ident type-suffix
type-suffix ::= ("[" num "]" | type-suffix)?
```