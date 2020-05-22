/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* list handler
*/

#include <stdlib.h>

#include "9cc.h"


// macro
#define define_list_operations(type) \
/* make a new list */\
List(type) *new_list(type)(type *element)\
{\
    List(type) *list = calloc(1, sizeof(List(type)));\
    list->next = NULL;\
    list->element = element;\
\
    return list;\
}\
/* get the entry of list */\
type *get_entry(type)(const List(type) *list)\
{\
    return list->element;\
}\


define_list_operations(Type)
define_list_operations(Variable)
