#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "../common/check.h"
#include "../common/configuration.h"

// type definition
struct param_t1 {long m0; long m1; char m2;};
struct param_t2 {int m0; char m1; long m2;};
struct param_t3 {int m0; char m1; long m2;};
struct param_t4 {char m0; char m1; char m2;};
struct param_t5 {char m0; char m1; int m2;};
struct param_t6 {long m0; long m1;};
struct param_t7 {long m0; long m1; char m2;};
struct param_t8 {long m0; long m1; long m2;};

// function declaration
int func_call_return0();
int func_call_return1();
int func_call_return2();
int func_call_add(int x, int y);
int func_call_arg6(int a0, int a1, int a2, int a3, int a4, int a5);
int func_call_arg7(int a0, int a1, int a2, int a3, int a4, int a5, int a6);
int func_call_arg8(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7);
long func_call_struct1(struct param_t1 s);
long func_call_struct2(struct param_t2 s);
long func_call_struct3(int a0, int a1, int a2, int a3, int a4, struct param_t3 s, int a5, int a6);
long func_call_struct4(struct param_t4 s);
struct param_t5 func_call_struct5(char a0, char a1, int a2);
struct param_t6 func_call_struct6(long a0, long a1);
struct param_t7 func_call_struct7(long a0, long a1, char a2);
struct param_t8 func_call_struct8(long a0, long a1, long a2, int a3, int a4, int a5, int a6);
int func_call_arg_array(int a[10]);
void alloc4(int **p, int a0, int a1, int a2, int a3);
int func_call_variadic(int count, ...);
#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
float func_call_return_float(double f);
double func_call_return_double(double d);
void func_call_arg8_double(double a0, double a1, double a2, double a3, double a4, double a5, double a6, double a7);
void func_call_arg9_double(double a0, double a1, double a2, double a3, double a4, double a5, double a6, double a7, double a8);
#endif /* INCLUDE_FLOATING_POINT_TYPE */

#endif /* FUNCTION_CALL_H */
