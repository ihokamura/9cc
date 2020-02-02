/*
test functions called from assembler code
*/

#include <stdio.h>
#include <stdlib.h>


#ifdef USE_EXTFUNC
extern int extfunc1();
extern int extfunc2(int a1, int a2, int a3, int a4, int a5, int a6, int a7);
#endif

// function without argument returning nothing
void testfunc1(void)
{
    printf("function call: testfunc1()\n");
}


// function with one argument returning nothing
void testfunc2(int a1)
{
    printf("function call: testfunc2(%d)\n", a1);
}


// function with multiple (less than or equal to 6) arguments returning nothing
void testfunc3(int a1, int a2)
{
    printf("function call: testfunc3(%d, %d)\n", a1, a2);
}

// function with multiple (less than or equal to 6) arguments returning an integer
int testfunc4(int a1, int a2, int a3, int a4, int a5, int a6)
{
    int r = 123;
    printf("function call: testfunc4(%d, %d, %d, %d, %d, %d) -> %d\n", a1, a2, a3, a4, a5, a6, r);
    return r;
}

// function with multiple (more than 6) arguments returning an integer
int testfunc5(int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
    int r = 123;
    printf("function call: testfunc5(%d, %d, %d, %d, %d, %d, %d) -> %d\n", a1, a2, a3, a4, a5, a6, a7, r);
    return r;
}

// function with multiple (more than 6) arguments returning an integer
int testfunc6(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
    int r = 123;
    printf("function call: testfunc6(%d, %d, %d, %d, %d, %d, %d, %d) -> %d\n", a1, a2, a3, a4, a5, a6, a7, a8, r);
    return r;
}

#ifdef USE_EXTFUNC
int testfunc7(void)
{
    int r;

    r = extfunc1();
    printf("extfunc1() -> r%d\n", r);

    r = extfunc2(1, 2, 3, 4, 5, 6, 7);
    printf("extfunc2(1, 2, 3, 4, 5, 6, 7) -> %d\n", r);

    return 123;
}
#endif

// print an int-type variable
void print_int(int n)
{
    printf("%d\n", n);
}


// allocate 4 integers
void alloc4(int **p, int a1, int a2, int a3, int a4)
{
    int *a = malloc(sizeof(int) * 4);

    a[0] = a1;
    a[1] = a2;
    a[2] = a3;
    a[3] = a4;
    *p = a;
}