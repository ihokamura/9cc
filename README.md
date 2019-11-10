# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook

## Syntax
* `program = stmt*`
* `stmt = expr ";" | "return" expr ";" | "if" "(" expr ")" stmt ("else" stmt)?`
* `expr = assign`
* `assign = equality ("=" assign)?`
* `equality = relational ("==" relational | "!=" relational)*`
* `relational = add ("<" add | "<=" add | ">" add | ">=" add)*`
* `add = mul ("+" mul | "-" mul)*`
* `mul = unary ("*" unary | "/" unary)*`
* `unary = ("+" | "-")? primary`
* `primary = num | ident | "(" expr ")"`