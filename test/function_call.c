/*
test functions called from assembler code
*/

#include <stdio.h>
#include <stdlib.h>


// return 0
int func_call_return0()
{
    return 0;
}


// return 1
int func_call_return1()
{
    return 1;
}


// return 2
int func_call_return2()
{
    return 2;
}


// add 2 arguments
int func_call_add(int x, int y)
{
    return x + y;
}


// function with 6 arguments
int func_call_arg6(int a0, int a1, int a2, int a3, int a4, int a5)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000;
}


// function with 7 arguments
int func_call_arg7(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000;
}


// function with 8 arguments
int func_call_arg8(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000 + a7*10000000;
}


// allocate 4 integers
void alloc4(int **p, int a0, int a1, int a2, int a3)
{
    int *a = malloc(sizeof(int) * 4);

    a[0] = a0;
    a[1] = a1;
    a[2] = a2;
    a[3] = a3;
    *p = a;
}