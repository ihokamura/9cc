/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __STATEMENT_H__
#define __STATEMENT_H__

#include "9cc.h"

Statement *new_statement(StatementKind kind);
Statement *compound_statement(void);

#endif /* !__STATEMENT_H__ */
