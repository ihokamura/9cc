/*
common header file for test code
*/

#ifndef __TEST_CODE_H__
#define __TEST_CODE_H__

// indicator of feature enabled or disabled
#define ENABLED     (1)
#define DISABLED    (0)

// features
#define INCLUDE_FLOATING_POINT_TYPE    (ENABLED) // include tests on floating-point types

#define assert_bool(expected, actual)  assert_bool_func(__FILE__, __LINE__, expected, actual)
#define assert_char(expected, actual)  assert_char_func(__FILE__, __LINE__, expected, actual)
#define assert_short(expected, actual)  assert_short_func(__FILE__, __LINE__, expected, actual)
#define assert_int(expected, actual)  assert_int_func(__FILE__, __LINE__, expected, actual)
#define assert_long(expected, actual)  assert_long_func(__FILE__, __LINE__, expected, actual)
#define assert_longlong(expected, actual)  assert_longlong_func(__FILE__, __LINE__, expected, actual)
#define assert_pointer(expected, actual)  assert_pointer_func(__FILE__, __LINE__, expected, actual)
#define assert_float(expected, actual)  assert_float_func(__FILE__, __LINE__, expected, actual)
#define assert_double(expected, actual)  assert_double_func(__FILE__, __LINE__, expected, actual)

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
int assert_bool_func(const char *file, int line, _Bool expected, _Bool actual);
int assert_char_func(const char *file, int line, char expected, char actual);
int assert_short_func(const char *file, int line, short expected, short actual);
int assert_int_func(const char *file, int line, int expected, int actual);
int assert_long_func(const char *file, int line, long expected, long actual);
int assert_longlong_func(const char *file, int line, long long expected, long long actual);
int assert_pointer_func(const char *file, int line, const void *expected, const void *actual);
int assert_float_func(const char *file, int line, float expected, float actual);
int assert_double_func(const char *file, int line, double expected, double actual);

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

#endif /* !__TEST_CODE_H__ */
