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


// function with structure (classified as MEMORY)
struct param_t1 {long m0; long m1; char m2;};
long func_call_struct1(struct param_t1 s)
{
    return s.m0 + s.m1*10 + s.m2*100;
}


// function with structure (classified as INTEGER and passed by registers)
struct param_t2 {int m0; char m1; long m2;};
long func_call_struct2(struct param_t2 s)
{
    return s.m0 + s.m1*10 + s.m2*100;
}


// function with structure (classified as INTEGER and passed by the stack)
struct param_t3 {int m0; char m1; long m2;};
long func_call_struct3(int a0, int a1, int a2, int a3, int a4, struct param_t3 s, int a5, int a6)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000 + 10000000*(s.m0 + s.m1*10 + s.m2*100);
}


// function with structure (classified as INTEGER and passed by registers)
struct param_t4 {char m0; char m1; char m2;};
long func_call_struct4(struct param_t4 s)
{
    return s.m0 + s.m1*10 + s.m2*100;
}


// function with structure (classified as INTEGER and returned by 1 register)
struct param_t5 {char m0; char m1; int m2;};
struct param_t5 func_call_struct5(char a0, char a1, int a2)
{
    struct param_t5 s = {a0, a1, a2};

    return s;
}

// function with structure (classified as INTEGER and returned by 2 registers)
struct param_t6 {long m0; long m1;};
struct param_t6 func_call_struct6(long a0, long a1)
{
    struct param_t6 s = {a0, a1};

    return s;
}


// function with structure (classified as MEMORY and returned by the hidden argument)
struct param_t7 {long m0; long m1; char m2;};
struct param_t7 func_call_struct7(long a0, long a1, char a2)
{
    struct param_t7 s = {a0, a1, a2};

    return s;
}


// function with structure (classified as MEMORY and returned by the hidden argument)
struct param_t8 {long m0; long m1; long m2;};
struct param_t8 func_call_struct8(long a0, long a1, long a2, int a3, int a4, int a5, int a6)
{
    struct param_t8 s = {a0 + a3 + a6, a1 + a4, a2 + a5};

    return s;
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