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
#define new_list(type) new_##type##List // function name to make a new list
#define get_entry(type) get_entry_##type // function name to get the entry of list
#define add_entry_head(type) add_entry_head_##type // function name to add an entry at the head of list
#define add_entry_tail(type) add_entry_tail_##type // function name to add an entry at the tail of list
#define for_each(type, cursor, list) for(List(type) *cursor = list; cursor != NULL; cursor = cursor->next) // iterator over list

#define define_list(type) \
/* definition of structure */\
typedef struct List(type) List(type);\
struct List(type) {\
    List(type) *next; /* pointer to the next element */\
    type *element;    /* element of list */\
};\
/* function prototypes */\
List(type) *new_list(type)(type *element);\
type *get_entry(type)(const List(type) *list);\
List(type) *add_entry_head(type)(List(type) *list, type *element);\
List(type) *add_entry_tail(type)(List(type) *list, type *element);\


define_list(Type)
define_list(Variable)
define_list(StringLiteral)
define_list(Expression)
define_list(Function)
define_list(Identifier)
define_list(Tag)

#endif /* !__LIST_H__ */
