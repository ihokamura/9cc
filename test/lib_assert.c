#include <stdio.h>
#include <stdlib.h>
#include "lib_assert.h"


#define assert_equal_integer(file, line, expected, actual) do \
{\
    if((expected) != (actual)) \
    { \
        const char *format = _Generic((expected), \
            unsigned: "%s(%d): %u expected, but got %u\n", \
            long: "%s(%d): %ld expected, but got %ld\n", \
            unsigned long: "%s(%d): %lu expected, but got %lu\n", \
            long long: "%s(%d): %lld expected, but got %lld\n", \
            unsigned long long: "%s(%d): %llu expected, but got %llu\n", \
            const void *: "%s(%d): %p expected, but got %p\n", \
            default: "%s(%d): %d expected, but got %d\n"); \
        printf(format, file, line, expected, actual); \
        exit(1); \
    } \
} while(0); \
return ((expected) == (actual)) ? 0 : 1

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
#define assert_equal_floating_point(file, line, expected, actual) do \
{\
    if((expected) != (actual)) \
    { \
        const char *format = "%s(%d): %f expected, but got %f\n"; \
        printf(format, file, line, expected, actual); \
        exit(1); \
    } \
} while(0); \
return ((expected) == (actual)) ? 0 : 1
#endif /* INCLUDE_FLOATING_POINT_TYPE */


/*
assertion for bool type
*/
int assert_equal_bool_func(const char *file, int line, _Bool expected, _Bool actual)
{
    assert_equal_integer(file, line, expected, actual);
}


/*
assertion for char type
*/
int assert_equal_char_func(const char *file, int line, char expected, char actual)
{
    assert_equal_integer(file, line, expected, actual);
}


/*
assertion for short type
*/
int assert_equal_short_func(const char *file, int line, short expected, short actual)
{
    assert_equal_integer(file, line, expected, actual);
}


/*
assertion for int type
*/
int assert_equal_int_func(const char *file, int line, int expected, int actual)
{
    assert_equal_integer(file, line, expected, actual);
}


/*
assertion for long type
*/
int assert_equal_long_func(const char *file, int line, long expected, long actual)
{
    assert_equal_integer(file, line, expected, actual);
}


/*
assertion for long long type
*/
int assert_equal_long_long_func(const char *file, int line, long long expected, long long actual)
{
    assert_equal_integer(file, line, expected, actual);
}


/*
assertion for pointer type
*/
int assert_equal_pointer_func(const char *file, int line, const void *expected, const void *actual)
{
    assert_equal_integer(file, line, expected, actual);
}


#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
/*
assertion for float type
*/
int assert_equal_float_func(const char *file, int line, float expected, float actual)
{
    assert_equal_floating_point(file, line, expected, actual);
}


/*
assertion for double type
*/
int assert_equal_double_func(const char *file, int line, double expected, double actual)
{
    assert_equal_floating_point(file, line, expected, actual);
}
#endif /* INCLUDE_FLOATING_POINT_TYPE */
