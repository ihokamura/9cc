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
/* make a new entry of list */\
ListEntry(type) *new_list_entry(type)(type *element)\
{\
    ListEntry(type) *entry = calloc(1, sizeof(ListEntry(type)));\
    entry->next = NULL;\
    entry->element = element;\
\
    return entry;\
}\
\
\
/* get the entry of list */\
type *get_element(type)(const ListEntry(type) *entry)\
{\
    return entry->element;\
}\
\
\
/* add an entry at the head of list */\
ListEntry(type) *add_entry_head(type)(ListEntry(type) *list, type *element)\
{\
    ListEntry(type) *head = new_list_entry(type)(element);\
    head->next = list;\
\
    return head;\
}\
\
\
/* add an entry at the tail of list */\
ListEntry(type) *add_entry_tail(type)(ListEntry(type) *list, type *element)\
{\
    ListEntry(type) *tail = new_list_entry(type)(element);\
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
