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
List(type) *new_list(type)(void)\
{\
    List(type) *list = calloc(1, sizeof(List(type)));\
    ListEntry(type) *dummy_entry = new_list_entry(type)(NULL);\
    dummy_entry->next = dummy_entry;\
    dummy_entry->prev = dummy_entry;\
    list->head = dummy_entry;\
    list->tail = dummy_entry;\
\
    return list;\
}\
\
\
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
/* get the element of an entry of list */\
type *get_element(type)(const ListEntry(type) *entry)\
{\
    return entry->element;\
}\
\
\
/* get the first entry of list */\
ListEntry(type) *get_first_entry(type)(const List(type) *list)\
{\
    return list->head->next;\
}\
\
\
/* check if iteration is over */\
bool end_iteration(type)(const List(type) *list, const ListEntry(type) *entry)\
{\
    return list->tail == entry;\
}\
\
\
/* add an entry at the head of list */\
ListEntry(type) *add_list_entry_head(type)(List(type) *list, type *element)\
{\
    ListEntry(type) *head = new_list_entry(type)(element);\
    head->prev = list->head;\
    head->next = list->head->next;\
    list->head->next = head;\
\
    return head;\
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
ListEntry(type) *add_list_entry_tail(type)(List(type) *list, type *element)\
{\
    ListEntry(type) *tail = new_list_entry(type)(element);\
    tail->prev = list->tail->prev;\
    tail->next = list->tail;\
    list->tail->prev->next = tail;\
    list->tail->prev = tail;\
\
    return tail;\
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
