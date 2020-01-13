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
static const size_t SIZEOF_INT = 4; // size of int type
static const size_t SIZEOF_PTR = 8; // size of pointer type


/*
make a new type
*/
Type *new_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(Type));
    type->kind = kind;
    type->base = NULL;
    type->len = 1;

    switch(kind)
    {
    case TY_INT:
        type->size = SIZEOF_INT;
        break;

    case TY_PTR:
    case TY_ARRAY:
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
* This function assumes that array-type be pointer-type.
*/
bool is_pointer(const Node *node)
{
    return (node->type->kind == TY_PTR) || (node->type->kind == TY_ARRAY);
}