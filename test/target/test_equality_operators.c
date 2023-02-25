#include "test_code.h"

#if ENABLE_TEST_CASE(TEST_EQUALITY_OPERATORS)
// 6.5.9 Equality operators
void test_equality_operators()
{
    put_title("Equality operators");

#define assert_arithmetic_relation_equal(lhs, rhs, result)    do { \
    assert_size_of_expression(sizeof(int), lhs == rhs); \
    assert_size_of_expression(sizeof(int), rhs == lhs); \
    assert_size_of_expression(sizeof(int), lhs != rhs); \
    assert_size_of_expression(sizeof(int), rhs != lhs); \
    assert_equal_int(result, lhs == rhs); \
    assert_equal_int(result, rhs == lhs); \
    assert_equal_int(1 - result, lhs != rhs); \
    assert_equal_int(1 - result, rhs != lhs); \
    } while(0)
#define test_arithmetic_relation_equal(type, lhs, rhs, result)    do { \
    type lhs_var = (type)lhs; \
    type rhs_var = (type)rhs; \
    assert_arithmetic_relation_equal((type)lhs, (type)rhs, result); \
    assert_arithmetic_relation_equal((type)lhs, rhs_var, result); \
    assert_arithmetic_relation_equal(lhs_var, (type)rhs, result); \
    assert_arithmetic_relation_equal(lhs_var, rhs_var, result); \
    } while(0)

    test_arithmetic_relation_equal(_Bool, 0, 0, 1);
    test_arithmetic_relation_equal(_Bool, 0, 1, 0);
    test_arithmetic_relation_equal(_Bool, 1, 0, 0);
    test_arithmetic_relation_equal(_Bool, 1, 1, 1);
    test_arithmetic_relation_equal(char, -128, -128, 1);
    test_arithmetic_relation_equal(char, -128, -127, 0);
    test_arithmetic_relation_equal(char, -128, -1, 0);
    test_arithmetic_relation_equal(char, -128, 0, 0);
    test_arithmetic_relation_equal(char, -128, 1, 0);
    test_arithmetic_relation_equal(char, -128, 126, 0);
    test_arithmetic_relation_equal(char, -128, 127, 0);
    test_arithmetic_relation_equal(char, -127, -128, 0);
    test_arithmetic_relation_equal(char, -127, -127, 1);
    test_arithmetic_relation_equal(char, -127, -126, 0);
    test_arithmetic_relation_equal(char, -127, -1, 0);
    test_arithmetic_relation_equal(char, -127, 0, 0);
    test_arithmetic_relation_equal(char, -127, 1, 0);
    test_arithmetic_relation_equal(char, -127, 126, 0);
    test_arithmetic_relation_equal(char, -127, 127, 0);
    test_arithmetic_relation_equal(char, -1, -128, 0);
    test_arithmetic_relation_equal(char, -1, -127, 0);
    test_arithmetic_relation_equal(char, -1, -126, 0);
    test_arithmetic_relation_equal(char, -1, -1, 1);
    test_arithmetic_relation_equal(char, -1, 0, 0);
    test_arithmetic_relation_equal(char, -1, 1, 0);
    test_arithmetic_relation_equal(char, -1, 126, 0);
    test_arithmetic_relation_equal(char, -1, 127, 0);
    test_arithmetic_relation_equal(char, 0, -128, 0);
    test_arithmetic_relation_equal(char, 0, -127, 0);
    test_arithmetic_relation_equal(char, 0, -1, 0);
    test_arithmetic_relation_equal(char, 0, 0, 1);
    test_arithmetic_relation_equal(char, 0, 1, 0);
    test_arithmetic_relation_equal(char, 0, 126, 0);
    test_arithmetic_relation_equal(char, 0, 127, 0);
    test_arithmetic_relation_equal(char, 1, -128, 0);
    test_arithmetic_relation_equal(char, 1, -127, 0);
    test_arithmetic_relation_equal(char, 1, -1, 0);
    test_arithmetic_relation_equal(char, 1, 0, 0);
    test_arithmetic_relation_equal(char, 1, 1, 1);
    test_arithmetic_relation_equal(char, 1, 126, 0);
    test_arithmetic_relation_equal(char, 1, 127, 0);
    test_arithmetic_relation_equal(char, 126, -128, 0);
    test_arithmetic_relation_equal(char, 126, -127, 0);
    test_arithmetic_relation_equal(char, 126, -1, 0);
    test_arithmetic_relation_equal(char, 126, 0, 0);
    test_arithmetic_relation_equal(char, 126, 1, 0);
    test_arithmetic_relation_equal(char, 126, 125, 0);
    test_arithmetic_relation_equal(char, 126, 126, 1);
    test_arithmetic_relation_equal(char, 126, 127, 0);
    test_arithmetic_relation_equal(char, 127, -128, 0);
    test_arithmetic_relation_equal(char, 127, -127, 0);
    test_arithmetic_relation_equal(char, 127, -1, 0);
    test_arithmetic_relation_equal(char, 127, 0, 0);
    test_arithmetic_relation_equal(char, 127, 1, 0);
    test_arithmetic_relation_equal(char, 127, 126, 0);
    test_arithmetic_relation_equal(char, 127, 127, 1);
    test_arithmetic_relation_equal(signed char, -128, -128, 1);
    test_arithmetic_relation_equal(signed char, -128, -127, 0);
    test_arithmetic_relation_equal(signed char, -128, -1, 0);
    test_arithmetic_relation_equal(signed char, -128, 0, 0);
    test_arithmetic_relation_equal(signed char, -128, 1, 0);
    test_arithmetic_relation_equal(signed char, -128, 126, 0);
    test_arithmetic_relation_equal(signed char, -128, 127, 0);
    test_arithmetic_relation_equal(signed char, -127, -128, 0);
    test_arithmetic_relation_equal(signed char, -127, -127, 1);
    test_arithmetic_relation_equal(signed char, -127, -126, 0);
    test_arithmetic_relation_equal(signed char, -127, -1, 0);
    test_arithmetic_relation_equal(signed char, -127, 0, 0);
    test_arithmetic_relation_equal(signed char, -127, 1, 0);
    test_arithmetic_relation_equal(signed char, -127, 126, 0);
    test_arithmetic_relation_equal(signed char, -127, 127, 0);
    test_arithmetic_relation_equal(signed char, -1, -128, 0);
    test_arithmetic_relation_equal(signed char, -1, -127, 0);
    test_arithmetic_relation_equal(signed char, -1, -126, 0);
    test_arithmetic_relation_equal(signed char, -1, -1, 1);
    test_arithmetic_relation_equal(signed char, -1, 0, 0);
    test_arithmetic_relation_equal(signed char, -1, 1, 0);
    test_arithmetic_relation_equal(signed char, -1, 126, 0);
    test_arithmetic_relation_equal(signed char, -1, 127, 0);
    test_arithmetic_relation_equal(signed char, 0, -128, 0);
    test_arithmetic_relation_equal(signed char, 0, -127, 0);
    test_arithmetic_relation_equal(signed char, 0, -1, 0);
    test_arithmetic_relation_equal(signed char, 0, 0, 1);
    test_arithmetic_relation_equal(signed char, 0, 1, 0);
    test_arithmetic_relation_equal(signed char, 0, 126, 0);
    test_arithmetic_relation_equal(signed char, 0, 127, 0);
    test_arithmetic_relation_equal(signed char, 1, -128, 0);
    test_arithmetic_relation_equal(signed char, 1, -127, 0);
    test_arithmetic_relation_equal(signed char, 1, -1, 0);
    test_arithmetic_relation_equal(signed char, 1, 0, 0);
    test_arithmetic_relation_equal(signed char, 1, 1, 1);
    test_arithmetic_relation_equal(signed char, 1, 126, 0);
    test_arithmetic_relation_equal(signed char, 1, 127, 0);
    test_arithmetic_relation_equal(signed char, 126, -128, 0);
    test_arithmetic_relation_equal(signed char, 126, -127, 0);
    test_arithmetic_relation_equal(signed char, 126, -1, 0);
    test_arithmetic_relation_equal(signed char, 126, 0, 0);
    test_arithmetic_relation_equal(signed char, 126, 1, 0);
    test_arithmetic_relation_equal(signed char, 126, 125, 0);
    test_arithmetic_relation_equal(signed char, 126, 126, 1);
    test_arithmetic_relation_equal(signed char, 126, 127, 0);
    test_arithmetic_relation_equal(signed char, 127, -128, 0);
    test_arithmetic_relation_equal(signed char, 127, -127, 0);
    test_arithmetic_relation_equal(signed char, 127, -1, 0);
    test_arithmetic_relation_equal(signed char, 127, 0, 0);
    test_arithmetic_relation_equal(signed char, 127, 1, 0);
    test_arithmetic_relation_equal(signed char, 127, 126, 0);
    test_arithmetic_relation_equal(signed char, 127, 127, 1);
    test_arithmetic_relation_equal(unsigned char, 0, 0, 1);
    test_arithmetic_relation_equal(unsigned char, 0, 1, 0);
    test_arithmetic_relation_equal(unsigned char, 0, 254, 0);
    test_arithmetic_relation_equal(unsigned char, 0, 255, 0);
    test_arithmetic_relation_equal(unsigned char, 1, 0, 0);
    test_arithmetic_relation_equal(unsigned char, 1, 1, 1);
    test_arithmetic_relation_equal(unsigned char, 1, 2, 0);
    test_arithmetic_relation_equal(unsigned char, 1, 254, 0);
    test_arithmetic_relation_equal(unsigned char, 1, 255, 0);
    test_arithmetic_relation_equal(unsigned char, 254, 0, 0);
    test_arithmetic_relation_equal(unsigned char, 254, 1, 0);
    test_arithmetic_relation_equal(unsigned char, 254, 253, 0);
    test_arithmetic_relation_equal(unsigned char, 254, 254, 1);
    test_arithmetic_relation_equal(unsigned char, 254, 255, 0);
    test_arithmetic_relation_equal(unsigned char, 255, 0, 0);
    test_arithmetic_relation_equal(unsigned char, 255, 1, 0);
    test_arithmetic_relation_equal(unsigned char, 255, 254, 0);
    test_arithmetic_relation_equal(unsigned char, 255, 255, 1);
    test_arithmetic_relation_equal(short, -32768, -32768, 1);
    test_arithmetic_relation_equal(short, -32768, -32767, 0);
    test_arithmetic_relation_equal(short, -32768, -1, 0);
    test_arithmetic_relation_equal(short, -32768, 0, 0);
    test_arithmetic_relation_equal(short, -32768, 1, 0);
    test_arithmetic_relation_equal(short, -32768, 32766, 0);
    test_arithmetic_relation_equal(short, -32768, 32767, 0);
    test_arithmetic_relation_equal(short, -32767, -32768, 0);
    test_arithmetic_relation_equal(short, -32767, -32767, 1);
    test_arithmetic_relation_equal(short, -32767, -32766, 0);
    test_arithmetic_relation_equal(short, -32767, -1, 0);
    test_arithmetic_relation_equal(short, -32767, 0, 0);
    test_arithmetic_relation_equal(short, -32767, 1, 0);
    test_arithmetic_relation_equal(short, -32767, 32766, 0);
    test_arithmetic_relation_equal(short, -32767, 32767, 0);
    test_arithmetic_relation_equal(short, -1, -32768, 0);
    test_arithmetic_relation_equal(short, -1, -32767, 0);
    test_arithmetic_relation_equal(short, -1, -32766, 0);
    test_arithmetic_relation_equal(short, -1, -1, 1);
    test_arithmetic_relation_equal(short, -1, 0, 0);
    test_arithmetic_relation_equal(short, -1, 1, 0);
    test_arithmetic_relation_equal(short, -1, 32766, 0);
    test_arithmetic_relation_equal(short, -1, 32767, 0);
    test_arithmetic_relation_equal(short, 0, -32768, 0);
    test_arithmetic_relation_equal(short, 0, -32767, 0);
    test_arithmetic_relation_equal(short, 0, -1, 0);
    test_arithmetic_relation_equal(short, 0, 0, 1);
    test_arithmetic_relation_equal(short, 0, 1, 0);
    test_arithmetic_relation_equal(short, 0, 32766, 0);
    test_arithmetic_relation_equal(short, 0, 32767, 0);
    test_arithmetic_relation_equal(short, 1, -32768, 0);
    test_arithmetic_relation_equal(short, 1, -32767, 0);
    test_arithmetic_relation_equal(short, 1, -1, 0);
    test_arithmetic_relation_equal(short, 1, 0, 0);
    test_arithmetic_relation_equal(short, 1, 1, 1);
    test_arithmetic_relation_equal(short, 1, 32766, 0);
    test_arithmetic_relation_equal(short, 1, 32767, 0);
    test_arithmetic_relation_equal(short, 32766, -32768, 0);
    test_arithmetic_relation_equal(short, 32766, -32767, 0);
    test_arithmetic_relation_equal(short, 32766, -1, 0);
    test_arithmetic_relation_equal(short, 32766, 0, 0);
    test_arithmetic_relation_equal(short, 32766, 1, 0);
    test_arithmetic_relation_equal(short, 32766, 32765, 0);
    test_arithmetic_relation_equal(short, 32766, 32766, 1);
    test_arithmetic_relation_equal(short, 32766, 32767, 0);
    test_arithmetic_relation_equal(short, 32767, -32768, 0);
    test_arithmetic_relation_equal(short, 32767, -32767, 0);
    test_arithmetic_relation_equal(short, 32767, -1, 0);
    test_arithmetic_relation_equal(short, 32767, 0, 0);
    test_arithmetic_relation_equal(short, 32767, 1, 0);
    test_arithmetic_relation_equal(short, 32767, 32766, 0);
    test_arithmetic_relation_equal(short, 32767, 32767, 1);
    test_arithmetic_relation_equal(unsigned short, 0, 0, 1);
    test_arithmetic_relation_equal(unsigned short, 0, 1, 0);
    test_arithmetic_relation_equal(unsigned short, 0, 65534, 0);
    test_arithmetic_relation_equal(unsigned short, 0, 65535, 0);
    test_arithmetic_relation_equal(unsigned short, 1, 0, 0);
    test_arithmetic_relation_equal(unsigned short, 1, 1, 1);
    test_arithmetic_relation_equal(unsigned short, 1, 2, 0);
    test_arithmetic_relation_equal(unsigned short, 1, 65534, 0);
    test_arithmetic_relation_equal(unsigned short, 1, 65535, 0);
    test_arithmetic_relation_equal(unsigned short, 65534, 0, 0);
    test_arithmetic_relation_equal(unsigned short, 65534, 1, 0);
    test_arithmetic_relation_equal(unsigned short, 65534, 65533, 0);
    test_arithmetic_relation_equal(unsigned short, 65534, 65534, 1);
    test_arithmetic_relation_equal(unsigned short, 65534, 65535, 0);
    test_arithmetic_relation_equal(unsigned short, 65535, 0, 0);
    test_arithmetic_relation_equal(unsigned short, 65535, 1, 0);
    test_arithmetic_relation_equal(unsigned short, 65535, 65534, 0);
    test_arithmetic_relation_equal(unsigned short, 65535, 65535, 1);
    test_arithmetic_relation_equal(int, -2147483648, -2147483648, 1);
    test_arithmetic_relation_equal(int, -2147483648, -2147483647, 0);
    test_arithmetic_relation_equal(int, -2147483648, -1, 0);
    test_arithmetic_relation_equal(int, -2147483648, 0, 0);
    test_arithmetic_relation_equal(int, -2147483648, 1, 0);
    test_arithmetic_relation_equal(int, -2147483648, 2147483646, 0);
    test_arithmetic_relation_equal(int, -2147483648, 2147483647, 0);
    test_arithmetic_relation_equal(int, -2147483647, -2147483648, 0);
    test_arithmetic_relation_equal(int, -2147483647, -2147483647, 1);
    test_arithmetic_relation_equal(int, -2147483647, -2147483646, 0);
    test_arithmetic_relation_equal(int, -2147483647, -1, 0);
    test_arithmetic_relation_equal(int, -2147483647, 0, 0);
    test_arithmetic_relation_equal(int, -2147483647, 1, 0);
    test_arithmetic_relation_equal(int, -2147483647, 2147483646, 0);
    test_arithmetic_relation_equal(int, -2147483647, 2147483647, 0);
    test_arithmetic_relation_equal(int, -1, -2147483648, 0);
    test_arithmetic_relation_equal(int, -1, -2147483647, 0);
    test_arithmetic_relation_equal(int, -1, -2147483646, 0);
    test_arithmetic_relation_equal(int, -1, -1, 1);
    test_arithmetic_relation_equal(int, -1, 0, 0);
    test_arithmetic_relation_equal(int, -1, 1, 0);
    test_arithmetic_relation_equal(int, -1, 2147483646, 0);
    test_arithmetic_relation_equal(int, -1, 2147483647, 0);
    test_arithmetic_relation_equal(int, 0, -2147483648, 0);
    test_arithmetic_relation_equal(int, 0, -2147483647, 0);
    test_arithmetic_relation_equal(int, 0, -1, 0);
    test_arithmetic_relation_equal(int, 0, 0, 1);
    test_arithmetic_relation_equal(int, 0, 1, 0);
    test_arithmetic_relation_equal(int, 0, 2147483646, 0);
    test_arithmetic_relation_equal(int, 0, 2147483647, 0);
    test_arithmetic_relation_equal(int, 1, -2147483648, 0);
    test_arithmetic_relation_equal(int, 1, -2147483647, 0);
    test_arithmetic_relation_equal(int, 1, -1, 0);
    test_arithmetic_relation_equal(int, 1, 0, 0);
    test_arithmetic_relation_equal(int, 1, 1, 1);
    test_arithmetic_relation_equal(int, 1, 2147483646, 0);
    test_arithmetic_relation_equal(int, 1, 2147483647, 0);
    test_arithmetic_relation_equal(int, 2147483646, -2147483648, 0);
    test_arithmetic_relation_equal(int, 2147483646, -2147483647, 0);
    test_arithmetic_relation_equal(int, 2147483646, -1, 0);
    test_arithmetic_relation_equal(int, 2147483646, 0, 0);
    test_arithmetic_relation_equal(int, 2147483646, 1, 0);
    test_arithmetic_relation_equal(int, 2147483646, 2147483645, 0);
    test_arithmetic_relation_equal(int, 2147483646, 2147483646, 1);
    test_arithmetic_relation_equal(int, 2147483646, 2147483647, 0);
    test_arithmetic_relation_equal(int, 2147483647, -2147483648, 0);
    test_arithmetic_relation_equal(int, 2147483647, -2147483647, 0);
    test_arithmetic_relation_equal(int, 2147483647, -1, 0);
    test_arithmetic_relation_equal(int, 2147483647, 0, 0);
    test_arithmetic_relation_equal(int, 2147483647, 1, 0);
    test_arithmetic_relation_equal(int, 2147483647, 2147483646, 0);
    test_arithmetic_relation_equal(int, 2147483647, 2147483647, 1);
    test_arithmetic_relation_equal(unsigned int, 0, 0, 1);
    test_arithmetic_relation_equal(unsigned int, 0, 1, 0);
    test_arithmetic_relation_equal(unsigned int, 0, 4294967294, 0);
    test_arithmetic_relation_equal(unsigned int, 0, 4294967295, 0);
    test_arithmetic_relation_equal(unsigned int, 1, 0, 0);
    test_arithmetic_relation_equal(unsigned int, 1, 1, 1);
    test_arithmetic_relation_equal(unsigned int, 1, 2, 0);
    test_arithmetic_relation_equal(unsigned int, 1, 4294967294, 0);
    test_arithmetic_relation_equal(unsigned int, 1, 4294967295, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967294, 0, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967294, 1, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967294, 4294967293, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967294, 4294967294, 1);
    test_arithmetic_relation_equal(unsigned int, 4294967294, 4294967295, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967295, 0, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967295, 1, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967295, 4294967294, 0);
    test_arithmetic_relation_equal(unsigned int, 4294967295, 4294967295, 1);
    test_arithmetic_relation_equal(long, -9223372036854775807L, -9223372036854775807L, 1);
    test_arithmetic_relation_equal(long, -9223372036854775807L, -9223372036854775806L, 0);
    test_arithmetic_relation_equal(long, -9223372036854775807L, -1L, 0);
    test_arithmetic_relation_equal(long, -9223372036854775807L, 0L, 0);
    test_arithmetic_relation_equal(long, -9223372036854775807L, 1L, 0);
    test_arithmetic_relation_equal(long, -9223372036854775807L, 9223372036854775806L, 0);
    test_arithmetic_relation_equal(long, -9223372036854775807L, 9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, -1L, -9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, -1L, -9223372036854775806L, 0);
    test_arithmetic_relation_equal(long, -1L, -1L, 1);
    test_arithmetic_relation_equal(long, -1L, 0L, 0);
    test_arithmetic_relation_equal(long, -1L, 1L, 0);
    test_arithmetic_relation_equal(long, -1L, 9223372036854775806L, 0);
    test_arithmetic_relation_equal(long, -1L, 9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 0L, -9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 0L, -1L, 0);
    test_arithmetic_relation_equal(long, 0L, 0L, 1);
    test_arithmetic_relation_equal(long, 0L, 1L, 0);
    test_arithmetic_relation_equal(long, 0L, 9223372036854775806L, 0);
    test_arithmetic_relation_equal(long, 0L, 9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 1L, -9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 1L, -1L, 0);
    test_arithmetic_relation_equal(long, 1L, 0L, 0);
    test_arithmetic_relation_equal(long, 1L, 1L, 1);
    test_arithmetic_relation_equal(long, 1L, 9223372036854775806L, 0);
    test_arithmetic_relation_equal(long, 1L, 9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775806L, -9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775806L, -1L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775806L, 0L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775806L, 1L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775806L, 9223372036854775805L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775806L, 9223372036854775806L, 1);
    test_arithmetic_relation_equal(long, 9223372036854775806L, 9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775807L, -9223372036854775807L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775807L, -1L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775807L, 0L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775807L, 1L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775807L, 9223372036854775806L, 0);
    test_arithmetic_relation_equal(long, 9223372036854775807L, 9223372036854775807L, 1);
    test_arithmetic_relation_equal(unsigned long, 0UL, 0UL, 1);
    test_arithmetic_relation_equal(unsigned long, 0UL, 1UL, 0);
    test_arithmetic_relation_equal(unsigned long, 0UL, 18446744073709551614UL, 0);
    test_arithmetic_relation_equal(unsigned long, 0UL, 18446744073709551615UL, 0);
    test_arithmetic_relation_equal(unsigned long, 1UL, 0UL, 0);
    test_arithmetic_relation_equal(unsigned long, 1UL, 1UL, 1);
    test_arithmetic_relation_equal(unsigned long, 1UL, 2UL, 0);
    test_arithmetic_relation_equal(unsigned long, 1UL, 18446744073709551614UL, 0);
    test_arithmetic_relation_equal(unsigned long, 1UL, 18446744073709551615UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551614UL, 0UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551614UL, 1UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551614UL, 18446744073709551613UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551614UL, 18446744073709551614UL, 1);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551614UL, 18446744073709551615UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551615UL, 0UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551615UL, 1UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551615UL, 18446744073709551614UL, 0);
    test_arithmetic_relation_equal(unsigned long, 18446744073709551615UL, 18446744073709551615UL, 1);

#undef test_arithmetic_relation_equal
#undef assert_arithmetic_relation_equal

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    float f = 1.0f;
    assert_equal_int(1, 1.0f == f); assert_equal_int(0, 0.0f == f);
    assert_equal_int(0, 1.0f != f); assert_equal_int(1, 0.0f != f);
    assert_equal_int(0, 1.0f < f); assert_equal_int(1, 0.0f < f);
    assert_equal_int(1, 1.0f <= f); assert_equal_int(0, 2.0f <= f);
    assert_equal_int(0, 1.0f > f); assert_equal_int(1, 2.0f > f);
    assert_equal_int(1, 1.0f >= f); assert_equal_int(0, 0.0f >= f);

    double d = 2.0;
    assert_equal_int(1, 2.0 == d); assert_equal_int(0, 0.0 == d);
    assert_equal_int(0, 2.0 != d); assert_equal_int(1, 0.0 != d);
    assert_equal_int(0, 2.0 < d); assert_equal_int(1, 0.0 < d);
    assert_equal_int(1, 2.0 <= d); assert_equal_int(0, 3.0 <= d);
    assert_equal_int(0, 2.0 > d); assert_equal_int(1, 3.0 > d);
    assert_equal_int(1, 2.0 >= d); assert_equal_int(0, 0.0 >= d);
#endif /* INCLUDE_FLOATING_POINT_TYPE */
}
#endif /* ENABLE_TEST_CASE(TEST_EQUALITY_OPERATORS) */