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
\
\
/* get the entry of list */\
type *get_entry(type)(const List(type) *list)\
{\
    return list->element;\
}\
\
\
/* add an entry at the head of list */\
List(type) *add_entry_head(type)(List(type) *list, type *element)\
{\
    List(type) *head = new_list(type)(element);\
    head->next = list;\
\
    return head;\
}\
\
\
/* add an entry at the tail of list */\
List(type) *add_entry_tail(type)(List(type) *list, type *element)\
{\
    List(type) *tail = new_list(type)(element);\
    list->next = tail;\
\
    return tail;\
}\


define_list_operations(Member)
define_list_operations(Type)
define_list_operations(Variable)
define_list_operations(StringLiteral)
define_list_operations(Expression)
define_list_operations(Function)
define_list_operations(Identifier)
define_list_operations(Tag)
