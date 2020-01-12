# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
[2] https://www.bottlecaps.de/rr/ui

## Syntax
```
program ::= func*
func ::= "int" declarator "(" ("int" declarator ("," "int" declarator)*)? ")" "{" stmt* "}"
stmt ::= declaration | expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "do" stmt "while" "(" expr ")" ";" | "for" "(" expr? ";" expr? ";" expr? ")" stmt | "{" stmt* "}"
declaration ::= "int" declarator ";"
expr ::= assign
assign ::= equality ("=" assign)?
equality ::= relational ("==" relational | "!=" relational)*
relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary | "/" unary)*
unary ::= sizeof unary | ("+" | "-")? primary | "&" unary | "*" unary
primary ::= num | ident ("(" (expr ("," expr)*)? ")")? | "(" expr ")"
declarator ::= "*"* ident
```