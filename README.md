# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook

## Syntax
* `program = func*`
* `func = ident "(" (ident ("," ident)*)? ")" "{" stmt* "}"`
* `* stmt = expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "do" stmt "while" "(" expr ")" ";" | "for" "(" expr? ";" expr? ";" expr? ")" stmt | "{" stmt* "}"`
* `expr = assign`
* `assign = equality ("=" assign)?`
* `equality = relational ("==" relational | "!=" relational)*`
* `relational = add ("<" add | "<=" add | ">" add | ">=" add)*`
* `add = mul ("+" mul | "-" mul)*`
* `mul = unary ("*" unary | "/" unary)*`
* `unary = ("+" | "-")? primary | "&" unary | "*" unary`
* `primary = num | ident ("(" (expr ("," expr)*)? ")")? | "(" expr ")"`