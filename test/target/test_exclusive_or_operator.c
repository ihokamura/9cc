#include "test_code.h"

#if ENABLE_TEST_CASE(TEST_EXCLUSIVE_OR_OPERATOR)
// 6.5.11 Bitwise exclusive OR operator
void test_exclusive_or_operator()
{
    put_title("Bitwise exclusive OR operator");

#define assert_exclusive_or(lhs, rhs, result, size, assert_function)    do { \
    assert_size_of_expression(size, lhs ^ rhs); \
    assert_size_of_expression(size, rhs ^ lhs); \
    assert_function(result, lhs ^ rhs); \
    assert_function(result, rhs ^ lhs); \
    } while(0)
#define test_exclusive_or(type, lhs, rhs, result, size, assert_function)    do { \
    type lhs_var = (type)lhs; \
    type rhs_var = (type)rhs; \
    assert_exclusive_or((type)lhs, (type)rhs, result, size, assert_function); \
    assert_exclusive_or((type)lhs, rhs_var, result, size, assert_function); \
    assert_exclusive_or(lhs_var, (type)rhs, result, size, assert_function); \
    assert_exclusive_or(lhs_var, rhs_var, result, size, assert_function); \
    } while(0)

    test_exclusive_or(_Bool, 0, 0, 0, 4, assert_equal_int);
    test_exclusive_or(_Bool, 0, 1, 1, 4, assert_equal_int);
    test_exclusive_or(_Bool, 1, 0, 1, 4, assert_equal_int);
    test_exclusive_or(_Bool, 1, 1, 0, 4, assert_equal_int);
    test_exclusive_or(char, -128, -128, 0, 4, assert_equal_int);
    test_exclusive_or(char, -128, -127, 1, 4, assert_equal_int);
    test_exclusive_or(char, -128, -1, 127, 4, assert_equal_int);
    test_exclusive_or(char, -128, 0, -128, 4, assert_equal_int);
    test_exclusive_or(char, -128, 1, -127, 4, assert_equal_int);
    test_exclusive_or(char, -128, 126, -2, 4, assert_equal_int);
    test_exclusive_or(char, -128, 127, -1, 4, assert_equal_int);
    test_exclusive_or(char, -127, -128, 1, 4, assert_equal_int);
    test_exclusive_or(char, -127, -127, 0, 4, assert_equal_int);
    test_exclusive_or(char, -127, -126, 3, 4, assert_equal_int);
    test_exclusive_or(char, -127, -1, 126, 4, assert_equal_int);
    test_exclusive_or(char, -127, 0, -127, 4, assert_equal_int);
    test_exclusive_or(char, -127, 1, -128, 4, assert_equal_int);
    test_exclusive_or(char, -127, 126, -1, 4, assert_equal_int);
    test_exclusive_or(char, -127, 127, -2, 4, assert_equal_int);
    test_exclusive_or(char, -1, -128, 127, 4, assert_equal_int);
    test_exclusive_or(char, -1, -127, 126, 4, assert_equal_int);
    test_exclusive_or(char, -1, -126, 125, 4, assert_equal_int);
    test_exclusive_or(char, -1, -1, 0, 4, assert_equal_int);
    test_exclusive_or(char, -1, 0, -1, 4, assert_equal_int);
    test_exclusive_or(char, -1, 1, -2, 4, assert_equal_int);
    test_exclusive_or(char, -1, 126, -127, 4, assert_equal_int);
    test_exclusive_or(char, -1, 127, -128, 4, assert_equal_int);
    test_exclusive_or(char, 0, -128, -128, 4, assert_equal_int);
    test_exclusive_or(char, 0, -127, -127, 4, assert_equal_int);
    test_exclusive_or(char, 0, -126, -126, 4, assert_equal_int);
    test_exclusive_or(char, 0, -1, -1, 4, assert_equal_int);
    test_exclusive_or(char, 0, 0, 0, 4, assert_equal_int);
    test_exclusive_or(char, 0, 1, 1, 4, assert_equal_int);
    test_exclusive_or(char, 0, 126, 126, 4, assert_equal_int);
    test_exclusive_or(char, 0, 127, 127, 4, assert_equal_int);
    test_exclusive_or(char, 1, -128, -127, 4, assert_equal_int);
    test_exclusive_or(char, 1, -127, -128, 4, assert_equal_int);
    test_exclusive_or(char, 1, -126, -125, 4, assert_equal_int);
    test_exclusive_or(char, 1, -1, -2, 4, assert_equal_int);
    test_exclusive_or(char, 1, 0, 1, 4, assert_equal_int);
    test_exclusive_or(char, 1, 1, 0, 4, assert_equal_int);
    test_exclusive_or(char, 1, 126, 127, 4, assert_equal_int);
    test_exclusive_or(char, 1, 127, 126, 4, assert_equal_int);
    test_exclusive_or(char, 126, -128, -2, 4, assert_equal_int);
    test_exclusive_or(char, 126, -127, -1, 4, assert_equal_int);
    test_exclusive_or(char, 126, -126, -4, 4, assert_equal_int);
    test_exclusive_or(char, 126, -1, -127, 4, assert_equal_int);
    test_exclusive_or(char, 126, 0, 126, 4, assert_equal_int);
    test_exclusive_or(char, 126, 1, 127, 4, assert_equal_int);
    test_exclusive_or(char, 126, 126, 0, 4, assert_equal_int);
    test_exclusive_or(char, 126, 127, 1, 4, assert_equal_int);
    test_exclusive_or(char, 127, -128, -1, 4, assert_equal_int);
    test_exclusive_or(char, 127, -127, -2, 4, assert_equal_int);
    test_exclusive_or(char, 127, -126, -3, 4, assert_equal_int);
    test_exclusive_or(char, 127, -1, -128, 4, assert_equal_int);
    test_exclusive_or(char, 127, 0, 127, 4, assert_equal_int);
    test_exclusive_or(char, 127, 1, 126, 4, assert_equal_int);
    test_exclusive_or(char, 127, 126, 1, 4, assert_equal_int);
    test_exclusive_or(char, 127, 127, 0, 4, assert_equal_int);
    test_exclusive_or(signed char, -128, -128, 0, 4, assert_equal_int);
    test_exclusive_or(signed char, -128, -127, 1, 4, assert_equal_int);
    test_exclusive_or(signed char, -128, -1, 127, 4, assert_equal_int);
    test_exclusive_or(signed char, -128, 0, -128, 4, assert_equal_int);
    test_exclusive_or(signed char, -128, 1, -127, 4, assert_equal_int);
    test_exclusive_or(signed char, -128, 126, -2, 4, assert_equal_int);
    test_exclusive_or(signed char, -128, 127, -1, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, -128, 1, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, -127, 0, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, -126, 3, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, -1, 126, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, 0, -127, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, 1, -128, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, 126, -1, 4, assert_equal_int);
    test_exclusive_or(signed char, -127, 127, -2, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, -128, 127, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, -127, 126, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, -126, 125, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, -1, 0, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, 0, -1, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, 1, -2, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, 126, -127, 4, assert_equal_int);
    test_exclusive_or(signed char, -1, 127, -128, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, -128, -128, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, -127, -127, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, -126, -126, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, -1, -1, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, 0, 0, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, 1, 1, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, 126, 126, 4, assert_equal_int);
    test_exclusive_or(signed char, 0, 127, 127, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, -128, -127, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, -127, -128, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, -126, -125, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, -1, -2, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, 0, 1, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, 1, 0, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, 126, 127, 4, assert_equal_int);
    test_exclusive_or(signed char, 1, 127, 126, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, -128, -2, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, -127, -1, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, -126, -4, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, -1, -127, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, 0, 126, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, 1, 127, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, 126, 0, 4, assert_equal_int);
    test_exclusive_or(signed char, 126, 127, 1, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, -128, -1, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, -127, -2, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, -126, -3, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, -1, -128, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, 0, 127, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, 1, 126, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, 126, 1, 4, assert_equal_int);
    test_exclusive_or(signed char, 127, 127, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 0, 0, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 0, 1, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 0, 254, 254, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 0, 255, 255, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 1, 0, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 1, 1, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 1, 254, 255, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 1, 255, 254, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 254, 0, 254, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 254, 1, 255, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 254, 254, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 254, 255, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 255, 0, 255, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 255, 1, 254, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 255, 254, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned char, 255, 255, 0, 4, assert_equal_int);
    test_exclusive_or(short, -32768, -32768, 0, 4, assert_equal_int);
    test_exclusive_or(short, -32768, -32767, 1, 4, assert_equal_int);
    test_exclusive_or(short, -32768, -1, 32767, 4, assert_equal_int);
    test_exclusive_or(short, -32768, 0, -32768, 4, assert_equal_int);
    test_exclusive_or(short, -32768, 1, -32767, 4, assert_equal_int);
    test_exclusive_or(short, -32768, 32766, -2, 4, assert_equal_int);
    test_exclusive_or(short, -32768, 32767, -1, 4, assert_equal_int);
    test_exclusive_or(short, -32767, -32768, 1, 4, assert_equal_int);
    test_exclusive_or(short, -32767, -32767, 0, 4, assert_equal_int);
    test_exclusive_or(short, -32767, -32766, 3, 4, assert_equal_int);
    test_exclusive_or(short, -32767, -1, 32766, 4, assert_equal_int);
    test_exclusive_or(short, -32767, 0, -32767, 4, assert_equal_int);
    test_exclusive_or(short, -32767, 1, -32768, 4, assert_equal_int);
    test_exclusive_or(short, -32767, 32766, -1, 4, assert_equal_int);
    test_exclusive_or(short, -32767, 32767, -2, 4, assert_equal_int);
    test_exclusive_or(short, -1, -32768, 32767, 4, assert_equal_int);
    test_exclusive_or(short, -1, -32767, 32766, 4, assert_equal_int);
    test_exclusive_or(short, -1, -32766, 32765, 4, assert_equal_int);
    test_exclusive_or(short, -1, -1, 0, 4, assert_equal_int);
    test_exclusive_or(short, -1, 0, -1, 4, assert_equal_int);
    test_exclusive_or(short, -1, 1, -2, 4, assert_equal_int);
    test_exclusive_or(short, -1, 32766, -32767, 4, assert_equal_int);
    test_exclusive_or(short, -1, 32767, -32768, 4, assert_equal_int);
    test_exclusive_or(short, 0, -32768, -32768, 4, assert_equal_int);
    test_exclusive_or(short, 0, -32767, -32767, 4, assert_equal_int);
    test_exclusive_or(short, 0, -32766, -32766, 4, assert_equal_int);
    test_exclusive_or(short, 0, -1, -1, 4, assert_equal_int);
    test_exclusive_or(short, 0, 0, 0, 4, assert_equal_int);
    test_exclusive_or(short, 0, 1, 1, 4, assert_equal_int);
    test_exclusive_or(short, 0, 32766, 32766, 4, assert_equal_int);
    test_exclusive_or(short, 0, 32767, 32767, 4, assert_equal_int);
    test_exclusive_or(short, 1, -32768, -32767, 4, assert_equal_int);
    test_exclusive_or(short, 1, -32767, -32768, 4, assert_equal_int);
    test_exclusive_or(short, 1, -32766, -32765, 4, assert_equal_int);
    test_exclusive_or(short, 1, -1, -2, 4, assert_equal_int);
    test_exclusive_or(short, 1, 0, 1, 4, assert_equal_int);
    test_exclusive_or(short, 1, 1, 0, 4, assert_equal_int);
    test_exclusive_or(short, 1, 32766, 32767, 4, assert_equal_int);
    test_exclusive_or(short, 1, 32767, 32766, 4, assert_equal_int);
    test_exclusive_or(short, 32766, -32768, -2, 4, assert_equal_int);
    test_exclusive_or(short, 32766, -32767, -1, 4, assert_equal_int);
    test_exclusive_or(short, 32766, -32766, -4, 4, assert_equal_int);
    test_exclusive_or(short, 32766, -1, -32767, 4, assert_equal_int);
    test_exclusive_or(short, 32766, 0, 32766, 4, assert_equal_int);
    test_exclusive_or(short, 32766, 1, 32767, 4, assert_equal_int);
    test_exclusive_or(short, 32766, 32766, 0, 4, assert_equal_int);
    test_exclusive_or(short, 32766, 32767, 1, 4, assert_equal_int);
    test_exclusive_or(short, 32767, -32768, -1, 4, assert_equal_int);
    test_exclusive_or(short, 32767, -32767, -2, 4, assert_equal_int);
    test_exclusive_or(short, 32767, -32766, -3, 4, assert_equal_int);
    test_exclusive_or(short, 32767, -1, -32768, 4, assert_equal_int);
    test_exclusive_or(short, 32767, 0, 32767, 4, assert_equal_int);
    test_exclusive_or(short, 32767, 1, 32766, 4, assert_equal_int);
    test_exclusive_or(short, 32767, 32766, 1, 4, assert_equal_int);
    test_exclusive_or(short, 32767, 32767, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 0, 0, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 0, 1, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 0, 65534, 65534, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 0, 65535, 65535, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 1, 0, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 1, 1, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 1, 65534, 65535, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 1, 65535, 65534, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65534, 0, 65534, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65534, 1, 65535, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65534, 65534, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65534, 65535, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65535, 0, 65535, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65535, 1, 65534, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65535, 65534, 1, 4, assert_equal_int);
    test_exclusive_or(unsigned short, 65535, 65535, 0, 4, assert_equal_int);
    test_exclusive_or(int, -2147483648, -2147483648, 0, 4, assert_equal_int);
    test_exclusive_or(int, -2147483648, -2147483647, 1, 4, assert_equal_int);
    test_exclusive_or(int, -2147483648, -1, 2147483647, 4, assert_equal_int);
    test_exclusive_or(int, -2147483648, 0, -2147483648, 4, assert_equal_int);
    test_exclusive_or(int, -2147483648, 1, -2147483647, 4, assert_equal_int);
    test_exclusive_or(int, -2147483648, 2147483646, -2, 4, assert_equal_int);
    test_exclusive_or(int, -2147483648, 2147483647, -1, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, -2147483648, 1, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, -2147483647, 0, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, -2147483646, 3, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, -1, 2147483646, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, 0, -2147483647, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, 1, -2147483648, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, 2147483646, -1, 4, assert_equal_int);
    test_exclusive_or(int, -2147483647, 2147483647, -2, 4, assert_equal_int);
    test_exclusive_or(int, -1, -2147483648, 2147483647, 4, assert_equal_int);
    test_exclusive_or(int, -1, -2147483647, 2147483646, 4, assert_equal_int);
    test_exclusive_or(int, -1, -2147483646, 2147483645, 4, assert_equal_int);
    test_exclusive_or(int, -1, -1, 0, 4, assert_equal_int);
    test_exclusive_or(int, -1, 0, -1, 4, assert_equal_int);
    test_exclusive_or(int, -1, 1, -2, 4, assert_equal_int);
    test_exclusive_or(int, -1, 2147483646, -2147483647, 4, assert_equal_int);
    test_exclusive_or(int, -1, 2147483647, -2147483648, 4, assert_equal_int);
    test_exclusive_or(int, 0, -2147483648, -2147483648, 4, assert_equal_int);
    test_exclusive_or(int, 0, -2147483647, -2147483647, 4, assert_equal_int);
    test_exclusive_or(int, 0, -2147483646, -2147483646, 4, assert_equal_int);
    test_exclusive_or(int, 0, -1, -1, 4, assert_equal_int);
    test_exclusive_or(int, 0, 0, 0, 4, assert_equal_int);
    test_exclusive_or(int, 0, 1, 1, 4, assert_equal_int);
    test_exclusive_or(int, 0, 2147483646, 2147483646, 4, assert_equal_int);
    test_exclusive_or(int, 0, 2147483647, 2147483647, 4, assert_equal_int);
    test_exclusive_or(int, 1, -2147483648, -2147483647, 4, assert_equal_int);
    test_exclusive_or(int, 1, -2147483647, -2147483648, 4, assert_equal_int);
    test_exclusive_or(int, 1, -2147483646, -2147483645, 4, assert_equal_int);
    test_exclusive_or(int, 1, -1, -2, 4, assert_equal_int);
    test_exclusive_or(int, 1, 0, 1, 4, assert_equal_int);
    test_exclusive_or(int, 1, 1, 0, 4, assert_equal_int);
    test_exclusive_or(int, 1, 2147483646, 2147483647, 4, assert_equal_int);
    test_exclusive_or(int, 1, 2147483647, 2147483646, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, -2147483648, -2, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, -2147483647, -1, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, -2147483646, -4, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, -1, -2147483647, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, 0, 2147483646, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, 1, 2147483647, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, 2147483646, 0, 4, assert_equal_int);
    test_exclusive_or(int, 2147483646, 2147483647, 1, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, -2147483648, -1, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, -2147483647, -2, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, -2147483646, -3, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, -1, -2147483648, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, 0, 2147483647, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, 1, 2147483646, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, 2147483646, 1, 4, assert_equal_int);
    test_exclusive_or(int, 2147483647, 2147483647, 0, 4, assert_equal_int);
    test_exclusive_or(unsigned int, 0U, 0U, 0U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 0U, 1U, 1U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 0U, 4294967294U, 4294967294U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 0U, 4294967295U, 4294967295U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 1U, 0U, 1U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 1U, 1U, 0U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 1U, 4294967294U, 4294967295U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 1U, 4294967295U, 4294967294U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967294U, 0U, 4294967294U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967294U, 1U, 4294967295U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967294U, 4294967294U, 0U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967294U, 4294967295U, 1U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967295U, 0U, 4294967295U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967295U, 1U, 4294967294U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967295U, 4294967294U, 1U, 4, assert_equal_unsigned_int);
    test_exclusive_or(unsigned int, 4294967295U, 4294967295U, 0U, 4, assert_equal_unsigned_int);
    test_exclusive_or(long, -9223372036854775807L, -9223372036854775807L, 0L, 8, assert_equal_long);
    test_exclusive_or(long, -9223372036854775807L, -9223372036854775806L, 3L, 8, assert_equal_long);
    test_exclusive_or(long, -9223372036854775807L, -1L, 9223372036854775806L, 8, assert_equal_long);
    test_exclusive_or(long, -9223372036854775807L, 0L, -9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, -9223372036854775807L, 9223372036854775806L, -1L, 8, assert_equal_long);
    test_exclusive_or(long, -9223372036854775807L, 9223372036854775807L, -2L, 8, assert_equal_long);
    test_exclusive_or(long, -1L, -9223372036854775807L, 9223372036854775806L, 8, assert_equal_long);
    test_exclusive_or(long, -1L, -9223372036854775806L, 9223372036854775805L, 8, assert_equal_long);
    test_exclusive_or(long, -1L, -1L, 0L, 8, assert_equal_long);
    test_exclusive_or(long, -1L, 0L, -1L, 8, assert_equal_long);
    test_exclusive_or(long, -1L, 1L, -2L, 8, assert_equal_long);
    test_exclusive_or(long, -1L, 9223372036854775806L, -9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, 0L, -9223372036854775807L, -9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, 0L, -9223372036854775806L, -9223372036854775806L, 8, assert_equal_long);
    test_exclusive_or(long, 0L, -1L, -1L, 8, assert_equal_long);
    test_exclusive_or(long, 0L, 0L, 0L, 8, assert_equal_long);
    test_exclusive_or(long, 0L, 1L, 1L, 8, assert_equal_long);
    test_exclusive_or(long, 0L, 9223372036854775806L, 9223372036854775806L, 8, assert_equal_long);
    test_exclusive_or(long, 0L, 9223372036854775807L, 9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, 1L, -9223372036854775806L, -9223372036854775805L, 8, assert_equal_long);
    test_exclusive_or(long, 1L, -1L, -2L, 8, assert_equal_long);
    test_exclusive_or(long, 1L, 0L, 1L, 8, assert_equal_long);
    test_exclusive_or(long, 1L, 1L, 0L, 8, assert_equal_long);
    test_exclusive_or(long, 1L, 9223372036854775806L, 9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, 1L, 9223372036854775807L, 9223372036854775806L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775806L, -9223372036854775807L, -1L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775806L, -9223372036854775806L, -4L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775806L, -1L, -9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775806L, 0L, 9223372036854775806L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775806L, 1L, 9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775806L, 9223372036854775806L, 0L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775806L, 9223372036854775807L, 1L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775807L, -9223372036854775807L, -2L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775807L, -9223372036854775806L, -3L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775807L, 0L, 9223372036854775807L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775807L, 1L, 9223372036854775806L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775807L, 9223372036854775806L, 1L, 8, assert_equal_long);
    test_exclusive_or(long, 9223372036854775807L, 9223372036854775807L, 0L, 8, assert_equal_long);
    test_exclusive_or(unsigned long, 0UL, 0UL, 0UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 0UL, 1UL, 1UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 0UL, 18446744073709551614UL, 18446744073709551614UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 0UL, 18446744073709551615UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 1UL, 0UL, 1UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 1UL, 1UL, 0UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 1UL, 18446744073709551614UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 1UL, 18446744073709551615UL, 18446744073709551614UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551614UL, 0UL, 18446744073709551614UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551614UL, 1UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551614UL, 18446744073709551614UL, 0UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551614UL, 18446744073709551615UL, 1UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551615UL, 0UL, 18446744073709551615UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551615UL, 1UL, 18446744073709551614UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551615UL, 18446744073709551614UL, 1UL, 8, assert_equal_unsigned_long);
    test_exclusive_or(unsigned long, 18446744073709551615UL, 18446744073709551615UL, 0UL, 8, assert_equal_unsigned_long);

#undef test_exclusive_or
#undef assert_exclusive_or
}
#endif /* ENABLE_TEST_CASE(TEST_EXCLUSIVE_OR_OPERATOR) */
