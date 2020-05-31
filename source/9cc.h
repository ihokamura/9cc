/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __9CC_H__
#define __9CC_H__

#include <stdbool.h>
#include <stddef.h>

// indicator of feature enabled or disabled
#define ENABLED     (1)
#define DISABLED    (0)

// features
#define WARN_IMPLICIT_DECLARATION_OF_FUNCTION    (ENABLED) // warn implicit declaration of function

// forward declaration of structure types
typedef struct DataSegment DataSegment;
typedef struct Declaration Declaration;
typedef struct Enumerator Enumerator;
typedef struct Expression Expression;
typedef struct Function Function;
typedef struct Identifier Identifier;
typedef struct Member Member;
typedef struct Program Program;
typedef struct Scope Scope;
typedef struct Statement Statement;
typedef struct StringLiteral StringLiteral;
typedef struct Tag Tag;
typedef struct Token Token;
typedef struct Type Type;
typedef struct Variable Variable;

// definition of list
#include "list.h"
define_list(DataSegment)
define_list(Declaration)
define_list(Expression)
define_list(Function)
define_list(Identifier)
define_list(Member)
define_list(Statement)
define_list(StringLiteral)
define_list(Tag)
define_list(Type)
define_list(Variable)

#endif /* !__9CC_H__ */
