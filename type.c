/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* type handler
*/

#include <stdlib.h>

#include "9cc.h"


// global variable
static const size_t SIZEOF_INT = 4;
static const size_t SIZEOF_PTR = 8;


/*
make a new type
*/
Type *new_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(Type));
    type->ty = kind;
    type->ptr_to = NULL;

    switch(kind)
    {
    case TY_INT:
        type->size = SIZEOF_INT;
        break;

    case TY_PTR:
        type->size = SIZEOF_PTR;
        break;

    default:
        type->size = SIZEOF_INT;
        break;
    }

    return type;
}


/*
check if a given node is pointer-type
*/
bool is_pointer(const Node *node)
{
    return (node->lvar != NULL) && (node->lvar->type->ty == TY_PTR);
}