#include "test_code.h"

#if ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS)
// 6.5.6 Additive operators
void test_additive_operators()
{
    put_title("Additive operators");

#define assert_arithmetic_addition(lhs, rhs, result, size, assert_function)    do { \
    assert_size_of_expression(size, lhs + rhs); \
    assert_size_of_expression(size, rhs + lhs); \
    assert_function(result, lhs + rhs); \
    assert_function(result, rhs + lhs); \
    } while(0)
#define test_arithmetic_addition(type, lhs, rhs, result, size, assert_function)    do { \
    type lhs_var = (type)lhs; \
    type rhs_var = (type)rhs; \
    assert_arithmetic_addition((type)lhs, (type)rhs, result, size, assert_function); \
    assert_arithmetic_addition((type)lhs, rhs_var, result, size, assert_function); \
    assert_arithmetic_addition(lhs_var, (type)rhs, result, size, assert_function); \
    assert_arithmetic_addition(lhs_var, rhs_var, result, size, assert_function); \
    } while(0)

    test_arithmetic_addition(_Bool, 0, 0, 0, 4, assert_equal_int);
    test_arithmetic_addition(_Bool, 0, 1, 1, 4, assert_equal_int);
    test_arithmetic_addition(_Bool, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_addition(_Bool, 1, 1, 2, 4, assert_equal_int);
    test_arithmetic_addition(char, 126, 0, 126, 4, assert_equal_int);
    test_arithmetic_addition(char, 126, 1, 127, 4, assert_equal_int);
    test_arithmetic_addition(char, 126, 2, 128, 4, assert_equal_int);
    test_arithmetic_addition(signed char, 126, 0, 126, 4, assert_equal_int);
    test_arithmetic_addition(signed char, 126, 1, 127, 4, assert_equal_int);
    test_arithmetic_addition(signed char, 126, 2, 128, 4, assert_equal_int);
    test_arithmetic_addition(unsigned char, 254, 0, 254, 4, assert_equal_int);
    test_arithmetic_addition(unsigned char, 254, 1, 255, 4, assert_equal_int);
    test_arithmetic_addition(unsigned char, 254, 2, 256, 4, assert_equal_int);
    test_arithmetic_addition(short, 32766, 0, 32766, 4, assert_equal_int);
    test_arithmetic_addition(short, 32766, 1, 32767, 4, assert_equal_int);
    test_arithmetic_addition(short, 32766, 2, 32768, 4, assert_equal_int);
    test_arithmetic_addition(unsigned short, 65534, 0, 65534, 4, assert_equal_int);
    test_arithmetic_addition(unsigned short, 65534, 1, 65535, 4, assert_equal_int);
    test_arithmetic_addition(unsigned short, 65534, 2, 65536, 4, assert_equal_int);
    test_arithmetic_addition(int, 2147483646, 0, 2147483646, 4, assert_equal_int);
    test_arithmetic_addition(int, 2147483646, 1, 2147483647, 4, assert_equal_int);
    // test_arithmetic_addition(int, 2147483646, 2, -2147483648, 4, assert_equal_int); // This is implementation-defined or raises an implementation-defined signal.
    test_arithmetic_addition(long, 2147483646L, 2L, 2147483648L, 8, assert_equal_long);
    test_arithmetic_addition(unsigned int, 4294967294U, 0U, 4294967294U, 4, assert_equal_unsigned_int);
    test_arithmetic_addition(unsigned int, 4294967294U, 1U, 4294967295U, 4, assert_equal_unsigned_int);
    test_arithmetic_addition(unsigned int, 4294967294U, 2U, 0U, 4, assert_equal_unsigned_int);
    test_arithmetic_addition(long, 4294967294U, 2U, 4294967296L, 8, assert_equal_long);
    test_arithmetic_addition(long, 9223372036854775806L, 0L, 9223372036854775806L, 8, assert_equal_long);
    test_arithmetic_addition(long, 9223372036854775806L, 1L, 9223372036854775807L, 8, assert_equal_long);
    // test_arithmetic_addition(long, 9223372036854775806L, 2L, -9223372036854775808L, 8, assert_equal_long); // This is implementation-defined or raises an implementation-defined signal.
    test_arithmetic_addition(unsigned long, 9223372036854775806UL, 2UL, 9223372036854775808UL, 8, assert_equal_unsigned_long);
    test_arithmetic_addition(unsigned long, 18446744073709551614UL, 0UL, 18446744073709551614UL, 8, assert_equal_unsigned_long);
    test_arithmetic_addition(unsigned long, 18446744073709551614UL, 1UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);
    test_arithmetic_addition(unsigned long, 18446744073709551614UL, 2UL, 0UL, 8, assert_equal_unsigned_long);

#undef test_arithmetic_addition
#undef assert_arithmetic_addition

#define assert_arithmetic_subtraction(lhs, rhs, result, size, assert_function)    do { \
    assert_size_of_expression(size, lhs - rhs); \
    assert_function(result, lhs - rhs); \
    } while(0)
#define test_arithmetic_subtraction(type, lhs, rhs, result, size, assert_function)    do { \
    type lhs_var = (type)lhs; \
    type rhs_var = (type)rhs; \
    assert_arithmetic_subtraction((type)lhs, (type)rhs, result, size, assert_function); \
    assert_arithmetic_subtraction((type)lhs, rhs_var, result, size, assert_function); \
    assert_arithmetic_subtraction(lhs_var, (type)rhs, result, size, assert_function); \
    assert_arithmetic_subtraction(lhs_var, rhs_var, result, size, assert_function); \
    } while(0)

    test_arithmetic_subtraction(_Bool, 0, 0, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(_Bool, 0, 1, -1, 4, assert_equal_int);
    test_arithmetic_subtraction(_Bool, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_subtraction(_Bool, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(char, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_subtraction(char, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(char, 1, 2, -1, 4, assert_equal_int);
    test_arithmetic_subtraction(signed char, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_subtraction(signed char, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(signed char, 1, 2, -1, 4, assert_equal_int);
    test_arithmetic_subtraction(signed char, -127, 0, -127, 4, assert_equal_int);
    test_arithmetic_subtraction(signed char, -127, 1, -128, 4, assert_equal_int);
    test_arithmetic_subtraction(signed char, -127, 2, -129, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned char, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned char, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned char, 1, 2, -1, 4, assert_equal_int);
    test_arithmetic_subtraction(short, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_subtraction(short, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(short, 1, 2, -1, 4, assert_equal_int);
    test_arithmetic_subtraction(short, -32767, 0, -32767, 4, assert_equal_int);
    test_arithmetic_subtraction(short, -32767, 1, -32768, 4, assert_equal_int);
    test_arithmetic_subtraction(short, -32767, 2, -32769, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned short, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned short, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned short, 1, 2, -1, 4, assert_equal_int);
    test_arithmetic_subtraction(int, 1, 0, 1, 4, assert_equal_int);
    test_arithmetic_subtraction(int, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_subtraction(int, 1, 2, -1, 4, assert_equal_int);
    test_arithmetic_subtraction(int, -2147483647, 0, -2147483647, 4, assert_equal_int);
    test_arithmetic_subtraction(int, -2147483647, 1, -2147483648, 4, assert_equal_int);
    // test_arithmetic_subtraction(int, -2147483647, 2, 2147483647, 4, assert_equal_int); // This is implementation-defined or raises an implementation-defined signal.
    test_arithmetic_subtraction(unsigned int, 1U, 0U, 1U, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned int, 1U, 1U, 0U, 4, assert_equal_int);
    test_arithmetic_subtraction(unsigned int, 1U, 2U, 4294967295U, 4, assert_equal_int);
    test_arithmetic_subtraction(long, 1L, 0L, 1L, 8, assert_equal_long);
    test_arithmetic_subtraction(long, 1L, 1L, 0L, 8, assert_equal_long);
    test_arithmetic_subtraction(long, 1L, 2L, -1L, 8, assert_equal_long);
    test_arithmetic_subtraction(long, -9223372036854775807L, 0L, -9223372036854775807L, 8, assert_equal_long);
    // test_arithmetic_subtraction(long, -9223372036854775807L, 1L, -9223372036854775808L, 8, assert_equal_long); // This is implementation-defined or raises an implementation-defined signal.
    // test_arithmetic_subtraction(long, -9223372036854775807L, 2L, -9223372036854775809L, 8, assert_equal_long); // This is implementation-defined or raises an implementation-defined signal.
    test_arithmetic_subtraction(unsigned long, 1UL, 0UL, 1UL, 8, assert_equal_unsigned_long);
    test_arithmetic_subtraction(unsigned long, 1UL, 1UL, 0UL, 8, assert_equal_unsigned_long);
    test_arithmetic_subtraction(unsigned long, 1UL, 2UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);

#undef test_arithmetic_subtraction
#undef assert_arithmetic_subtraction

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    assert_equal_float(21.0f, 5.0f+20.0f-4.0f);
    assert_equal_double(21.0, 5.0+20.0-4.0);
#endif /* INCLUDE_FLOATING_POINT_TYPE */
}
#endif /* ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS) */
