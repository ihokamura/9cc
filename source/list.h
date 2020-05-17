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

#define define_list(type) \
/* definition of structure */\
typedef struct List(type) List(type);\
struct List(type) {\
    List(type) *next; /* pointer to the next element */\
    type *element;    /* element of list */\
};\
/* function prototypes */\
List(type) *new_list(type)(type *element);\


define_list(Type)

#endif /* !__LIST_H__ */
