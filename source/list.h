/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#ifndef __LIST_H__
#define __LIST_H__

#include "9cc.h"


// macro
#define List(type) type##List // type-name of list
#define ListEntry(type) type##ListEntry // type-name of entry of list
#define new_list(type) new_##type##List // function name to make a new list
#define new_list_entry(type) new_##type##ListEntry // function name to make a new entry of list
#define get_element(type) get_entry_##type // function name to get the element of an entry of list
#define get_first_entry(type) get_first_entry_##type // function name to get the first entry of list
#define get_first_element(type) get_first_element_##type // function name to get the element of the first entry of list
#define get_last_entry(type) get_last_entry_##type // function name to get the last entry of list
#define set_first_entry(type) set_first_entry_##type // function name to set the first entry of list
#define end_iteration(type) end_iteration_##type // function name to check if iteration is over
#define add_list_entry_head(type) add_list_entry_head_##type // function name to add an entry at the head of list
#define add_entry_head(type) add_entry_head_##type // function name to add an entry at the head of list
#define add_list_entry_tail(type) add_list_entry_tail_##type // function name to add an entry at the tail of list
#define add_entry_tail(type) add_entry_tail_##type // function name to add an entry at the tail of list
#define concatenate_list(type) concatenate_list_##type // function name to concatenate lists
#define for_each(type, cursor, list) for(ListEntry(type) *cursor = list; cursor != NULL; cursor = cursor->next) // iterator over list
#define for_each_entry(type, cursor, list) for(ListEntry(type) *cursor = get_first_entry(type)(list); !end_iteration(type)(list, cursor); cursor = cursor->next) // iterator over list

#define define_list(type) \
/* definition of structure */\
typedef struct List(type) List(type);\
typedef struct ListEntry(type) ListEntry(type);\
struct List(type) {\
    ListEntry(type) *head; /* first entry of list */\
    ListEntry(type) *tail; /* last entry of list */\
};\
struct ListEntry(type) {\
    ListEntry(type) *prev; /* pointer to the previous element */\
    ListEntry(type) *next; /* pointer to the next element */\
    type *element;         /* element of list */\
};\
/* function prototypes */\
List(type) *new_list(type)(void);\
ListEntry(type) *new_list_entry(type)(type *element);\
type *get_element(type)(const ListEntry(type) *entry);\
ListEntry(type) *get_first_entry(type)(const List(type) *list);\
type *get_first_element(type)(const List(type) *list);\
ListEntry(type) *get_last_entry(type)(const List(type) *list);\
void set_first_entry(type)(List(type) *list, ListEntry(type) *entry);\
bool end_iteration(type)(const List(type) *list, const ListEntry(type) *entry);\
ListEntry(type) *add_list_entry_head(type)(List(type) *list, type *element);\
ListEntry(type) *add_entry_head(type)(ListEntry(type) *list, type *element);\
ListEntry(type) *add_list_entry_tail(type)(List(type) *list, type *element);\
ListEntry(type) *add_entry_tail(type)(ListEntry(type) *list, type *element);\
List(type) *concatenate_list(type)(List(type) *self, List(type) *other);\


define_list(Token)
define_list(Initializer)
define_list(DataSegment)
define_list(Member)
define_list(Type)
define_list(Variable)
define_list(StringLiteral)
define_list(Expression)
define_list(Declaration)
define_list(Statement)
define_list(Function)
define_list(Identifier)
define_list(Tag)

#endif /* !__LIST_H__ */
