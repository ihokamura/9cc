# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook

## Syntax
* `expr = equality`
* `equality = relational ("==" relational | "!=" relational)*`
* `relational = add ("<" add | "<=" add | ">" add | ">=" add)*`
* `add = mul ("+" mul | "-" mul)*`
* `mul = unary ("*" unary | "/" unary)*`
* `unary = ("+" | "-")? primary`
* `primary = num | "(" expr ")"`