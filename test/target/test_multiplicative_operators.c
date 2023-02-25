#include "test_code.h"

#if ENABLE_TEST_CASE(TEST_MULTIPLICATIVE_OPERATORS)
// 6.5.5 Multiplicative operators
void test_multiplicative_operators()
{
    put_title("Multiplicative operators");

#define assert_arithmetic_multiplication(lhs, rhs, result, size, assert_function)    do { \
    assert_size_of_expression(size, lhs * rhs); \
    assert_size_of_expression(size, rhs * lhs); \
    assert_function(result, lhs * rhs); \
    assert_function(result, rhs * lhs); \
    } while(0)
#define test_arithmetic_multiplication(type, lhs, rhs, result, size, assert_function)    do { \
    type lhs_var = (type)lhs; \
    type rhs_var = (type)rhs; \
    assert_arithmetic_multiplication((type)lhs, (type)rhs, result, size, assert_function); \
    assert_arithmetic_multiplication((type)lhs, rhs_var, result, size, assert_function); \
    assert_arithmetic_multiplication(lhs_var, (type)rhs, result, size, assert_function); \
    assert_arithmetic_multiplication(lhs_var, rhs_var, result, size, assert_function); \
    } while(0)

    test_arithmetic_multiplication(_Bool, 0, 0, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(_Bool, 0, 1, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(_Bool, 1, 1, 1, 4, assert_equal_int);
    test_arithmetic_multiplication(char, 127, 0, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(char, 127, 1, 127, 4, assert_equal_int);
    test_arithmetic_multiplication(char, 127, 2, 254, 4, assert_equal_int);
    test_arithmetic_multiplication(char, 64, 2, 128, 4, assert_equal_int);
    test_arithmetic_multiplication(char, 64, 3, 192, 4, assert_equal_int);
    test_arithmetic_multiplication(signed char, 127, 0, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(signed char, 127, 1, 127, 4, assert_equal_int);
    test_arithmetic_multiplication(signed char, 127, 2, 254, 4, assert_equal_int);
    test_arithmetic_multiplication(signed char, 64, 2, 128, 4, assert_equal_int);
    test_arithmetic_multiplication(signed char, 64, 3, 192, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned char, 255, 0, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned char, 255, 1, 255, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned char, 255, 2, 510, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned char, 128, 2, 256, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned char, 128, 3, 384, 4, assert_equal_int);
    test_arithmetic_multiplication(short, 32767, 0, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(short, 32767, 1, 32767, 4, assert_equal_int);
    test_arithmetic_multiplication(short, 32767, 2, 65534, 4, assert_equal_int);
    test_arithmetic_multiplication(short, 16384, 2, 32768, 4, assert_equal_int);
    test_arithmetic_multiplication(short, 16384, 3, 49152, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned short, 65535, 0, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned short, 65535, 1, 65535, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned short, 65535, 2, 131070, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned short, 32768, 2, 65536, 4, assert_equal_int);
    test_arithmetic_multiplication(unsigned short, 32768, 3, 98304, 4, assert_equal_int);
    test_arithmetic_multiplication(int, 2147483647, -1, -2147483647, 4, assert_equal_int);
    test_arithmetic_multiplication(int, 2147483647, 0, 0, 4, assert_equal_int);
    test_arithmetic_multiplication(int, 2147483647, 1, 2147483647, 4, assert_equal_int);
    // test_arithmetic_multiplication(int, 2147483647, 2, 4294967294, 4, assert_equal_int); // This is implementation-defined or raises an implementation-defined signal.
    // test_arithmetic_multiplication(int, 1073741824, 2, 2147483648, 4, assert_equal_int); // This is implementation-defined or raises an implementation-defined signal.
    test_arithmetic_multiplication(unsigned int, 4294967295U, 0U, 0U, 4, assert_equal_unsigned_int);
    test_arithmetic_multiplication(unsigned int, 4294967295U, 1U, 4294967295U, 4, assert_equal_unsigned_int);
    test_arithmetic_multiplication(unsigned int, 4294967295U, 2U, 4294967294U, 4, assert_equal_unsigned_int);
    test_arithmetic_multiplication(unsigned int, 2147483648U, 2U, 0U, 4, assert_equal_unsigned_int);
    test_arithmetic_multiplication(unsigned int, 2147483648U, 3U, 2147483648U, 4, assert_equal_unsigned_int);
    test_arithmetic_multiplication(long, 9223372036854775807L, -1L, -9223372036854775807L, 8, assert_equal_long);
    test_arithmetic_multiplication(long, 9223372036854775807L, 0L, 0L, 8, assert_equal_long);
    test_arithmetic_multiplication(long, 9223372036854775807L, 1L, 9223372036854775807L, 8, assert_equal_long);
    // test_arithmetic_multiplication(long, 9223372036854775807L, 2L, 18446744073709551614L, 8, assert_equal_long); // This is implementation-defined or raises an implementation-defined signal.
    // test_arithmetic_multiplication(long, 4611686018427387904L, 2L, 9223372036854775808L, 8, assert_equal_long); // This is implementation-defined or raises an implementation-defined signal.
    test_arithmetic_multiplication(unsigned long, 18446744073709551615UL, 0UL, 0UL, 8, assert_equal_unsigned_long);
    test_arithmetic_multiplication(unsigned long, 18446744073709551615UL, 1UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);
    test_arithmetic_multiplication(unsigned long, 18446744073709551615UL, 2UL, 18446744073709551614UL, 8, assert_equal_unsigned_long);
    test_arithmetic_multiplication(unsigned long, 9223372036854775808UL, 2UL, 0UL, 8, assert_equal_unsigned_long);
    test_arithmetic_multiplication(unsigned long, 9223372036854775808UL, 3UL, 9223372036854775808UL, 8, assert_equal_unsigned_long);

#undef test_arithmetic_multiplication
#undef assert_arithmetic_multiplication

#define assert_arithmetic_division(lhs, rhs, result, size, assert_function)    do { \
    assert_size_of_expression(size, lhs / rhs); \
    assert_function(result, lhs / rhs); \
    } while(0)
#define test_arithmetic_division(type, lhs, rhs, result, size, assert_function)    do { \
    type lhs_var = (type)lhs; \
    type rhs_var = (type)rhs; \
    assert_arithmetic_division((type)lhs, (type)rhs, result, size, assert_function); \
    assert_arithmetic_division((type)lhs, rhs_var, result, size, assert_function); \
    assert_arithmetic_division(lhs_var, (type)rhs, result, size, assert_function); \
    assert_arithmetic_division(lhs_var, rhs_var, result, size, assert_function); \
    } while(0)

    test_arithmetic_division(_Bool, 0, 1, 0, 4, assert_equal_int);
    test_arithmetic_division(_Bool, 1, 1, 1, 4, assert_equal_int);
    test_arithmetic_division(char, 127, 1, 127, 4, assert_equal_int);
    test_arithmetic_division(char, 127, 2, 63, 4, assert_equal_int);
    test_arithmetic_division(char, 127, 127, 1, 4, assert_equal_int);
    test_arithmetic_division(char, 126, 127, 0, 4, assert_equal_int);
    test_arithmetic_division(signed char, 127, 1, 127, 4, assert_equal_int);
    test_arithmetic_division(signed char, 127, 2, 63, 4, assert_equal_int);
    test_arithmetic_division(signed char, 127, 127, 1, 4, assert_equal_int);
    test_arithmetic_division(signed char, 126, 127, 0, 4, assert_equal_int);
    test_arithmetic_division(unsigned char, 255, 1, 255, 4, assert_equal_int);
    test_arithmetic_division(unsigned char, 255, 2, 127, 4, assert_equal_int);
    test_arithmetic_division(unsigned char, 255, 255, 1, 4, assert_equal_int);
    test_arithmetic_division(unsigned char, 254, 255, 0, 4, assert_equal_int);
    test_arithmetic_division(short, 32767, 1, 32767, 4, assert_equal_int);
    test_arithmetic_division(short, 32767, 2, 16383, 4, assert_equal_int);
    test_arithmetic_division(short, 32767, 32767, 1, 4, assert_equal_int);
    test_arithmetic_division(short, 32766, 32767, 0, 4, assert_equal_int);
    test_arithmetic_division(unsigned short, 65535, 1, 65535, 4, assert_equal_int);
    test_arithmetic_division(unsigned short, 65535, 2, 32767, 4, assert_equal_int);
    test_arithmetic_division(unsigned short, 65535, 65535, 1, 4, assert_equal_int);
    test_arithmetic_division(unsigned short, 65534, 65535, 0, 4, assert_equal_int);
    test_arithmetic_division(int, 2147483647, -2, -1073741823, 4, assert_equal_int);
    test_arithmetic_division(int, 2147483647, -1, -2147483647, 4, assert_equal_int);
    test_arithmetic_division(int, 2147483647, 1, 2147483647, 4, assert_equal_int);
    test_arithmetic_division(int, 2147483647, 2, 1073741823, 4, assert_equal_int);
    test_arithmetic_division(int, 2147483647, 2147483647, 1, 4, assert_equal_int);
    test_arithmetic_division(int, 2147483646, 2147483647, 0, 4, assert_equal_int);
    test_arithmetic_division(unsigned int, 4294967295U, 1U, 4294967295U, 4, assert_equal_unsigned_int);
    test_arithmetic_division(unsigned int, 4294967295U, 2U, 2147483647U, 4, assert_equal_unsigned_int);
    test_arithmetic_division(unsigned int, 4294967295U, 4294967295U, 1U, 4, assert_equal_unsigned_int);
    test_arithmetic_division(unsigned int, 4294967294U, 4294967295U, 0U, 4, assert_equal_unsigned_int);
    test_arithmetic_division(long, 9223372036854775807L, -2L, -4611686018427387903L, 8, assert_equal_long);
    test_arithmetic_division(long, 9223372036854775807L, -1L, -9223372036854775807L, 8, assert_equal_long);
    test_arithmetic_division(long, 9223372036854775807L, 1L, 9223372036854775807L, 8, assert_equal_long);
    test_arithmetic_division(long, 9223372036854775807L, 2L, 4611686018427387903L, 8, assert_equal_long);
    test_arithmetic_division(long, 9223372036854775807L, 9223372036854775807L, 1L, 8, assert_equal_long);
    test_arithmetic_division(long, 9223372036854775806L, 9223372036854775807L, 0L, 8, assert_equal_long);
    test_arithmetic_division(unsigned long, 18446744073709551615UL, 1UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);
    test_arithmetic_division(unsigned long, 18446744073709551615UL, 2UL, 9223372036854775807UL, 8, assert_equal_unsigned_long);
    test_arithmetic_division(unsigned long, 18446744073709551615UL, 18446744073709551615UL, 1UL, 8, assert_equal_unsigned_long);
    test_arithmetic_division(unsigned long, 18446744073709551614UL, 18446744073709551615UL, 0UL, 8, assert_equal_unsigned_long);

#undef test_arithmetic_division
#undef assert_arithmetic_division

#define assert_arithmetic_modulation(lhs, rhs, result, size, assert_function)    do { \
    assert_size_of_expression(size, lhs % rhs); \
    assert_function(result, lhs % rhs); \
    } while(0)
#define test_arithmetic_modulation(type, lhs, rhs, result, size, assert_function)    do { \
    type lhs_var = (type)lhs; \
    type rhs_var = (type)rhs; \
    assert_arithmetic_modulation((type)lhs, (type)rhs, result, size, assert_function); \
    assert_arithmetic_modulation((type)lhs, rhs_var, result, size, assert_function); \
    assert_arithmetic_modulation(lhs_var, (type)rhs, result, size, assert_function); \
    assert_arithmetic_modulation(lhs_var, rhs_var, result, size, assert_function); \
    } while(0)

    test_arithmetic_modulation(_Bool, 0, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(_Bool, 1, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(char, 127, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(char, 127, 2, 1, 4, assert_equal_int);
    test_arithmetic_modulation(char, 127, 127, 0, 4, assert_equal_int);
    test_arithmetic_modulation(char, 126, 127, 126, 4, assert_equal_int);
    test_arithmetic_modulation(signed char, 127, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(signed char, 127, 2, 1, 4, assert_equal_int);
    test_arithmetic_modulation(signed char, 127, 127, 0, 4, assert_equal_int);
    test_arithmetic_modulation(signed char, 126, 127, 126, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned char, 255, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned char, 255, 2, 1, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned char, 255, 255, 0, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned char, 254, 255, 254, 4, assert_equal_int);
    test_arithmetic_modulation(short, 32767, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(short, 32767, 2, 1, 4, assert_equal_int);
    test_arithmetic_modulation(short, 32767, 32767, 0, 4, assert_equal_int);
    test_arithmetic_modulation(short, 32766, 32767, 32766, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned short, 65535, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned short, 65535, 2, 1, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned short, 65535, 65535, 0, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned short, 65534, 65535, 65534, 4, assert_equal_int);
    test_arithmetic_modulation(int, 2147483646, -2147483647, 2147483646, 4, assert_equal_int);
    test_arithmetic_modulation(int, 2147483647, -1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(int, 2147483647, 1, 0, 4, assert_equal_int);
    test_arithmetic_modulation(int, 2147483647, 2, 1, 4, assert_equal_int);
    test_arithmetic_modulation(int, 2147483647, 2147483647, 0, 4, assert_equal_int);
    test_arithmetic_modulation(int, 2147483646, 2147483647, 2147483646, 4, assert_equal_int);
    test_arithmetic_modulation(unsigned int, 4294967295U, 1U, 0U, 4, assert_equal_unsigned_int);
    test_arithmetic_modulation(unsigned int, 4294967295U, 2U, 1U, 4, assert_equal_unsigned_int);
    test_arithmetic_modulation(unsigned int, 4294967295U, 4294967295U, 0U, 4, assert_equal_unsigned_int);
    test_arithmetic_modulation(unsigned int, 4294967294U, 4294967295U, 4294967294U, 4, assert_equal_unsigned_int);
    test_arithmetic_modulation(long, 9223372036854775806L, -9223372036854775807L, 9223372036854775806L, 8, assert_equal_long);
    test_arithmetic_modulation(long, 9223372036854775807L, -1L, 0L, 8, assert_equal_long);
    test_arithmetic_modulation(long, 9223372036854775807L, 1L, 0L, 8, assert_equal_long);
    test_arithmetic_modulation(long, 9223372036854775807L, 2L, 1L, 8, assert_equal_long);
    test_arithmetic_modulation(long, 9223372036854775807L, 9223372036854775807L, 0L, 8, assert_equal_long);
    test_arithmetic_modulation(long, 9223372036854775806L, 9223372036854775807L, 9223372036854775806L, 8, assert_equal_long);
    test_arithmetic_modulation(unsigned long, 18446744073709551615UL, 1UL, 0UL, 8, assert_equal_unsigned_long);
    test_arithmetic_modulation(unsigned long, 18446744073709551615UL, 2UL, 1UL, 8, assert_equal_unsigned_long);
    test_arithmetic_modulation(unsigned long, 18446744073709551615UL, 18446744073709551615UL, 0UL, 8, assert_equal_unsigned_long);
    test_arithmetic_modulation(unsigned long, 18446744073709551614UL, 18446744073709551615UL, 18446744073709551614UL, 8, assert_equal_unsigned_long);

#undef test_arithmetic_modulation
#undef assert_arithmetic_modulation

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    assert_equal_float(3.0f, 0.5f * (9.0f - 6.0f / 2.0f));
    assert_equal_double(3.0, 0.5 * (9.0 - 6.0 / 2.0));
#endif /* INCLUDE_FLOATING_POINT_TYPE */
}
#endif /* ENABLE_TEST_CASE(TEST_MULTIPLICATIVE_OPERATORS) */
