#include <stdio.h>
#include <stdlib.h>
#include "lib_assert.h"


/*
assertion for bool type
*/
int assert_bool_func(const char *file, int line, _Bool expected, _Bool actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %d expected, but got %d\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for char type
*/
int assert_char_func(const char *file, int line, char expected, char actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %d expected, but got %d\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for short type
*/
int assert_short_func(const char *file, int line, short expected, short actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %d expected, but got %d\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for int type
*/
int assert_int_func(const char *file, int line, int expected, int actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %d expected, but got %d\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for long type
*/
int assert_long_func(const char *file, int line, long expected, long actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %ld expected, but got %ld\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for long long type
*/
int assert_longlong_func(const char *file, int line, long long expected, long long actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %lld expected, but got %lld\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for pointer type
*/
int assert_pointer_func(const char *file, int line, const void *expected, const void *actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        
        printf("%s(%d): %p expected, but got %p\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
/*
assertion for float type
*/
int assert_float_func(const char *file, int line, float expected, float actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %f expected, but got %f\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for double type
*/
int assert_double_func(const char *file, int line, double expected, double actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%s(%d): %f expected, but got %f\n", file, line, expected, actual);
        exit(1);
        return 1;
    }
}
#endif /* INCLUDE_FLOATING_POINT_TYPE */
