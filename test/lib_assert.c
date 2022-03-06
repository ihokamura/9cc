#include <stdio.h>
#include <stdlib.h>
#include "lib_assert.h"


#define assert_equal_integer(file, line, expected, actual) do \
{\
    if((expected) != (actual)) \
    { \
        const char *format = _Generic((actual), \
            unsigned int: "%s(%d): %u expected, but got %u\n", \
            long: "%s(%d): %ld expected, but got %ld\n", \
            unsigned long: "%s(%d): %lu expected, but got %lu\n", \
            long long: "%s(%d): %lld expected, but got %lld\n", \
            unsigned long long: "%s(%d): %llu expected, but got %llu\n", \
            const void *: "%s(%d): %p expected, but got %p\n", \
            default: "%s(%d): %d expected, but got %d\n"); \
        printf(format, file, line, expected, actual); \
        exit(1); \
    } \
} while(0)

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
#define assert_equal_floating_point(file, line, expected, actual) do \
{\
    if((expected) != (actual)) \
    { \
        const char *format = "%s(%d): %f expected, but got %f\n"; \
        printf(format, file, line, expected, actual); \
        exit(1); \
    } \
} while(0)
#endif /* INCLUDE_FLOATING_POINT_TYPE */


// assertion for bool type
void assert_equal_bool_func(const char *file, int line, _Bool expected, _Bool actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for char type
void assert_equal_char_func(const char *file, int line, char expected, char actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for short type
void assert_equal_short_func(const char *file, int line, short expected, short actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for int type
void assert_equal_int_func(const char *file, int line, int expected, int actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for unsigned int type
void assert_equal_unsigned_int_func(const char *file, int line, unsigned int expected, unsigned int actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for long type
void assert_equal_long_func(const char *file, int line, long expected, long actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for unsigned long type
void assert_equal_unsigned_long_func(const char *file, int line, unsigned long expected, unsigned long actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for long long type
void assert_equal_long_long_func(const char *file, int line, long long expected, long long actual)
{
    assert_equal_integer(file, line, expected, actual);
}


// assertion for pointer type
void assert_equal_pointer_func(const char *file, int line, const void *expected, const void *actual)
{
    assert_equal_integer(file, line, expected, actual);
}


#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
// assertion for float type
void assert_equal_float_func(const char *file, int line, float expected, float actual)
{
    assert_equal_floating_point(file, line, expected, actual);
}


// assertion for double type
void assert_equal_double_func(const char *file, int line, double expected, double actual)
{
    assert_equal_floating_point(file, line, expected, actual);
}
#endif /* INCLUDE_FLOATING_POINT_TYPE */


// assertion for decimal constant with all digits
void assert_constant_decimal_all_digit_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 1234567890, actual);
}


// assertion for octal constant with all digits
void assert_constant_octal_all_digit_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 012345670, actual);
}


// assertion for hexadecimal constant with all digits
void assert_constant_hexadecimal_all_digit_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 0x1234567890ABCDEF, actual);
}


// assertion for maximum of int type minus 1
void assert_constant_int_max_minus1_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 2147483646, actual);
}


// assertion for maximum of int type
void assert_constant_int_max_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 2147483647, actual);
}


// assertion for maximum of int type plus 1
void assert_constant_int_max_plus1_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 2147483648L, actual);
}


// assertion for maximum of unsigned int type minus 1
void assert_constant_unsigned_int_max_minus1_func(const char *file, int line, unsigned long actual)
{
    assert_equal_integer(file, line, 4294967294U, actual);
}


// assertion for maximum of unsigned int type
void assert_constant_unsigned_int_max_func(const char *file, int line, unsigned long actual)
{
    assert_equal_integer(file, line, 4294967295U, actual);
}


// assertion for maximum of unsigned int type plus 1
void assert_constant_unsigned_int_max_plus1_func(const char *file, int line, unsigned long actual)
{
    assert_equal_integer(file, line, 4294967296U, actual);
}


// assertion for maximum of long type minus 1
void assert_constant_long_max_minus1_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 9223372036854775806L, actual);
}


// assertion for maximum of long type
void assert_constant_long_max_func(const char *file, int line, long actual)
{
    assert_equal_integer(file, line, 9223372036854775807L, actual);
}


// assertion for maximum of long type plus 1
void assert_constant_long_max_plus1_func(const char *file, int line, unsigned long actual)
{
    assert_equal_integer(file, line, 9223372036854775808UL, actual);
}


// assertion for maximum of unsigned long type minus 1
void assert_constant_unsigned_long_max_minus1_func(const char *file, int line, unsigned long actual)
{
    assert_equal_integer(file, line, 18446744073709551614UL, actual);
}


// assertion for maximum of unsigned long type
void assert_constant_unsigned_long_max_func(const char *file, int line, unsigned long actual)
{
    assert_equal_integer(file, line, 18446744073709551615UL, actual);
}
