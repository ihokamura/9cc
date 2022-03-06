#ifndef LIB_ASSERT_H
#define LIB_ASSERT_H

#include "configuration.h"

#define assert_equal_bool(expected, actual)  assert_equal_bool_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_char(expected, actual)  assert_equal_char_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_short(expected, actual)  assert_equal_short_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_int(expected, actual)  assert_equal_int_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_long(expected, actual)  assert_equal_long_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_long_long(expected, actual)  assert_equal_long_long_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_pointer(expected, actual)  assert_equal_pointer_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_float(expected, actual)  assert_equal_float_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_double(expected, actual)  assert_equal_double_func(__FILE__, __LINE__, expected, actual)

// function declaration
void assert_equal_bool_func(const char *file, int line, _Bool expected, _Bool actual);
void assert_equal_char_func(const char *file, int line, char expected, char actual);
void assert_equal_short_func(const char *file, int line, short expected, short actual);
void assert_equal_int_func(const char *file, int line, int expected, int actual);
void assert_equal_long_func(const char *file, int line, long expected, long actual);
void assert_equal_long_long_func(const char *file, int line, long long expected, long long actual);
void assert_equal_pointer_func(const char *file, int line, const void *expected, const void *actual);
#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
void assert_equal_float_func(const char *file, int line, float expected, float actual);
void assert_equal_double_func(const char *file, int line, double expected, double actual);
#endif /* INCLUDE_FLOATING_POINT_TYPE */

#endif /* !LIB_ASSERT_H */
