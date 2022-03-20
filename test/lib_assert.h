#ifndef LIB_ASSERT_H
#define LIB_ASSERT_H

#include "configuration.h"

#define assert_equal_bool(expected, actual)  assert_equal_bool_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_char(expected, actual)  assert_equal_char_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_short(expected, actual)  assert_equal_short_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_int(expected, actual)  assert_equal_int_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_unsigned_int(expected, actual)  assert_equal_unsigned_int_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_long(expected, actual)  assert_equal_long_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_unsigned_long(expected, actual)  assert_equal_unsigned_long_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_long_long(expected, actual)  assert_equal_long_long_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_pointer(expected, actual)  assert_equal_pointer_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_float(expected, actual)  assert_equal_float_func(__FILE__, __LINE__, expected, actual)
#define assert_equal_double(expected, actual)  assert_equal_double_func(__FILE__, __LINE__, expected, actual)
#define assert_constant_decimal_all_digit(actual)  assert_constant_decimal_all_digit_func(__FILE__, __LINE__, actual)
#define assert_constant_octal_all_digit(actual)  assert_constant_octal_all_digit_func(__FILE__, __LINE__, actual)
#define assert_constant_hexadecimal_all_digit(actual)  assert_constant_hexadecimal_all_digit_func(__FILE__, __LINE__, actual)
#define assert_constant_int_max_minus1(actual)  assert_constant_int_max_minus1_func(__FILE__, __LINE__, actual)
#define assert_constant_int_max(actual)  assert_constant_int_max_func(__FILE__, __LINE__, actual)
#define assert_constant_int_max_plus1(actual)  assert_constant_int_max_plus1_func(__FILE__, __LINE__, actual)
#define assert_constant_unsigned_int_max_minus1(actual)  assert_constant_unsigned_int_max_minus1_func(__FILE__, __LINE__, actual)
#define assert_constant_unsigned_int_max(actual)  assert_constant_unsigned_int_max_func(__FILE__, __LINE__, actual)
#define assert_constant_unsigned_int_max_plus1(actual)  assert_constant_unsigned_int_max_plus1_func(__FILE__, __LINE__, actual)
#define assert_constant_long_max_minus1(actual)  assert_constant_long_max_minus1_func(__FILE__, __LINE__, actual)
#define assert_constant_long_max(actual)  assert_constant_long_max_func(__FILE__, __LINE__, actual)
#define assert_constant_long_max_plus1(actual)  assert_constant_long_max_plus1_func(__FILE__, __LINE__, actual)
#define assert_constant_unsigned_long_max_minus1(actual)  assert_constant_unsigned_long_max_minus1_func(__FILE__, __LINE__, actual)
#define assert_constant_unsigned_long_max(actual)  assert_constant_unsigned_long_max_func(__FILE__, __LINE__, actual)

#define assert_equal_size_t(expected, actual)    assert_equal_unsigned_long(expected, actual)
#define assert_size_of_expression(size, expression)    assert_equal_size_t(size, sizeof(expression))

// function declaration
void assert_equal_bool_func(const char *file, int line, _Bool expected, _Bool actual);
void assert_equal_char_func(const char *file, int line, char expected, char actual);
void assert_equal_short_func(const char *file, int line, short expected, short actual);
void assert_equal_int_func(const char *file, int line, int expected, int actual);
void assert_equal_unsigned_int_func(const char *file, int line, unsigned int expected, unsigned int actual);
void assert_equal_long_func(const char *file, int line, long expected, long actual);
void assert_equal_unsigned_long_func(const char *file, int line, unsigned long expected, unsigned long actual);
void assert_equal_long_long_func(const char *file, int line, long long expected, long long actual);
void assert_equal_pointer_func(const char *file, int line, const void *expected, const void *actual);
#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
void assert_equal_float_func(const char *file, int line, float expected, float actual);
void assert_equal_double_func(const char *file, int line, double expected, double actual);
#endif /* INCLUDE_FLOATING_POINT_TYPE */
void assert_constant_decimal_all_digit_func(const char *file, int line, long actual);
void assert_constant_octal_all_digit_func(const char *file, int line, long actual);
void assert_constant_hexadecimal_all_digit_func(const char *file, int line, long actual);
void assert_constant_int_max_minus1_func(const char *file, int line, long actual);
void assert_constant_int_max_func(const char *file, int line, long actual);
void assert_constant_int_max_plus1_func(const char *file, int line, long actual);
void assert_constant_unsigned_int_max_minus1_func(const char *file, int line, unsigned long actual);
void assert_constant_unsigned_int_max_func(const char *file, int line, unsigned long actual);
void assert_constant_unsigned_int_max_plus1_func(const char *file, int line, unsigned long actual);
void assert_constant_long_max_minus1_func(const char *file, int line, long actual);
void assert_constant_long_max_func(const char *file, int line, long actual);
void assert_constant_long_max_plus1_func(const char *file, int line, unsigned long actual);
void assert_constant_unsigned_long_max_minus1_func(const char *file, int line, unsigned long actual);
void assert_constant_unsigned_long_max_func(const char *file, int line, unsigned long actual);

#endif /* !LIB_ASSERT_H */
