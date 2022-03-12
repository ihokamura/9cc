#ifndef QCC_COMPILER
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif /* !QCC_COMPILER */
#include "lib_assert.h"
#include "test_code.h"


/*
print title of a test case
*/
int put_title(const char *title)
{
    return printf("# %s\n", title);
}


#if ENABLE_TEST_CASE(TEST_CONSTANTS)
// 6.4.4 Constants
void test_constants()
{
    put_title("Constants");

    // 6.4.4.1 Integer constants
    assert_constant_decimal_all_digit(1234567890);
    assert_constant_octal_all_digit(012345670);
    assert_constant_hexadecimal_all_digit(0X1234567890ABCDEF);
    assert_constant_hexadecimal_all_digit(0x1234567890abcdef);

    assert_equal_size_t(4, sizeof(2147483646)); assert_constant_int_max_minus1(2147483646);
    assert_equal_size_t(4, sizeof(2147483647)); assert_constant_int_max(2147483647);
    assert_equal_size_t(8, sizeof(2147483648)); assert_constant_int_max_plus1(2147483648);
    assert_equal_size_t(8, sizeof(9223372036854775806)); assert_constant_long_max_minus1(9223372036854775806);
    assert_equal_size_t(8, sizeof(9223372036854775807)); assert_constant_long_max(9223372036854775807);

    assert_equal_size_t(4, sizeof(4294967294U)); assert_constant_unsigned_int_max_minus1(4294967294U);
    assert_equal_size_t(4, sizeof(4294967295U)); assert_constant_unsigned_int_max(4294967295U);
    assert_equal_size_t(8, sizeof(4294967296U)); assert_constant_unsigned_int_max_plus1(4294967296U);
    assert_equal_size_t(8, sizeof(18446744073709551614U)); assert_constant_unsigned_long_max_minus1(18446744073709551614U);
    assert_equal_size_t(8, sizeof(18446744073709551615U)); assert_constant_unsigned_long_max(18446744073709551615U);

    assert_equal_size_t(8, sizeof(9223372036854775806L)); assert_constant_long_max_minus1(9223372036854775806L);
    assert_equal_size_t(8, sizeof(9223372036854775807L)); assert_constant_long_max(9223372036854775807L);

    assert_equal_size_t(8, sizeof(18446744073709551614UL)); assert_constant_unsigned_long_max_minus1(18446744073709551614UL);
    assert_equal_size_t(8, sizeof(18446744073709551615UL)); assert_constant_unsigned_long_max(18446744073709551615UL);

    assert_equal_size_t(8, sizeof(9223372036854775806LL)); assert_constant_long_max_minus1(9223372036854775806LL);
    assert_equal_size_t(8, sizeof(9223372036854775807LL)); assert_constant_long_max(9223372036854775807LL);

    assert_equal_size_t(8, sizeof(18446744073709551614ULL)); assert_constant_unsigned_long_max_minus1(18446744073709551614ULL);
    assert_equal_size_t(8, sizeof(18446744073709551615ULL)); assert_constant_unsigned_long_max(18446744073709551615ULL);

    assert_equal_size_t(4, sizeof(017777777776)); assert_constant_int_max_minus1(017777777776);
    assert_equal_size_t(4, sizeof(017777777777)); assert_constant_int_max(017777777777);
    assert_equal_size_t(4, sizeof(020000000000)); assert_constant_int_max_plus1(020000000000);
    assert_equal_size_t(4, sizeof(037777777777)); assert_constant_unsigned_int_max_minus1(037777777776);
    assert_equal_size_t(4, sizeof(037777777777)); assert_constant_unsigned_int_max(037777777777);
    assert_equal_size_t(8, sizeof(040000000000)); assert_constant_unsigned_int_max_plus1(040000000000);
    assert_equal_size_t(8, sizeof(0777777777777777777776)); assert_constant_long_max_minus1(0777777777777777777776);
    assert_equal_size_t(8, sizeof(0777777777777777777777)); assert_constant_long_max(0777777777777777777777);
    assert_equal_size_t(8, sizeof(01000000000000000000000)); assert_constant_long_max_plus1(01000000000000000000000);
    assert_equal_size_t(8, sizeof(01777777777777777777776)); assert_constant_unsigned_long_max_minus1(01777777777777777777776);
    assert_equal_size_t(8, sizeof(01777777777777777777777)); assert_constant_unsigned_long_max(01777777777777777777777);

    assert_equal_size_t(4, sizeof(037777777777U)); assert_constant_unsigned_int_max_minus1(037777777776U);
    assert_equal_size_t(4, sizeof(037777777777U)); assert_constant_unsigned_int_max(037777777777U);
    assert_equal_size_t(8, sizeof(040000000000U)); assert_constant_unsigned_int_max_plus1(040000000000U);
    assert_equal_size_t(8, sizeof(01777777777777777777776U)); assert_constant_unsigned_long_max_minus1(01777777777777777777776U);
    assert_equal_size_t(8, sizeof(01777777777777777777777U)); assert_constant_unsigned_long_max(01777777777777777777777U);

    assert_equal_size_t(8, sizeof(0777777777777777777776L)); assert_constant_long_max_minus1(0777777777777777777776L);
    assert_equal_size_t(8, sizeof(0777777777777777777777L)); assert_constant_long_max(0777777777777777777777L);
    assert_equal_size_t(8, sizeof(01000000000000000000000L)); assert_constant_long_max_plus1(01000000000000000000000L);
    assert_equal_size_t(8, sizeof(01777777777777777777776L)); assert_constant_unsigned_long_max_minus1(01777777777777777777776L);
    assert_equal_size_t(8, sizeof(01777777777777777777777L)); assert_constant_unsigned_long_max(01777777777777777777777L);

    assert_equal_size_t(8, sizeof(01777777777777777777776UL)); assert_constant_unsigned_long_max_minus1(01777777777777777777776UL);
    assert_equal_size_t(8, sizeof(01777777777777777777777UL)); assert_constant_unsigned_long_max(01777777777777777777777UL);

    assert_equal_size_t(8, sizeof(01777777777777777777776ULL)); assert_constant_unsigned_long_max_minus1(01777777777777777777776ULL);
    assert_equal_size_t(8, sizeof(01777777777777777777777ULL)); assert_constant_unsigned_long_max(01777777777777777777777ULL);

    assert_equal_size_t(4, sizeof(0x7FFFFFFE)); assert_constant_int_max_minus1(0x7FFFFFFE);
    assert_equal_size_t(4, sizeof(0x7FFFFFFF)); assert_constant_int_max(0x7FFFFFFF);
    assert_equal_size_t(4, sizeof(0x80000000)); assert_constant_int_max_plus1(0x80000000);
    assert_equal_size_t(4, sizeof(0xFFFFFFFF)); assert_constant_unsigned_int_max_minus1(037777777776);
    assert_equal_size_t(4, sizeof(0xFFFFFFFF)); assert_constant_unsigned_int_max(0xFFFFFFFF);
    assert_equal_size_t(8, sizeof(0x100000000)); assert_constant_unsigned_int_max_plus1(0x100000000);
    assert_equal_size_t(8, sizeof(0x7FFFFFFFFFFFFFFE)); assert_constant_long_max_minus1(0x7FFFFFFFFFFFFFFE);
    assert_equal_size_t(8, sizeof(0x7FFFFFFFFFFFFFFF)); assert_constant_long_max(0x7FFFFFFFFFFFFFFF);
    assert_equal_size_t(8, sizeof(0x8000000000000000)); assert_constant_long_max_plus1(0x8000000000000000);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFE)); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFE);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFF)); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFF);

    assert_equal_size_t(4, sizeof(0xFFFFFFFFU)); assert_constant_unsigned_int_max_minus1(037777777776U);
    assert_equal_size_t(4, sizeof(0xFFFFFFFFU)); assert_constant_unsigned_int_max(0xFFFFFFFFU);
    assert_equal_size_t(8, sizeof(0x100000000U)); assert_constant_unsigned_int_max_plus1(0x100000000U);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFEU)); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEU);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFFU)); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFU);

    assert_equal_size_t(8, sizeof(0x7FFFFFFFFFFFFFFEL)); assert_constant_long_max_minus1(0x7FFFFFFFFFFFFFFEL);
    assert_equal_size_t(8, sizeof(0x7FFFFFFFFFFFFFFFL)); assert_constant_long_max(0x7FFFFFFFFFFFFFFFL);
    assert_equal_size_t(8, sizeof(0x8000000000000000L)); assert_constant_long_max_plus1(0x8000000000000000L);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFEL)); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEL);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFFL)); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFL);

    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFEUL)); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEUL);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFFUL)); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFUL);

    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFEULL)); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEULL);
    assert_equal_size_t(8, sizeof(0xFFFFFFFFFFFFFFFFULL)); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFULL);

    assert_equal_size_t(4, sizeof(1));
    assert_equal_size_t(4, sizeof(1U)); assert_equal_size_t(4, sizeof(1u));
    assert_equal_size_t(8, sizeof(1L)); assert_equal_size_t(8, sizeof(1l));
    assert_equal_size_t(8, sizeof(1UL)); assert_equal_size_t(8, sizeof(1Ul)); assert_equal_size_t(8, sizeof(1uL)); assert_equal_size_t(8, sizeof(1ul));
    assert_equal_size_t(8, sizeof(1LL)); assert_equal_size_t(8, sizeof(1ll));
    assert_equal_size_t(8, sizeof(1ULL)); assert_equal_size_t(8, sizeof(1Ull)); assert_equal_size_t(8, sizeof(1uLL)); assert_equal_size_t(8, sizeof(1ull));

    assert_equal_size_t(4, sizeof(01));
    assert_equal_size_t(4, sizeof(01U)); assert_equal_size_t(4, sizeof(01u));
    assert_equal_size_t(8, sizeof(01L)); assert_equal_size_t(8, sizeof(01l));
    assert_equal_size_t(8, sizeof(01UL)); assert_equal_size_t(8, sizeof(01Ul)); assert_equal_size_t(8, sizeof(01uL)); assert_equal_size_t(8, sizeof(01ul));
    assert_equal_size_t(8, sizeof(01LL)); assert_equal_size_t(8, sizeof(01ll));
    assert_equal_size_t(8, sizeof(01ULL)); assert_equal_size_t(8, sizeof(01Ull)); assert_equal_size_t(8, sizeof(01uLL)); assert_equal_size_t(8, sizeof(01ull));

    assert_equal_size_t(4, sizeof(0x1));
    assert_equal_size_t(4, sizeof(0x1U)); assert_equal_size_t(4, sizeof(0x1u));
    assert_equal_size_t(8, sizeof(0x1L)); assert_equal_size_t(8, sizeof(0x1l));
    assert_equal_size_t(8, sizeof(0x1UL)); assert_equal_size_t(8, sizeof(0x1Ul)); assert_equal_size_t(8, sizeof(0x1uL)); assert_equal_size_t(8, sizeof(0x1ul));
    assert_equal_size_t(8, sizeof(0x1LL)); assert_equal_size_t(8, sizeof(0x1ll));
    assert_equal_size_t(8, sizeof(0x1ULL)); assert_equal_size_t(8, sizeof(0x1Ull)); assert_equal_size_t(8, sizeof(0x1uLL)); assert_equal_size_t(8, sizeof(0x1ull));

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    // 6.4.4.2 Floating constants
    float f = 1.0f; assert_equal_float(1.0f, f);
    double d = 2.0; assert_equal_double(2.0, d);
#endif /* INCLUDE_FLOATING_POINT_TYPE */
}
#endif /* ENABLE_TEST_CASE(TEST_CONSTANTS) */


#if ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS)
// 6.5.6 Additive operators
void test_additive_operators()
{
    put_title("Additive operators");

    assert_equal_size_t(4, sizeof((signed char)126 + (signed char)0)); assert_equal_int(126, (signed char)126 + (signed char)0);
    assert_equal_size_t(4, sizeof((signed char)126 + (signed char)1)); assert_equal_int(127, (signed char)126 + (signed char)1);
    assert_equal_size_t(4, sizeof((signed char)126 + (signed char)2)); assert_equal_int(128, (signed char)126 + (signed char)2);
    assert_equal_size_t(4, sizeof((unsigned char)254 + (unsigned char)0)); assert_equal_int(254, (unsigned char)254 + (unsigned char)0);
    assert_equal_size_t(4, sizeof((unsigned char)254 + (unsigned char)1)); assert_equal_int(255, (unsigned char)254 + (unsigned char)1);
    assert_equal_size_t(4, sizeof((unsigned char)254 + (unsigned char)2)); assert_equal_int(256, (unsigned char)254 + (unsigned char)2);
    assert_equal_size_t(4, sizeof((short)32766 + (short)0)); assert_equal_int(32766, (short)32766 + (short)0);
    assert_equal_size_t(4, sizeof((short)32766 + (short)1)); assert_equal_int(32767, (short)32766 + (short)1);
    assert_equal_size_t(4, sizeof((short)32766 + (short)2)); assert_equal_int(32768, (short)32766 + (short)2);
    assert_equal_size_t(4, sizeof((unsigned short)65534 + (unsigned short)0)); assert_equal_int(65534, (unsigned short)65534 + (unsigned short)0);
    assert_equal_size_t(4, sizeof((unsigned short)65534 + (unsigned short)1)); assert_equal_int(65535, (unsigned short)65534 + (unsigned short)1);
    assert_equal_size_t(4, sizeof((unsigned short)65534 + (unsigned short)2)); assert_equal_int(65536, (unsigned short)65534 + (unsigned short)2);
    assert_equal_size_t(4, sizeof(2147483646 + 0)); assert_equal_int(2147483646, 2147483646 + 0);
    assert_equal_size_t(4, sizeof(2147483646 + 1)); assert_equal_int(2147483647, 2147483646 + 1);
    assert_equal_size_t(4, sizeof(2147483647 + 2)); // assert_equal_int(-2147483648, 2147483646 + 2); // This is implementation-defined or raises an implementation-defined signal.
    assert_equal_size_t(8, sizeof(2147483647L + 2L)); assert_equal_long(2147483648L, 2147483646L + 2L);
    assert_equal_size_t(4, sizeof(4294967294U + 0U)); assert_equal_unsigned_int(4294967294U, 4294967294U + 0U);
    assert_equal_size_t(4, sizeof(4294967294U + 1U)); assert_equal_unsigned_int(4294967295U, 4294967294U + 1U);
    assert_equal_size_t(4, sizeof(4294967294U + 2U)); assert_equal_unsigned_int(0U, 4294967294U + 2U);
    assert_equal_size_t(8, sizeof((long)4294967294U + (long)2U)); assert_equal_long(4294967296L, (long)4294967294U + (long)2U);
    assert_equal_size_t(8, sizeof(9223372036854775806L + 0L)); assert_equal_long(9223372036854775806L, 9223372036854775806L + 0L);
    assert_equal_size_t(8, sizeof(9223372036854775806L + 1L)); assert_equal_long(9223372036854775807L, 9223372036854775806L + 1L);
    assert_equal_size_t(8, sizeof(9223372036854775806L + 2L)); // assert_equal_long(-9223372036854775808L, 9223372036854775806L + 2L); // This is implementation-defined or raises an implementation-defined signal.
    assert_equal_size_t(8, sizeof(9223372036854775806UL + 2UL)); assert_equal_unsigned_long(9223372036854775808UL, 9223372036854775806UL + 2UL);
    assert_equal_size_t(8, sizeof(18446744073709551614UL + 0UL)); assert_equal_unsigned_long(18446744073709551614UL, 18446744073709551614UL + 0UL);
    assert_equal_size_t(8, sizeof(18446744073709551614UL + 1UL)); assert_equal_unsigned_long(18446744073709551615UL, 18446744073709551614UL + 1UL);
    assert_equal_size_t(8, sizeof(18446744073709551614UL + 2UL)); assert_equal_unsigned_long(0UL, 18446744073709551614UL + 2UL);

    assert_equal_size_t(4, sizeof((signed char)1 - (signed char)0)); assert_equal_int(1, (signed char)1 - (signed char)0);
    assert_equal_size_t(4, sizeof((signed char)1 - (signed char)1)); assert_equal_int(0, (signed char)1 - (signed char)1);
    assert_equal_size_t(4, sizeof((signed char)1 - (signed char)2)); assert_equal_int(-1, (signed char)1 - (signed char)2);
    assert_equal_size_t(4, sizeof((signed char)(-127) - (signed char)0)); assert_equal_int(-127, (signed char)(-127) - (signed char)0);
    assert_equal_size_t(4, sizeof((signed char)(-127) - (signed char)1)); assert_equal_int(-128, (signed char)(-127) - (signed char)1);
    assert_equal_size_t(4, sizeof((signed char)(-127) - (signed char)2)); assert_equal_int(-129, (signed char)(-127) - (signed char)2);
    assert_equal_size_t(4, sizeof((unsigned char)1 - (unsigned char)0)); assert_equal_int(1, (unsigned char)1 - (unsigned char)0);
    assert_equal_size_t(4, sizeof((unsigned char)1 - (unsigned char)1)); assert_equal_int(0, (unsigned char)1 - (unsigned char)1);
    assert_equal_size_t(4, sizeof((unsigned char)1 - (unsigned char)2)); assert_equal_int(-1, (unsigned char)1 - (unsigned char)2);
    assert_equal_size_t(4, sizeof((short)1 - (short)0)); assert_equal_int(1, (short)1 - (short)0);
    assert_equal_size_t(4, sizeof((short)1 - (short)1)); assert_equal_int(0, (short)1 - (short)1);
    assert_equal_size_t(4, sizeof((short)1 - (short)2)); assert_equal_int(-1, (short)1 - (short)2);
    assert_equal_size_t(4, sizeof((short)(-32767) - (short)0)); assert_equal_int(-32767, (short)(-32767) - (short)0);
    assert_equal_size_t(4, sizeof((short)(-32767) - (short)1)); assert_equal_int(-32768, (short)(-32767) - (short)1);
    assert_equal_size_t(4, sizeof((short)(-32767) - (short)2)); assert_equal_int(-32769, (short)(-32767) - (short)2);
    assert_equal_size_t(4, sizeof((unsigned short)1 - (unsigned short)0)); assert_equal_int(1, (unsigned short)1 - (unsigned short)0);
    assert_equal_size_t(4, sizeof((unsigned short)1 - (unsigned short)1)); assert_equal_int(0, (unsigned short)1 - (unsigned short)1);
    assert_equal_size_t(4, sizeof((unsigned short)1 - (unsigned short)2)); assert_equal_int(-1, (unsigned short)1 - (unsigned short)2);
    assert_equal_size_t(4, sizeof(1 - 0)); assert_equal_int(1, 1 - 0);
    assert_equal_size_t(4, sizeof(1 - 1)); assert_equal_int(0, 1 - 1);
    assert_equal_size_t(4, sizeof(1 - 2)); assert_equal_int(-1, 1 - 2);
    assert_equal_size_t(4, sizeof(-2147483647 - 0)); assert_equal_int(-2147483647, -2147483647 - 0);
    assert_equal_size_t(4, sizeof(-2147483647 - 1)); assert_equal_int(-2147483648, -2147483647 - 1);
    assert_equal_size_t(4, sizeof(-2147483647 - 2)); // assert_equal_int(-2147483649, -2147483647 - 2);  // This is implementation-defined or raises an implementation-defined signal.
    assert_equal_size_t(4, sizeof(1U - 0U)); assert_equal_unsigned_int(1U, 1U - 0U);
    assert_equal_size_t(4, sizeof(1U - 1U)); assert_equal_unsigned_int(0U, 1U - 1U);
    assert_equal_size_t(4, sizeof(1U - 2U)); assert_equal_unsigned_int(4294967295U, 1U - 2U);
    assert_equal_size_t(8, sizeof(1L - 0L)); assert_equal_int(1L, 1L - 0L);
    assert_equal_size_t(8, sizeof(1L - 1L)); assert_equal_int(0L, 1L - 1L);
    assert_equal_size_t(8, sizeof(1L - 2L)); assert_equal_int(-1L, 1L - 2L);
    assert_equal_size_t(8, sizeof(-9223372036854775807L - 0L)); assert_equal_long(-9223372036854775807L, -9223372036854775807L - 0L);
    assert_equal_size_t(8, sizeof(-9223372036854775807L - 1L)); // assert_equal_long(-9223372036854775808L, -9223372036854775807L - 1L); // The first operand cannot be represented as constant.
    assert_equal_size_t(8, sizeof(-9223372036854775807L - 2L)); // assert_equal_long(-9223372036854775809L, -9223372036854775807L - 2L); // The first operand cannot be represented as constant.
    assert_equal_size_t(8, sizeof(1UL - 0UL)); assert_equal_unsigned_long(1UL, 1UL - 0UL);
    assert_equal_size_t(8, sizeof(1UL - 1UL)); assert_equal_unsigned_long(0UL, 1UL - 1UL);
    assert_equal_size_t(8, sizeof(1UL - 2UL)); assert_equal_unsigned_long(18446744073709551615UL, 1UL - 2UL);

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    assert_equal_float(21.0f, 5.0f+20.0f-4.0f);
    assert_equal_double(21.0, 5.0+20.0-4.0);
#endif /* INCLUDE_FLOATING_POINT_TYPE */
}
#endif /* ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS) */


#if ENABLE_TEST_CASE(TEST_OTHERS)
/*
multiplicative operator
*/
int test_multiplicative()
{
    assert_equal_int(47, 5 + 6 * 7);
    assert_equal_int(15, 5 * (9 - 6));
    assert_equal_int(4, (3 + 5) / 2);
    assert_equal_int(0, 123 % 3);
    assert_equal_int(1, ((123 + 1) % 5) % 3);

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    assert_equal_float(3.0f, 0.5f * (9.0f - 6.0f / 2.0f));
    assert_equal_double(3.0, 0.5 * (9.0 - 6.0 / 2.0));
#endif /* INCLUDE_FLOATING_POINT_TYPE */

    return 0;
}


/*
shift operator
*/
int test_shift()
{
    assert_equal_int(4, 1 << 2);
    assert_equal_int(40, 5 << 3);
    assert_equal_int(2, 16 >> 3);
    assert_equal_int(1, 15 >> 3);

    return 0;
}


/*
bitwise AND operator
*/
int test_bitwise_and()
{
    assert_equal_int(0, 1 & 16);
    assert_equal_int(1, 1 & 15);
    assert_equal_int(0, 3 & 12);
    assert_equal_int(0, 0 & 12);

    return 0;
}


/*
bitwise exclusive OR operator
*/
int test_bitwise_xor()
{
    assert_equal_int(17, 1 ^ 16);
    assert_equal_int(14, 1 ^ 15);
    assert_equal_int(15, 3 ^ 12);
    assert_equal_int(12, 0 ^ 12);

    return 0;
}


/*
bitwise inclusive OR operator
*/
int test_bitwise_or()
{
    assert_equal_int(17, 1 | 16);
    assert_equal_int(15, 1 | 15);
    assert_equal_int(15, 3 | 12);
    assert_equal_int(12, 0 | 12);

    return 0;
}


/*
logical AND operator
*/
int test_logical_and()
{
    assert_equal_int(0, 0 && 0);
    assert_equal_int(0, 0 && 1);
    assert_equal_int(0, 1 && 0);
    assert_equal_int(1, 1 && 1);

    int a = 0; int b = 0;
    assert_equal_int(0, a++ && ++b); assert_equal_int(1, a); assert_equal_int(0, b);
    assert_equal_int(1, ++a && ++b); assert_equal_int(2, a); assert_equal_int(1, b);

    return 0;
}


/*
logical OR operator
*/
int test_logical_or()
{
    assert_equal_int(0, 0 || 0);
    assert_equal_int(1, 0 || 1);
    assert_equal_int(1, 1 || 0);
    assert_equal_int(1, 1 || 1);

    int a = 0; int b = 0;
    assert_equal_int(1, a++ || ++b); assert_equal_int(1, a); assert_equal_int(1, b);
    assert_equal_int(1, ++a || ++b); assert_equal_int(2, a); assert_equal_int(1, b);

    return 0;
}


/*
conditional operator
*/
int test_conditional()
{
    int a = 1; int b = 0; int r;
    r = a ? 123 : 456; assert_equal_int(123, r);
    r = b ? 123 : 456; assert_equal_int(456, r);
    r = a-- ? b++ : b--; assert_equal_int(0, r); assert_equal_int(0, a); assert_equal_int(1, b);
    r = a++ ? ++b : --b; assert_equal_int(0, r); assert_equal_int(1, a); assert_equal_int(0, b);

    return 0;
}


/*
generic-selection
*/
short func_def_square_short(short x)
{
    return x * x;
}
int func_def_square_int(int x)
{
    return x * x;
}
long func_def_square_long(long x)
{
    return x * x;
}
int test_generic_selection()
{
    short s = _Generic(s, short: func_def_square_short(1), int: func_def_square_int(2)); assert_equal_short(1, s);
    int i = _Generic(i, short: func_def_square_short(1), int: func_def_square_int(2), default: func_def_square_long(3)); assert_equal_short(4, i);
    long l1 = _Generic(l1, short: func_def_square_short(1), int: func_def_square_int(2), default: func_def_square_long(3)); assert_equal_short(9, l1);
    long l2 = _Generic(l2, short: func_def_square_short(11), default: func_def_square_long(33), int: func_def_square_int(22)); assert_equal_short(1089, l2);

    return 0;
}


/*
compound-literal
*/
int test_compound_literal()
{
    int i = (int){1}; assert_equal_int(1, i);

    int *p = (int []){2, 4, 6}; assert_equal_int(2, p[0]); assert_equal_int(4, p[1]); assert_equal_int(6, p[2]);
    p = (int [2]){*p, }; assert_equal_int(2, p[0]); assert_equal_int(0, p[1]);

    typedef struct {char m1; int m2; short m3;} st_type;
    st_type st1 = (st_type){1, 2, 3}; assert_equal_char(1, st1.m1); assert_equal_int(2, st1.m2); assert_equal_short(3, st1.m3);
    st_type st2 = (st_type){.m2 = 22, .m1 = 11, }; assert_equal_char(11, st2.m1); assert_equal_int(22, st2.m2); assert_equal_short(0, st2.m3);
    assert_equal_long(210, func_call_struct1((struct param_t1){0, 1, 2}));
    assert_equal_long(543, func_call_struct1((struct param_t1){.m1 = 4, .m2 = 5, .m0 = 3}));
    assert_equal_int(55, func_call_arg_array((int [10]){1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    assert_equal_int(6, func_call_arg_array((int [10]){[9] = 1, [1] = 2, 3}));

    return 0;
}


/*
unary operator
*/
int test_unary()
{
    assert_equal_int(10, -10 + 20);
    assert_equal_int(1, +1 * -2 - (-3));

    int a; a = 123; assert_equal_int(124, ++a); assert_equal_int(124, a);
    int b; b = 123; assert_equal_int(122, --b); assert_equal_int(122, b);
    int c; c = 123; assert_equal_int(123, c++); assert_equal_int(124, c);
    int d; d = 123; assert_equal_int(123, d--); assert_equal_int(122, d);
    int e; e = 15; assert_equal_int(-16, ~e);
    int f; f = 15; assert_equal_int(0, !f); assert_equal_int(1, !(!f));

    int *p; alloc4(&p, 1, 2, 4, 8);
    ++p; assert_equal_int(4, *(++p)); assert_equal_int(4, *p);
    --p; assert_equal_int(1, *(--p)); assert_equal_int(1, *p);
    p++; assert_equal_int(2, *(p++)); assert_equal_int(4, *p);
    p--; assert_equal_int(2, *(p--)); assert_equal_int(1, *p);

    return 0;
}


/*
cast operator
*/
int test_cast()
{
    long l;

    char c;
    l = 255; c = (char)l; assert_equal_char(255, c);
    l = 256; c = (char)l; assert_equal_char(0, c);

    short s;
    l = 65535; s = (short)l; assert_equal_short(65535, s);
    l = 65536; s = (short)l; assert_equal_short(0, s);

    int i;
    l = 4294967295; i = (int)l; assert_equal_int(4294967295, i);
    l = 4294967296; i = (int)l; assert_equal_int(0, i);

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    float f = 1.0f; assert_equal_double(1.0, (double)f); assert_equal_int(1, (int)f); assert_equal_float(f, (float)1);
    double d = 2.0; assert_equal_float(2.0f, (float)d); assert_equal_int(2, (int)d); assert_equal_double(d, (double)2);
#endif /* INCLUDE_FLOATING_POINT_TYPE */

    return 0;
}


/*
comparision operator
*/
int test_comparision()
{
    assert_equal_int(1, 42 == 6 * 7);
    assert_equal_int(0, 42 != 6 * 7);
    assert_equal_int(0, 42 < 6 * 7);
    assert_equal_int(1, 42 <= 6 * 7);
    assert_equal_int(0, 42 > 6 * 7);
    assert_equal_int(1, 42 >= 6 * 7);

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

    return 0;
}


/*
local variable
*/
int test_local_variable()
{
    int a; int b; a = 1; b = 2; assert_equal_int(3, a + b);
    int x; int y; int q; x = 42; y = x / 2; q = (x - y) / 7; assert_equal_int(3, q);
    int alpha; int beta; int gamma; alpha = 2; beta = 5 * alpha; gamma = beta - 3 * alpha; assert_equal_int(4, gamma);

    return 0;
}


/*
assignment
*/
int test_assignment()
{
    int a; assert_equal_int(123, a = 123);
    int b; b = 123; assert_equal_int(369, b += 246);
    int c; c = 123; assert_equal_int(246, c -= -123);
    int d; d = 123; assert_equal_int(369, d *= 3);
    int e; e = 123; assert_equal_int(41, e /= 3);
    int f; f = 123; assert_equal_int(0, f %= 3);
    int g; g = 4; assert_equal_int(16, g <<= 2);
    int h; h = 4; assert_equal_int(1, h >>= 2);
    int i; i = 8; assert_equal_int(8, i &= 12);
    int j; j = 8; assert_equal_int(4, j ^= 12);
    int k; k = 8; assert_equal_int(12, k |= 12);

    int *p; alloc4(&p, 1, 2, 4, 8);
    p += 2; assert_equal_int(*p, 4);
    p -= 1; assert_equal_int(*p, 2);

    return 0;
}


/*
comma operator
*/
int test_comma()
{
    int a = 0; volatile int b = 123; int r;

    r = a, b; assert_equal_int(0, r);
    r = (a++, b++); assert_equal_int(123, r); assert_equal_int(1, a); assert_equal_int(124, b);
    r = ++a, ++b; assert_equal_int(2, r); assert_equal_int(2, a); assert_equal_int(125, b);

    return 0;
}


/*
goto statement
*/
int func_def_goto(int condition)
{
    if(condition == 1)
    {
        goto label1;
    }

    if(condition == 2)
    {
        goto label2;
    }

    return 0;

label1:
    return 1;

label2:
    return 2;
}
int test_goto_statement()
{
    int condition; int r;
    condition = 1; r = func_def_goto(condition); assert_equal_int(1, r);
    condition = 2; r = func_def_goto(condition); assert_equal_int(2, r);
    condition = 0; r = func_def_goto(condition); assert_equal_int(0, r);

    return 0;
}


/*
if statement
*/
int test_if_statement()
{
    int tmp; int condition;
    tmp = 3; condition = 1; if(condition == 1) tmp = tmp * 2; assert_equal_int(6, tmp);
    tmp = 3; condition = 1; if(condition != 0) tmp = tmp * 2; else tmp = tmp * 3; assert_equal_int(6, tmp);
    tmp = 3; condition = 0; if(condition) tmp = tmp * 2; else tmp = tmp * 3; assert_equal_int(9, tmp);

    return 0;
}


/*
switch statement
*/
int func_def_switch1(int condition)
{
    switch(condition)
    {
    case 1:
        return 1;

    case 3:
        return 3;

    default:
        return 0;
    }
}
int func_def_switch2(int condition)
{
    int r = 0;

    switch(condition)
    {
    case 1:
        r = 1;
        break;

    case 2:
        r = 2;
        ; // intentionally omit break statement

    case 3:
        r = 3;
        break;

    default:
        r = 0;
        break;
    }

    return r;
}
int func_def_switch3(int condition1, int condition2)
{
    int r = 0;

    switch(condition1)
    {
    case 1:
        r = 1;
        break;

    case 2:
    {
        switch(condition2)
        {
        case 5:
            r = 5;
            break;

        case 6:
            r = 6;
            break;

        default:
            r = -1;
            break;
        }
        break;
    }

    case 3:
        r = 3;
        break;

    default:
        r = 0;
        break;
    }

    return r;
}
int test_switch_statement()
{
    int condition; int condition1; int condition2; int r;
    condition = 1; r = func_def_switch1(condition); assert_equal_int(1, r);
    condition = 3; r = func_def_switch1(condition); assert_equal_int(3, r);
    condition = 5; r = func_def_switch1(condition); assert_equal_int(0, r);
    condition = 1; r = func_def_switch2(condition); assert_equal_int(1, r);
    condition = 2; r = func_def_switch2(condition); assert_equal_int(3, r);
    condition = 3; r = func_def_switch2(condition); assert_equal_int(3, r);
    condition = 5; r = func_def_switch2(condition); assert_equal_int(0, r);
    condition1 = 1; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_equal_int(1, r);
    condition1 = 2; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_equal_int(5, r);
    condition1 = 2; condition2 = 6; r = func_def_switch3(condition1, condition2); assert_equal_int(6, r);
    condition1 = 2; condition2 = 7; r = func_def_switch3(condition1, condition2); assert_equal_int(-1, r);
    condition1 = 3; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_equal_int(3, r);
    condition1 = 5; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_equal_int(0, r);

    return 0;
}


/*
while statement
*/
int test_while_statement()
{
    int sum; int i;
    sum = i = 0; while(i < 10){sum = sum + i; i = i + 1;} assert_equal_int(45, sum);
    sum = i = 0; while(1){if(i >= 10){assert_equal_int(45, sum); return sum;} sum = sum + i; i = i + 1;}
    sum = i = 0; while(1){if(i >= 10){break;} sum = sum + i; ++i;} assert_equal_int(45, sum);
    sum = i = 0; while(1){if(i >= 100){sum += i; break;} if(i >= 10){continue;} sum = sum + i; ++i;} assert_equal_int(145, sum);

    return 0;
}


/*
do statement
*/
int test_do_statement()
{
    int sum; int i;
    sum = i = 0; do{sum = sum + i; i = i + 1;} while(i < 10); assert_equal_int(45, sum);
    sum = i = 0; do{if(i >= 10){assert_equal_int(45, sum); return sum;} else {sum = sum + i; i = i + 1;}} while(1);
    sum = i = 0; do{if(i >= 10){break;} else {sum = sum + i; ++i;}} while(1); assert_equal_int(45, sum);
    sum = i = 0; do{if(i >= 100){sum += i; break;} else if(i >= 10){continue;} else {sum = sum + i; ++i;}} while(1); assert_equal_int(145, sum);

    return 0;
}


/*
for statement
*/
int test_for_statement()
{
    int sum; int i; int j;
    sum = 0; for(i = 0; i < 10; i = i + 1){sum = sum + i;} assert_equal_int(45, sum);
    sum = 0; i = 0; for(; i < 10; i = i + 1){sum = sum + i;} assert_equal_int(45, sum);
    sum = 0; for(i = 0; i < 10; ){sum = sum + i; i = i + 1;} assert_equal_int(45, sum);
    sum = 0; for(int i = 1; i <= 10; i++){sum = sum + i;} assert_equal_int(55, sum); assert_equal_int(10, i);
    sum = 0; i = 0; for( ; ; ){sum = sum + i; if(++i >= 10){break;}} assert_equal_int(45, sum);
    sum = 0; i = 0; for( ; ; i++){if(i >= 100){sum += i; break;} else if(i >= 10){continue;} else{sum = sum + i;}} assert_equal_int(145, sum);
    sum = 0; for(i = 0; ; i++){if(i >= 5){break;} for(j = 0; ; j++){if(j >= 10){break;} sum += j;}} assert_equal_int(225, sum);
    sum = 0; for(i = 0; i < 10; i++){if(i >= 5){continue;} for(j = 0; j < 100; ++j){if(j >= 10){continue;} sum += j;}} assert_equal_int(225, sum);

    return 0;
}


/*
function call
*/
int test_function_call()
{
    assert_equal_int(0, func_call_return0());
    assert_equal_int(3, func_call_return1() + func_call_return2());
    assert_equal_int(3, func_call_add(1, 2));
    assert_equal_int(543210, func_call_arg6(0, 1, 2, 3, 4, 5));
    assert_equal_int(6543210, func_call_arg7(0, 1, 2, 3, 4, 5, 6));
    assert_equal_int(76543210, func_call_arg8(0, 1, 2, 3, 4, 5, 6, 7));
    struct param_t1 s1 = {0, 1, 2}; assert_equal_long(210, func_call_struct1(s1));
    struct param_t2 s2 = {0, 1, 2}; assert_equal_long(210, func_call_struct2(s2));
    struct param_t3 s3 = {7, 8, 9}; assert_equal_long(9876543210, func_call_struct3(0, 1, 2, 3, 4, s3, 5, 6));
    struct param_t4 s4 = {0, 1, 2}; assert_equal_long(210, func_call_struct4(s4));
    struct param_t5 s5 = func_call_struct5(11, 22, 33); assert_equal_char(11, s5.m0); assert_equal_char(22, s5.m1); assert_equal_int(33, s5.m2);
    struct param_t6 s6 = func_call_struct6(11, 22); assert_equal_long(11, s6.m0); assert_equal_long(22, s6.m1);
    struct param_t7 s7 = func_call_struct7(11, 22, 33); assert_equal_long(11, s7.m0); assert_equal_long(22, s7.m1); assert_equal_char(33, s7.m2);
    struct param_t8 s8 = func_call_struct8(1, 2, 3, 10, 20, 30, 100); assert_equal_long(111, s8.m0); assert_equal_long(22, s8.m1); assert_equal_long(33, s8.m2);
    assert_equal_int(10, func_call_variadic(5, 0, 1, 2, 3, 4)); assert_equal_int(15, func_call_variadic(6, 0, 1, 2, 3, 4, 5)); assert_equal_int(21, func_call_variadic(7, 0, 1, 2, 3, 4, 5, 6));

    int (*pf)(void);
    pf = func_call_return0; assert_equal_int(0, pf());
    pf = &func_call_return1; assert_equal_int(1, pf());
    pf = func_call_return2; assert_equal_int(2, (**func_call_return2)()); assert_equal_int(2, (*pf)()); assert_equal_int(2, (***pf)());
    int (*af[3])(void) = {func_call_return0, func_call_return1, func_call_return2}; int i; for(i = 0; i < sizeof(af) / sizeof(af[0]); i++) {assert_equal_int(i, af[i]());}

    int (*pf2)(int, int);
    pf2 = func_call_add; assert_equal_int(3, pf2(1, 2));
    pf2 = &func_call_add; assert_equal_int(3, pf2(1, 2));

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    assert_equal_float(1.0, func_call_return_float(1.0f));
    assert_equal_double(2.0, func_call_return_double(2.0));
    func_call_arg8_double(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    func_call_arg9_double(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
#endif /* INCLUDE_FLOATING_POINT_TYPE */

    return 0;
}


/*
function definition
*/
int func_def_return0()
{
    return 0;
}
int func_def_return1()
{
    return 1;
}
int func_def_return2()
{
    return 2;
}
int func_def_add(int x, int y)
{
    return x + y;
}
int func_def_arg6(int a0, int a1, int a2, int a3, int a4, int a5)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000;
}
int func_def_arg7(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000;
}
int func_def_arg8(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000 + a7*10000000;
}
long func_def_struct1(struct param_t1 s)
{
    return s.m0 + s.m1*10 + s.m2*100;
}
long func_def_struct2(struct param_t2 s)
{
    return s.m0 + s.m1*10 + s.m2*100;
}
long func_def_struct3(int a0, int a1, int a2, int a3, int a4, struct param_t3 s, int a5, int a6)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000 + 10000000*(s.m0 + s.m1*10 + s.m2*100);
}
long func_def_struct4(struct param_t4 s)
{
    return s.m0 + s.m1*10 + s.m2*100;
}
struct param_t5 func_def_struct5(char a0, char a1, int a2)
{
    struct param_t5 s = {a0, a1, a2};

    return s;
}
struct param_t6 func_def_struct6(long a0, long a1)
{
    struct param_t6 s = {a0, a1};

    return s;
}
struct param_t7 func_def_struct7(long a0, long a1, char a2)
{
    struct param_t7 s = {a0, a1, a2};

    return s;
}
struct param_t8 func_def_struct8(long a0, long a1, long a2, int a3, int a4, int a5, int a6)
{
    struct param_t8 s = {a0 + a3 + a6, a1 + a4, a2 + a5};

    return s;
}
int func_def_arg_array(int a[10])
{
    int sum = 0;
    int i;
    for(i = 0; i < 10; i++)
    {
        sum += a[i];
    }

    return sum;
}
int func_def_arg_func(int f(void))
{
    return f();
}
int func_def_factorial(int n)
{
    if(n == 0)
    {
        return 1;
    }
    else
    {
        return n * func_def_factorial(n - 1);
    }
}
int func_def_fibonacci(int n)
{
    if(n == 0)
    {
        return 0;
    }
    else if (n == 1)
    {
        return 1;
    }
    else
    {
        return func_def_fibonacci(n - 2) + func_def_fibonacci(n - 1);
    }
}
int func_def_variadic(int count, ...) 
{
    va_list ap;
    __builtin_va_start(ap, count);

    int sum = 0;
    long *reg_base = ap->reg_save_area;
    long *stack_base = ap->overflow_arg_area;
    for (int i = 1; i <= count; i++)
    {
        int arg = (i < 6) ? reg_base[i] : stack_base[i - 6];
        sum += arg;
    }

    return sum;
}
static inline int func_def_inline(int i)
{
    return i;
}
void _Noreturn func_def_noreturn(int status)
{
    exit(status);
}
static inline void _Noreturn func_def_inline_noreturn(int status)
{
    exit(status);
}
#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
float func_def_return_float(double f)
{
    return f;
}
double func_def_return_double(double d)
{
    return d;
}
#endif /* INCLUDE_FLOATING_POINT_TYPE */
int test_function_definition()
{
    assert_equal_int(0, func_def_return0());
    assert_equal_int(3, func_def_return1() + func_def_return2());
    assert_equal_int(3, func_def_add(1, 2));
    assert_equal_int(6, func_def_factorial(3));
    assert_equal_int(543210, func_def_arg6(0, 1, 2, 3, 4, 5));
    assert_equal_int(6543210, func_def_arg7(0, 1, 2, 3, 4, 5, 6));
    assert_equal_int(76543210, func_def_arg8(0, 1, 2, 3, 4, 5, 6, 7));
    struct param_t1 s1 = {0, 1, 2}; assert_equal_long(210, func_def_struct1(s1));
    struct param_t2 s2 = {0, 1, 2}; assert_equal_long(210, func_def_struct2(s2));
    struct param_t3 s3 = {7, 8, 9}; assert_equal_long(9876543210, func_def_struct3(0, 1, 2, 3, 4, s3, 5, 6));
    struct param_t4 s4 = {0, 1, 2}; assert_equal_long(210, func_def_struct4(s4));
    struct param_t5 s5 = func_def_struct5(11, 22, 33); assert_equal_char(11, s5.m0); assert_equal_char(22, s5.m1); assert_equal_int(33, s5.m2);
    struct param_t6 s6 = func_def_struct6(11, 22); assert_equal_long(11, s6.m0); assert_equal_long(22, s6.m1);
    struct param_t7 s7 = func_def_struct7(11, 22, 33); assert_equal_long(11, s7.m0); assert_equal_long(22, s7.m1); assert_equal_char(33, s7.m2);
    struct param_t8 s8 = func_def_struct8(1, 2, 3, 10, 20, 30, 100); assert_equal_long(111, s8.m0); assert_equal_long(22, s8.m1); assert_equal_long(33, s8.m2);
    int a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; assert_equal_int(45, func_def_arg_array(a));
    assert_equal_int(0, func_def_arg_func(func_def_return0));
    assert_equal_int(6, func_def_factorial(3));    
    assert_equal_int(21, func_def_fibonacci(8));
    assert_equal_int(10, func_def_variadic(5, 0, 1, 2, 3, 4)); assert_equal_int(15, func_def_variadic(6, 0, 1, 2, 3, 4, 5)); assert_equal_int(21, func_def_variadic(7, 0, 1, 2, 3, 4, 5, 6));
    assert_equal_int(1, func_def_inline(1));
#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    assert_equal_float(1.0f, func_def_return_float(1.0f));
    assert_equal_double(2.0, func_def_return_double(2.0));
#endif /* INCLUDE_FLOATING_POINT_TYPE */

    return 0;
}


/*
address operator and dereference operator
*/
int func_def_argp1(int *x)
{
    return *x;
}
int func_def_argp2(int **x)
{
    return **x;
}
int func_def_argp3(int ***x)
{
    return ***x;
}
int *func_def_returnp1(int *x)
{
    return x;
}
int **func_def_returnp2(int **x)
{
    return x;
}
int test_address_dereference()
{
    int x; int *y; int **z;
    x = 1; y = &x; assert_equal_int(3, *y + 2);
    x = 1; y = &x; *y = 3; assert_equal_int(3 , x);
    x = 1; y = &x; z = &y; **z = 3; assert_equal_int(3, x);

    int a0; int *a1; int **a2; int b0; int *b1; int **b2;
    a0 = 3; b0 = func_def_argp1(&a0); assert_equal_int(3, b0);
    a0 = 3; a1 = &a0; b0 = func_def_argp2(&a1); assert_equal_int(3, b0);
    a0 = 3; a1 = &a0; a2 = &a1; b0 = func_def_argp3(&a2); assert_equal_int(3, b0);
    a0 = 3; b1 = func_def_returnp1(&a0); assert_equal_int(3, *b1);
    a0 = 3; a1 = &a0; b2 = func_def_returnp2(&a1); assert_equal_int(3, **b2);

    return 0;
}


/*
sizeof operator
*/
int test_sizeof()
{
    assert_equal_int(4, sizeof(1));
    assert_equal_int(8, sizeof sizeof(1));

    char c; char *pc;
    assert_equal_int(1, sizeof(char));
    assert_equal_int(8, sizeof(char *));
    assert_equal_int(8, sizeof(char **));
    assert_equal_int(1, sizeof(c));
    assert_equal_int(8, sizeof(pc));
    assert_equal_int(8, sizeof(&c));
    assert_equal_int(1, sizeof(*pc));
    assert_equal_int(4, sizeof(c + 1));
    assert_equal_int(8, sizeof(pc + 1));

    short s; short *ps;
    assert_equal_int(2, sizeof(short));
    assert_equal_int(8, sizeof(short *));
    assert_equal_int(8, sizeof(short **));
    assert_equal_int(2, sizeof(s));
    assert_equal_int(8, sizeof(ps));
    assert_equal_int(8, sizeof(&s));
    assert_equal_int(2, sizeof(*ps));
    assert_equal_int(4, sizeof(s + 1));
    assert_equal_int(8, sizeof(ps + 1));

    int i; int *pi;
    assert_equal_int(4, sizeof(int));
    assert_equal_int(8, sizeof(int *));
    assert_equal_int(8, sizeof(int **));
    assert_equal_int(4, sizeof(i));
    assert_equal_int(8, sizeof(pi));
    assert_equal_int(8, sizeof(&i));
    assert_equal_int(4, sizeof(*pi));
    assert_equal_int(4, sizeof(i + 1));
    assert_equal_int(8, sizeof(pi + 1));

    long l; long *pl;
    assert_equal_int(8, sizeof(long));
    assert_equal_int(8, sizeof(long *));
    assert_equal_int(8, sizeof(long **));
    assert_equal_int(8, sizeof(l));
    assert_equal_int(8, sizeof(pl));
    assert_equal_int(8, sizeof(&l));
    assert_equal_int(8, sizeof(*pl));
    assert_equal_int(8, sizeof(l + 1));
    assert_equal_int(8, sizeof(pl + 1));

    struct {int m1; int m2;} st1; assert_equal_int(8, sizeof(st1));
    struct {char m1; int m2; short m3;} st2; assert_equal_int(12, sizeof(st2));
    struct {char m1; struct {char mm1; int mm2; short mm3;} m2; short m3;} st3; assert_equal_int(20, sizeof(st3));

    union {int m1; int m2;} un1; assert_equal_int(4, sizeof(un1));
    union {char m1; int m2; short m3;} un2; assert_equal_int(4, sizeof(un2));
    union {char m1; union {char mm1; int mm2; short mm3;} m2; short m3;} un3; assert_equal_int(4, sizeof(un3));

    assert_equal_int(12, sizeof(const int [3]));
    assert_equal_int(24, sizeof(int const [3][2]));
    assert_equal_int(48, sizeof(int const *[3][2][1]));
    assert_equal_int(48, sizeof(int const * const[3][2][1]));
    assert_equal_int(8, sizeof(int (*)[3][2][1]));
    assert_equal_int(32, sizeof(int (*[4])[3][2][1]));
    assert_equal_int(32, sizeof(int (**[4])[3][2][1]));
    assert_equal_int(8, sizeof(int (*(*)[4])[3][2][1]));
    assert_equal_int(8, sizeof(int (*)(void)));
    assert_equal_int(24, sizeof(int (*[3])(int, int param, char *, volatile long ** const *)));
    typedef void (*sig_handler)(int); assert_equal_int(8, sizeof(sig_handler));

    return 0;
}


/*
_Alignof operator
*/
int test_alignof()
{
    assert_equal_int(1, _Alignof(char));
    assert_equal_int(1, _Alignof(char [5]));
    assert_equal_int(8, _Alignof(char *));
    assert_equal_int(8, _Alignof(char *[5]));
    assert_equal_int(8, _Alignof(char **));

    assert_equal_int(2, _Alignof(short));
    assert_equal_int(2, _Alignof(short [5]));
    assert_equal_int(8, _Alignof(short *));
    assert_equal_int(8, _Alignof(short *[5]));
    assert_equal_int(8, _Alignof(short **));


    assert_equal_int(4, _Alignof(int));
    assert_equal_int(4, _Alignof(int [5]));
    assert_equal_int(8, _Alignof(int *));
    assert_equal_int(8, _Alignof(int *[5]));
    assert_equal_int(8, _Alignof(int **));

    assert_equal_int(8, _Alignof(long));
    assert_equal_int(8, _Alignof(long [5]));
    assert_equal_int(8, _Alignof(long *));
    assert_equal_int(8, _Alignof(long *[5]));
    assert_equal_int(8, _Alignof(long **));

    typedef struct {int m1; int m2;} st1_t; assert_equal_int(4, _Alignof(st1_t));
    typedef struct {char m1; int m2; short m3;} st2_t; assert_equal_int(4, _Alignof(st2_t));
    typedef struct {char m1; struct {char mm1; int mm2; short mm3;} m2; short m3;} st3_t; assert_equal_int(4, _Alignof(st3_t));

    typedef union {int m1; int m2;} un1_t; assert_equal_int(4, _Alignof(un1_t));
    typedef union {char m1; int m2; short m3;} un2_t; assert_equal_int(4, _Alignof(un2_t));
    typedef union {char m1; union {char mm1; int mm2; short mm3;} m2; short m3;} un3_t; assert_equal_int(4, _Alignof(un3_t));

    assert_equal_int(4, _Alignof(const int [3]));
    assert_equal_int(4, _Alignof(int const [3][2]));
    assert_equal_int(8, _Alignof(int const *[3][2][1]));
    assert_equal_int(8, _Alignof(int const * const[3][2][1]));
    assert_equal_int(8, _Alignof(int (*)[3][2][1]));
    assert_equal_int(8, _Alignof(int (*[4])[3][2][1]));
    assert_equal_int(8, _Alignof(int (**[4])[3][2][1]));
    assert_equal_int(8, _Alignof(int (*(*)[4])[3][2][1]));
    assert_equal_int(8, _Alignof(int (*)(void)));
    assert_equal_int(8, _Alignof(int (*[3])(int, int param, char *, volatile long ** const *)));
    typedef void (*sig_handler)(int); assert_equal_int(8, _Alignof(sig_handler));

    return 0;
}


/*
pointer operation
*/
int test_pointer_operation()
{
    int *p; int *q; alloc4(&p, 1, 2, 4, 8);
    q = p + 2; assert_equal_int(4, *q);
    q = 2 + p; assert_equal_int(4, *q);
    q = p + 2; assert_equal_int(2, *(q - 1));
    assert_equal_int(2, q - p); assert_equal_int(-2, p - q);
    assert_equal_int(1, p < q); assert_equal_int(1, p <= q); assert_equal_int(0, p > q); assert_equal_int(0, p >= q);

    return 0;
}


/*
array
*/
int test_array()
{
    int a1[1]; int a2[2]; int a3[1 + 2]; int a4[2 << 1]; int a32[3][2]; int *p; int i; int j;
    *a1 = 2; assert_equal_int(2, *a1);
    *(a2 + 1) = 2; assert_equal_int(2, 2 * *(a2 + 1) - 2);
    *a2 = 1; *(a2 + 1) = 2; p = a2; assert_equal_int(3, *p + *(p + 1));
    for(i = 0; i < 4; i = i + 1){*(a4 + i) = i + 1;} assert_equal_int(10, *a4 + *(a4 + 1) + *(a4 + 2) + *(a4 + 3));
    for(i = 0; i < 4; i = i + 1){a4[i] = i + 1;} assert_equal_int(10, a4[0] + 1[a4] + a4[2] + 3[a4]);
    assert_equal_int(12, sizeof(a3));
    assert_equal_int(1, (a3 == (int *)&a3));
    assert_equal_int(1, ((int *)a3 == (int *)&a3));
    a32[0][0] = 0; assert_equal_int(0, a32[0][0]);
    a32[0][1] = 1; assert_equal_int(1, a32[0][1]);
    a32[0][2] = 2; assert_equal_int(2, a32[0][2]);
    a32[1][0] = 3; assert_equal_int(3, a32[1][0]);
    a32[1][1] = 4; assert_equal_int(4, a32[1][1]);
    a32[1][2] = 5; assert_equal_int(5, a32[1][2]);
    a32[2][1] = 5; assert_equal_int(5, a32[2][1]);
    assert_equal_int(24, sizeof(a32));
    assert_equal_int(8, sizeof(a32[0]));
    assert_equal_int(4, sizeof(a32[0][0]));
    for(i = 0; i < 3; i = i + 1){for(j = 0; j < 2; j = j + 1){a32[i][j] = 2 * i + j;}} assert_equal_int(15, a32[0][0] + a32[0][1] + a32[1][0] + a32[1][1] + a32[2][0] + a32[2][1]);

    return 0;
}


/*
global variable
*/
int gvar_int1; int gvar_int2; int *gvar_p1; int gvar_a4[4]; int gvar_a45[4][5];
int func_def_set_gvar_int1()
{
    gvar_int1 = 1;

    return gvar_int1;
}
int test_global_variable()
{
    int i;
    gvar_int1 = 1; assert_equal_int(1, gvar_int1);
    gvar_int2 = 2; assert_equal_int(3, func_def_set_gvar_int1() + gvar_int2);
    gvar_int1 = 3; gvar_p1 = &gvar_int1; assert_equal_int(3, *gvar_p1);
    for(i = 0; i < 4; i = i + 1){gvar_a4[i] = i;} assert_equal_int(6, gvar_a4[0] + gvar_a4[1] + gvar_a4[2] + gvar_a4[3]);
    assert_equal_int(80, sizeof gvar_a45);

    return 0;
}


/*
void type
*/
int gvar_int;
void func_def_arg_v(void)
{
    gvar_int *= 2;

    return;
}
int test_void()
{
    gvar_int = 123;
    func_def_arg_v();
    assert_equal_int(246, gvar_int);

    return 0;
}


/*
bool type
*/
_Bool func_def_equal(int a, int b)
{
    return a == b;
}
int test_bool()
{
    _Bool bf = 0; assert_equal_bool(0, bf);
    _Bool bt = 1; assert_equal_bool(1, bt);
    assert_equal_bool(1, func_def_equal(1, 1)); assert_equal_bool(0, func_def_equal(1, 2));

    return 0;
}


/*
char type
*/
int test_char()
{
    char c = -1; assert_equal_char(-1, c);
    signed char sc = -2; assert_equal_char(-2, sc);

    return 0;
}


/*
short type
*/
int test_short()
{
    short s = -1; assert_equal_short(-1, s);
    signed short ss = -2; assert_equal_short(-2, ss);
    short int si = -3; assert_equal_short(-3, si);
    signed short int ssi = -4; assert_equal_short(-4, ssi);

    return 0;
}


/*
int type
*/
int test_int()
{
    int i = -1; assert_equal_int(-1, i);
    signed s = -2; assert_equal_int(-2, s);
    signed int si = -3; assert_equal_int(-3, si);

    return 0;
}


/*
long type
*/
int test_long()
{
    long l = -1; assert_equal_long(-1, l);
    signed long sl = -2; assert_equal_long(-2, sl);
    long int li = -3; assert_equal_long(-3, li);
    signed long int sli = -4; assert_equal_long(-4, sli);

    return 0;
}


/*
long long type
*/
int test_longlong()
{
    long long ll = -1; assert_equal_long_long(-1, ll);
    signed long long sll = -2; assert_equal_long_long(-2, sll);
    long long int lli = -3; assert_equal_long_long(-3, lli);
    signed long long int slli = -4; assert_equal_long_long(-4, slli);

    return 0;
}


/*
atomic type
*/
int test_atomic()
{
    int i = 1; assert_equal_int(1, i);
    _Atomic(int) ai = 11; assert_equal_int(11, ai);
    _Atomic(int) *pai1 = &ai; assert_equal_int(11, *pai1);
    _Atomic int * pai2 = &ai; assert_equal_int(11, *pai2);
    int * _Atomic api = &i; assert_equal_int(1, *api);

    return 0;
}


/*
struct type
*/
int test_struct()
{
    struct st1_tag {int m1; int m2;} st1, *pst1;
    st1.m1 = 1; assert_equal_int(1, st1.m1);
    st1.m2 = 2; assert_equal_int(2, st1.m2);
    pst1 = &st1;
    pst1->m1 = 3; assert_equal_int(3, st1.m1);
    pst1->m2 = 4; assert_equal_int(4, st1.m2);
    struct st1_tag st1_copy = st1; assert_equal_int(3, st1_copy.m1); assert_equal_int(8, sizeof(struct st1_tag));

    struct {char m1; int m2; short m3;} st2, st2_copy;
    st2.m1 = 1; assert_equal_char(1, st2.m1);
    st2.m2 = 2; assert_equal_int(2, st2.m2);
    st2.m3 = 3; assert_equal_int(3, st2.m3);
    st2_copy = st2; assert_equal_char(1, st2_copy.m1); assert_equal_int(2, st2_copy.m2); assert_equal_short(3, st2_copy.m3);

    struct {char m1; struct {char mm1; int mm2; short mm3;} m2; short m3;} st3, st3_copy;
    st3.m1 = 1; assert_equal_char(1, st3.m1);
    st3.m2.mm1 = 11; assert_equal_char(11, st3.m2.mm1);
    st3.m2.mm2 = 22; assert_equal_int(22, st3.m2.mm2);
    st3.m2.mm3 = 33; assert_equal_short(33, st3.m2.mm3);
    st3.m3 = 3; assert_equal_char(3, st3.m3);
    st3_copy = st3; assert_equal_char(1, st3_copy.m1); assert_equal_char(11, st3_copy.m2.mm1); assert_equal_int(22, st3_copy.m2.mm2); assert_equal_short(33, st3_copy.m2.mm3); assert_equal_char(3, st3_copy.m3);

    struct {char m1, *m2; int *m3, m4, **m5;} st4;
    st4.m1 = 1; st4.m2 = &st4.m1; assert_equal_char(1, *st4.m2);
    st4.m4 = 2; st4.m3 = &st4.m4; st4.m5 = &st4.m3; assert_equal_int(2, *st4.m3); assert_equal_int(2, **st4.m5);

    struct {int a1[5];} st5, st5_copy;
    int i; for(i = 0; i < 5; i++) st5.a1[i] = i; st5_copy = st5; for(i = 0; i < 5; i++) assert_equal_int(i, st5_copy.a1[i]); assert_equal_int(20, sizeof(st5.a1));

    typedef struct st6_tag st6_type;
    struct st6_tag {st6_type *next; int val;} st6_1, st6_2;
    st6_1.val = 1; st6_1.next = &st6_2; st6_2.val = 2; assert_equal_int(1, st6_1.val); assert_equal_int(2, st6_1.next->val);

    struct {int m1; struct {int mm1; struct {int mmm1; int mmm2;};};} st7;
    st7.m1 = 1; assert_equal_int(1, st7.m1); st7.mm1 = 11; assert_equal_int(11, st7.mm1); st7.mmm1 = 111; assert_equal_int(111, st7.mmm1); st7.mmm2 = 222; assert_equal_int(222, st7.mmm2);

    struct {int m1:12, m2:12, m3:8; short m4:12, :0;} st8;
    st8.m1 = 1; assert_equal_int(1, st8.m1); st8.m2 = 2; assert_equal_int(2, st8.m2); st8.m3 = 3; assert_equal_int(3, st8.m3); st8.m4 = 4; assert_equal_int(4, st8.m4);

    return 0;
}


/*
union type
*/
int test_union()
{
    union {int m1; int m2;} un1, *pun1;
    un1.m1 = 1; assert_equal_int(1, un1.m1);
    un1.m2 = 2; assert_equal_int(2, un1.m2);
    pun1 = &un1;
    pun1->m1 = 3; assert_equal_int(3, un1.m1);
    pun1->m2 = 4; assert_equal_int(4, un1.m2);

    union {char m1; int m2; short m3;} un2;
    un2.m1 = 1; assert_equal_char(1, un2.m1);
    un2.m2 = 2; assert_equal_int(2, un2.m2);
    un2.m3 = 3; assert_equal_int(3, un2.m3);

    union {char m1; union {char mm1; int mm2; short mm3;} m2; short m3;} un3;
    un3.m1 = 1; assert_equal_char(1, un3.m1);
    un3.m2.mm1 = 11; assert_equal_char(11, un3.m2.mm1);
    un3.m2.mm2 = 22; assert_equal_int(22, un3.m2.mm2);
    un3.m2.mm3 = 33; assert_equal_short(33, un3.m2.mm3);
    un3.m3 = 3; assert_equal_char(3, un3.m3);

    union {struct {long mm1; long mm2;} m1; int m2[3];} un4, un4_copy;
    un4.m1.mm1 = 1; un4.m1.mm2 = 2; un4_copy = un4; assert_equal_long(1, un4_copy.m1.mm1); assert_equal_long(2, un4_copy.m1.mm2);
    int i; for(i = 0; i < 3; i++) un4.m2[i] = 10 * i; un4_copy = un4; for(i = 0; i < 3; i++) assert_equal_int(10 * i, un4_copy.m2[i]); assert_equal_int(12, sizeof(un4.m2));

    union un5_type {char m1; struct {short mm1; union {int mmm1; int mmm2;};};};
    union un5_type un5 = (union un5_type){.mmm1 = 1}; assert_equal_int(8, sizeof(union un5_type)); assert_equal_int(1, *(int *)((char *)&un5 + 4)); assert_equal_long(4, (char *)&un5.mmm1 - (char *)&un5); assert_equal_long(4, (char *)&un5.mmm2 - (char *)&un5);
    un5.m1 = 1; assert_equal_int(1, un5.m1); un5.mm1 = 11; assert_equal_int(11, un5.mm1); un5.mmm1 = 111; assert_equal_int(111, un5.mmm1); un5.mmm2 = 222; assert_equal_int(222, un5.mmm2);

    struct st6_type {char m1; union {short mm1; struct {int mmm1; int mmm2;};};};
    struct st6_type un6 = (struct st6_type){.mmm1 = 1}; assert_equal_int(12, sizeof(struct st6_type)); assert_equal_int(1, *(int *)((char *)&un6 + 4)); assert_equal_long(4, (char *)&un6.mmm1 - (char *)&un6); assert_equal_long(8, (char *)&un6.mmm2 - (char *)&un6);
    un6.m1 = 1; assert_equal_int(1, un6.m1); un6.mm1 = 11; assert_equal_int(11, un6.mm1); un6.mmm1 = 111; assert_equal_int(111, un6.mmm1); un6.mmm2 = 222; assert_equal_int(222, un6.mmm2);

    return 0;
}


/*
enumeration type
*/
int test_enum()
{
    enum {EN1, EN2} en1;
    en1 = EN2 * 2;
    assert_equal_int(0, EN1); assert_equal_int(1, EN2); assert_equal_int(2, en1);

    enum {EN3 = 11, EN4, EN5 = 11 * 2, EN6, } en2 = EN4;
    assert_equal_int(11, EN3); assert_equal_int(12, EN4); assert_equal_int(22, EN5); assert_equal_int(23, EN6); assert_equal_int(12, en2);

    return 0;
}


/*
string-literal
*/
int test_string_literal()
{
    char *s;
    s = "foo";
    assert_equal_char('f', s[0]);
    assert_equal_char('o', s[1]);
    assert_equal_char('o', s[2]);
    assert_equal_char('\0', s[3]);

    return 0;
}


/*
character constant
*/
int test_character_constant()
{
    assert_equal_char(111, 'o');
    assert_equal_char(48, '0');
    assert_equal_char(34, '\"'); assert_equal_char(34, '"');
    assert_equal_char(39, '\'');
    assert_equal_char(63, '\?'); assert_equal_char(63, '?');
    assert_equal_char(92, '\\');
    assert_equal_char(7, '\a');
    assert_equal_char(8, '\b');
    assert_equal_char(12, '\f');
    assert_equal_char(13, '\r');
    assert_equal_char(9, '\t');
    assert_equal_char(11, '\v');
    assert_equal_char(0, '\0');
    assert_equal_char(1, '\1');
    assert_equal_char(10, '\12');
    assert_equal_char(83, '\123');

    return 0;
}


/*
initializer
*/
char gvar_init_char = 1;
short gvar_init_short = 2;
int gvar_init_int = 3;
long gvar_init_long = 4;
int gvar_int_uninit1, gvar_int_init1 = 11, gvar_int_init2 = 12, gvar_int_uninit2;
int gvar_init_a1[3] = {10 * +1 + (1 - 2) / -1, (1 << 1) * ((8 >> 3 / 2) * 3 - !0), (0 < 1) * 11 + 22};
int gvar_init_a2[2][2] = {{11, 22}, {33, 44}};
int gvar_init_a3[][3] = {{11, 22, }, {44, }};
int gvar_init_a4[6] = {1, [2] = 3, 4, [4] = 5};
int gvar_init_a5[6] = {1, [2] = 3, 4, 5, [0] = 5, 6, 7};
int gvar_init_a6[3][2] = {{1, 2}, [2][1] = 6};
int gvar_init_a7[] = {1, 2, [3] = 4};
int gvar_init_a8[] = {1, 2, [0] = 4};
struct {int m1; int m2;} gvar_init_st1 = {111, 222};
struct {char m1; struct {int mm1; short mm2;} m2;} gvar_init_st2 = {111, {222, 333}};
struct {int m1; int m2; int m3;} gvar_init_st3 = {.m2 = 2, 3};
struct {int m1[3], m2;} gvar_init_st4[] = {[0].m1 = {1}, [1].m1[0] = 2};
union {char m1[8]; int m2[2]; long m3[1];} gvar_init_un1_char = {.m1 = {0x01, 0x02, [4]=0x11, 0x22}};
union {char m1[8]; int m2[2]; long m3[1];} gvar_init_un1_int = {.m2 = {0x03, }};
union {char m1[8]; int m2[2]; long m3[1];} gvar_init_un1_long = {.m3 = {0x04}};
char gvar_init_c1[] = "foo";
char *gvar_init_str = "foo";
int gvar_init_cond = 1 ? 2 : 1 / 0;
int gvar_init_generic1 = _Generic(gvar_init_generic1, char: 1, short:2, int: 4, long: 8);
int *gvar_init_generic2 = _Generic(gvar_init_generic2, char *: &gvar_init_char, short *: &gvar_init_short, int *: &gvar_init_int, long *: &gvar_init_long);
typedef struct {char m1; struct {int mm1; short mm2; int *mm3;} m2;} gvar_init_st_type;
gvar_init_st_type gvar_init_st = {111, {222, 333, &gvar_init_int}};
int *gvar_init_p1 = &gvar_init_int + 3;
int *gvar_init_p2 = &gvar_init_a1[3] - 3;
int *gvar_init_p3 = 1 + 2 + gvar_init_a1 - 2 - 1;
int (*gvar_init_p4)(const char *) = put_title;
int (*gvar_init_p5)(const char *) = put_title + 3;
char *gvar_init_p6 = &gvar_init_st.m1;
int *gvar_init_p7 = &gvar_init_st.m2.mm1;
short *gvar_init_p8 = &*&gvar_init_st.m2.mm2;
gvar_init_st_type gvar_init_compound = (gvar_init_st_type){.m2.mm1 = 2, .m2.mm2 = 3, .m2.mm3 = &gvar_init_int, .m1 = 1};
int test_initializer()
{
    char c = 1; assert_equal_char(1, c);
    short s = 2; assert_equal_short(2, s);
    int i = 3; assert_equal_int(3, i);
    long l = 4; assert_equal_long(4, l);

    int *p; alloc4(&p, 1, 2, 4, 8); int *q = &p[1]; assert_equal_int(4, q[1]);
    char *ps = "foo"; assert_equal_char(111, ps[1]);

    int i1 = 1, i2 = 2, *p1 = &p[1]; assert_equal_int(1, i1); assert_equal_int(i2, 2); assert_equal_int(4, p1[1]);

    assert_equal_int(1, gvar_init_char);
    assert_equal_int(2, gvar_init_short);
    assert_equal_int(3, gvar_init_int);
    assert_equal_int(4, gvar_init_long);

    gvar_int_uninit1 = 1, gvar_int_uninit2 = 2;
    assert_equal_int(1, gvar_int_uninit1);
    assert_equal_int(2, gvar_int_uninit2);
    assert_equal_int(11, gvar_int_init1);
    assert_equal_int(12, gvar_int_init2);

    assert_equal_int(11, gvar_init_a1[0]); assert_equal_int(22, gvar_init_a1[1]); assert_equal_int(33, gvar_init_a1[2]);
    assert_equal_int(11, gvar_init_a2[0][0]); assert_equal_int(22, gvar_init_a2[0][1]); assert_equal_int(33, gvar_init_a2[1][0]); assert_equal_int(44, gvar_init_a2[1][1]);
    assert_equal_int(11, gvar_init_a3[0][0]); assert_equal_int(22, gvar_init_a3[0][1]); assert_equal_int(0, gvar_init_a3[0][2]); assert_equal_int(44, gvar_init_a3[1][0]); assert_equal_int(0, gvar_init_a3[1][1]); assert_equal_int(0, gvar_init_a3[1][2]);
    assert_equal_int(1, gvar_init_a4[0]); assert_equal_int(0, gvar_init_a4[1]); assert_equal_int(3, gvar_init_a4[2]); assert_equal_int(4, gvar_init_a4[3]); assert_equal_int(5, gvar_init_a4[4]); assert_equal_int(0, gvar_init_a4[5]);
    assert_equal_int(5, gvar_init_a5[0]); assert_equal_int(6, gvar_init_a5[1]); assert_equal_int(7, gvar_init_a5[2]); assert_equal_int(4, gvar_init_a5[3]); assert_equal_int(5, gvar_init_a5[4]); assert_equal_int(0, gvar_init_a5[5]);
    assert_equal_int(1, gvar_init_a6[0][0]); assert_equal_int(2, gvar_init_a6[0][1]); assert_equal_int(0, gvar_init_a6[1][0]); assert_equal_int(0, gvar_init_a6[1][1]); assert_equal_int(0, gvar_init_a6[2][0]); assert_equal_int(6, gvar_init_a6[2][1]);
    assert_equal_int(1, gvar_init_a7[0]); assert_equal_int(2, gvar_init_a7[1]); assert_equal_int(0, gvar_init_a7[2]); assert_equal_int(4, gvar_init_a7[3]); assert_equal_int(4, sizeof(gvar_init_a7) / sizeof(gvar_init_a7[0]));
    assert_equal_int(4, gvar_init_a8[0]); assert_equal_int(2, gvar_init_a8[1]); assert_equal_int(2, sizeof(gvar_init_a8) / sizeof(gvar_init_a8[0]));
    assert_equal_int(111, gvar_init_st1.m1); assert_equal_int(222, gvar_init_st1.m2);
    assert_equal_int(111, gvar_init_st2.m1); assert_equal_int(222, gvar_init_st2.m2.mm1); assert_equal_int(333, gvar_init_st2.m2.mm2);
    assert_equal_int(0, gvar_init_st3.m1); assert_equal_int(2, gvar_init_st3.m2); assert_equal_int(3, gvar_init_st3.m3);
    assert_equal_int(1, gvar_init_st4[0].m1[0]); assert_equal_int(0, gvar_init_st4[0].m1[1]); assert_equal_int(0, gvar_init_st4[0].m1[2]); assert_equal_int(2, gvar_init_st4[1].m1[0]); assert_equal_int(0, gvar_init_st4[1].m1[1]); assert_equal_int(0, gvar_init_st4[1].m1[2]); assert_equal_int(32, sizeof(gvar_init_st4));
    assert_equal_char(0x01, gvar_init_un1_char.m1[0]); assert_equal_char(0x02, gvar_init_un1_char.m1[1]); assert_equal_char(0x00, gvar_init_un1_char.m1[2]); assert_equal_char(0x00, gvar_init_un1_char.m1[3]); assert_equal_char(0x11, gvar_init_un1_char.m1[4]); assert_equal_char(0x22, gvar_init_un1_char.m1[5]); assert_equal_char(0x00, gvar_init_un1_char.m1[6]); assert_equal_char(0x00, gvar_init_un1_char.m1[7]);
    assert_equal_int(0x03, gvar_init_un1_int.m2[0]); assert_equal_int(0x00, gvar_init_un1_int.m2[1]);
    assert_equal_long(0x04, gvar_init_un1_long.m3[0]);
    assert_equal_char(111, gvar_init_c1[1]);
    assert_equal_char(111, gvar_init_str[1]);

    assert_equal_int(2, gvar_init_cond);
    assert_equal_int(4, gvar_init_generic1);
    assert_equal_pointer(&gvar_init_int, gvar_init_generic2);
    assert_equal_pointer(&gvar_init_int + 3, gvar_init_p1);
    assert_equal_pointer(&gvar_init_a1[0], gvar_init_p2);
    assert_equal_pointer(&gvar_init_a1[0], gvar_init_p3);
    assert_equal_pointer(&put_title, gvar_init_p4);
    assert_equal_pointer(&put_title + 3, gvar_init_p5);
    assert_equal_pointer(&gvar_init_st.m1, gvar_init_p6);
    assert_equal_pointer(&gvar_init_st.m2.mm1, gvar_init_p7);
    assert_equal_pointer(&gvar_init_st.m2.mm2, gvar_init_p8);
    assert_equal_char(1, gvar_init_compound.m1); assert_equal_int(2, gvar_init_compound.m2.mm1); assert_equal_short(3, gvar_init_compound.m2.mm2); assert_equal_pointer(&gvar_init_int, gvar_init_compound.m2.mm3);

    int a0 = {1}; assert_equal_int(1, a0);
    int a1[3] = {1, 2, 3}; assert_equal_int(1, a1[0]); assert_equal_int(2, a1[1]); assert_equal_int(3, a1[2]);
    int a2[2][3] = {{1, 2, 3}, {4, 5, 6}}; assert_equal_int(1, a2[0][0]); assert_equal_int(2, a2[0][1]); assert_equal_int(3, a2[0][2]); assert_equal_int(4, a2[1][0]); assert_equal_int(5, a2[1][1]); assert_equal_int(6, a2[1][2]);
    int a3[2][2][2] = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}}; int idx1, idx2, idx3; for(idx1 = 0; idx1 < 2; idx1++) for(idx2 = 0; idx2 < 2; idx2++) for(idx3 = 0; idx3 < 2; idx3++)  assert_equal_int(4 * idx1 + 2 * idx2 + 1 * idx3 + 1, a3[idx1][idx2][idx3]);
    int a4[3] = {1, 2, }; assert_equal_int(1, a4[0]); assert_equal_int(2, a4[1]); assert_equal_int(0, a4[2]);
    int a5[2][3] = {{1}, {4, 5}}; assert_equal_int(1, a5[0][0]); assert_equal_int(0, a5[0][1]); assert_equal_int(0, a5[0][2]); assert_equal_int(4, a5[1][0]); assert_equal_int(5, a5[1][1]); assert_equal_int(0, a5[1][2]);
    int a6[] = {1, 2, 3}; assert_equal_int(1, a6[0]); assert_equal_int(2, a6[1]); assert_equal_int(3, a6[2]); assert_equal_int(3, sizeof(a6) / sizeof(a6[0]));
    int a7[][3] = {{1, 2, }, {4, 5, }}; assert_equal_int(1, a7[0][0]); assert_equal_int(2, a7[0][1]); assert_equal_int(0, a7[0][2]); assert_equal_int(4, a7[1][0]); assert_equal_int(5, a7[1][1]); assert_equal_int(0, a7[1][2]); assert_equal_int(2, sizeof(a7) / sizeof(a7[0]));
    int a8[6] = {1, [2] = 3, 4, [4] = 5}; assert_equal_int(1, a8[0]); assert_equal_int(0, a8[1]); assert_equal_int(3, a8[2]); assert_equal_int(4, a8[3]); assert_equal_int(5, a8[4]); assert_equal_int(0, a8[5]);
    int a9[6] = {1, [2] = 3, 4, 5, [0] = 5, 6, 7}; assert_equal_int(5, a9[0]); assert_equal_int(6, a9[1]); assert_equal_int(7, a9[2]); assert_equal_int(4, a9[3]); assert_equal_int(5, a9[4]); assert_equal_int(0, a9[5]);
    int a10[3][2] = {{1, 2}, [2][1] = 6}; assert_equal_int(1, a10[0][0]); assert_equal_int(2, a10[0][1]); assert_equal_int(0, a10[1][0]); assert_equal_int(0, a10[1][1]); assert_equal_int(0, a10[2][0]); assert_equal_int(6, a10[2][1]);
    int a11[] = {1, 2, [3] = 4}; assert_equal_int(1, a11[0]); assert_equal_int(2, a11[1]); assert_equal_int(0, a11[2]); assert_equal_int(4, a11[3]); assert_equal_int(4, sizeof(a11) / sizeof(a11[0]));
    int a12[] = {1, 2, [0] = 4}; assert_equal_int(4, a12[0]); assert_equal_int(2, a12[1]); assert_equal_int(2, sizeof(a12) / sizeof(a12[0]));
    char c1[5] = "foo"; assert_equal_char(102, c1[0]); assert_equal_char(111, c1[1]); assert_equal_char(111, c1[2]); assert_equal_char(0, c1[3]); assert_equal_char(0, c1[4]); assert_equal_int(5, sizeof(c1));
    char c2[] = "foo"; assert_equal_char(102, c2[0]); assert_equal_char(111, c2[1]); assert_equal_char(111, c2[2]); assert_equal_char(0, c2[3]); assert_equal_int(4, sizeof(c2));
    const char *c3[] = {"foo", "f"}; assert_equal_char(102, c3[0][0]); assert_equal_char(111, c3[0][1]); assert_equal_char(111, c3[0][2]); assert_equal_char(0, c3[0][3]); assert_equal_char(102, c3[1][0]); assert_equal_char(0, c3[1][1]); 

    typedef struct {int m1; int m2;} st_type;
    st_type st1 = {1, 2}; assert_equal_int(1, st1.m1); assert_equal_int(2, st1.m2);
    st_type st2[3] = {{1, 2}, {3, 4}, {5, 6}}; assert_equal_int(1, st2[0].m1); assert_equal_int(2, st2[0].m2); assert_equal_int(3, st2[1].m1); assert_equal_int(4, st2[1].m2); assert_equal_int(5, st2[2].m1); assert_equal_int(6, st2[2].m2);
    st_type st3[3] = {{1}, st2[1], }; assert_equal_int(1, st3[0].m1); assert_equal_int(0, st3[0].m2); assert_equal_int(3, st3[1].m1); assert_equal_int(4, st3[1].m2); assert_equal_int(0, st3[2].m1); assert_equal_int(0, st3[2].m2);
    struct {int m1; int m2; int m3;} st4 = {.m2 = 2, 3}; assert_equal_int(0, st4.m1); assert_equal_int(2, st4.m2); assert_equal_int(3, st4.m3);
    struct {int m1[3], m2;} st5[] = {[0].m1 = {1}, [1].m1[0] = 2}; assert_equal_int(1, st5[0].m1[0]); assert_equal_int(0, st5[0].m1[1]); assert_equal_int(0, st5[0].m1[2]); assert_equal_int(2, st5[1].m1[0]); assert_equal_int(0, st5[1].m1[1]); assert_equal_int(0, st5[1].m1[2]);
    typedef union {int m1; int m2;} un_type;
    un_type un1 = {1}; assert_equal_int(1, un1.m1);
    typedef union {char m1[8]; int m2[2]; long m3[1];} un_type2;
    un_type2 un2_char = {.m1 = {0x01, 0x02, [4]=0x11, 0x22}}; assert_equal_char(0x01, un2_char.m1[0]); assert_equal_char(0x02, un2_char.m1[1]); assert_equal_char(0x00, un2_char.m1[2]); assert_equal_char(0x00, un2_char.m1[3]); assert_equal_char(0x11, un2_char.m1[4]); assert_equal_char(0x22, un2_char.m1[5]); assert_equal_char(0x00, un2_char.m1[6]); assert_equal_char(0x00, un2_char.m1[7]);
    un_type2 un2_int = {.m2 = {0x03, }}; assert_equal_int(0x03, un2_int.m2[0]); assert_equal_int(0x00, un2_int.m2[1]);
    un_type2 un2_long = {.m3 = {0x04}}; assert_equal_long(0x04, un2_long.m3[0]);

    return 0;
}


/*
scope of variables and tags
*/
int test_scope_var = 1;
struct test_scope_tag {int s1;};
int test_scope(void)
{
    assert_equal_int(1, test_scope_var);

    int test_scope_var = 2;
    assert_equal_int(2, test_scope_var);

    {
        int test_scope_var = 3;
        assert_equal_int(3, test_scope_var);
    }

    assert_equal_int(2, test_scope_var);
    struct test_scope_tag tag_var;
    tag_var.s1 = 11; assert_equal_int(11, tag_var.s1);

    {
        struct test_scope_tag {int s1;};
        tag_var.s1 = 22; assert_equal_int(22, tag_var.s1);

        struct test_scope_tag tag_var;
        tag_var.s1 = 33; assert_equal_int(33, tag_var.s1);
    }

    assert_equal_int(22, tag_var.s1);

    {
        struct test_scope_tag;
        struct test_scope_tag {int s1;} tag_var;
        tag_var.s1 = 44; assert_equal_int(44, tag_var.s1);
    }

    {
        union test_scope_tag {int u1;} tag_var;
        tag_var.u1 = -11; assert_equal_int(-11, tag_var.u1);
    }

    {
        enum test_scope_tag {EN1 = 100, EN2};
        enum test_scope_tag tag_var = EN1; assert_equal_int(100, tag_var);
        {
            enum test_scope_tag tag_var = EN2; assert_equal_int(101, tag_var);

            enum test_scope_tag {EN1 = 1000, EN2};
            tag_var = EN2; assert_equal_int(1001, tag_var);
        }
        assert_equal_int(100, tag_var);
    }

    assert_equal_int(22, tag_var.s1);

    return 0;
}


/*
storage class specifier
*/
typedef int int_type;
static int static_int;
int func_def_static_var0(void)
{
    extern int static_int;

    static_int++;

    return static_int;
}
int func_def_static_var1(void)
{
    static int static_int = 10;

    static_int++;

    return static_int;
}
int func_def_static_var2(void)
{
    static int static_int = 100;

    static_int++;

    return static_int;
}
static int static_int = 1000;
extern int static_int;
static int static_int;
_Thread_local int thread_local_extern_int;
int test_storage_class(void)
{
    int_type t = 0; assert_equal_int(0, t);
    static int s = 2; assert_equal_int(2, s);
    auto int a = 3; assert_equal_int(3, a);
    register int r = 4; assert_equal_int(4, r);

    typedef char type, *ptr_type;
    type c1 = 1; assert_equal_char(1, c1);
    ptr_type pc1 = &c1; assert_equal_char(1, *pc1);

    {
        type c2 = 2; assert_equal_char(2, c2);

        typedef short type;
        type s = -1; assert_equal_short(-1, s);
        assert_equal_int(2, sizeof(type));
    }

    assert_equal_int(1, sizeof(type));

    assert_equal_int(1001, func_def_static_var0());
    assert_equal_int(1002, func_def_static_var0());
    assert_equal_int(1002, static_int);

    assert_equal_int(11, func_def_static_var1());
    assert_equal_int(12, func_def_static_var1());
    assert_equal_int(1002, static_int);

    assert_equal_int(101, func_def_static_var2());
    assert_equal_int(102, func_def_static_var2());
    assert_equal_int(1002, static_int);

    {
        static int static_int = 1;
        assert_equal_int(1, static_int);
    }
    assert_equal_int(1002, static_int);

    {
        extern int static_int;
        static_int = 0;
    }
    assert_equal_int(0, static_int);

    _Thread_local extern int thread_local_extern_int;
    _Thread_local static int thread_local_static_int;
    thread_local_extern_int = 1; assert_equal_int(1, thread_local_extern_int);
    thread_local_static_int = 2; assert_equal_int(2, thread_local_static_int);

    return 0;
}


/*
type qualifier
*/
int test_type_qualifier(void)
{
    typedef const int const_int;

    const int a = 1; assert_equal_int(1, a);
    volatile int b = 2; assert_equal_int(2, b);
    const volatile int c = 3; assert_equal_int(3, c);
    const int volatile unsigned d = 4; assert_equal_int(4, d);
    const_int *e = &a; assert_equal_int(1, *e);
    const int * const f = &a; assert_equal_int(1, *f);
    const int * const * restrict volatile g = &f; assert_equal_int(1, **g);

    return 0;
}


/*
alignment specifier
*/
char _Alignas(0) gvar_aligned_char0 = 0;
char _Alignas(1) gvar_aligned_char1 = 1;
char _Alignas(2) gvar_aligned_char2 = 2;
char _Alignas(4) gvar_aligned_char4 = 4;
_Alignas(1) const _Alignas(8) _Alignas(4) char gvar_aligned_char8 = 8;
char _Alignas(16) gvar_aligned_char16 = 16;
int test_alignas(void)
{
    assert_equal_char(0, gvar_aligned_char0);
    assert_equal_char(1, gvar_aligned_char1);
    assert_equal_char(2, gvar_aligned_char2);
    assert_equal_char(4, gvar_aligned_char4);
    assert_equal_char(8, gvar_aligned_char8);
    assert_equal_char(16, gvar_aligned_char16);

    int _Alignas(int) ia_int = 0; assert_equal_int(0, ia_int);
    _Alignas(sizeof(char) * 4) int ia4 = 4; assert_equal_int(4, ia4);
    _Alignas(1) const _Alignas(8) _Alignas(4) int ia8 = 8; assert_equal_int(8, ia8);
    _Alignas(64) int *pia64 = &ia4; assert_equal_int(4, *pia64);

    return 0;
}


/*
static assertion
*/
int test_static_assertion(void)
{
    _Static_assert(0 + 1, "failed at test_static_assertion()");

    return 0;
}


/*
conversion
*/
int test_conversion(void)
{
    char c = 123; assert_equal_int(4, sizeof(c == 0)); assert_equal_int(1, c == 123);
    short s = 123; assert_equal_int(4, sizeof(s == 0)); assert_equal_int(1, s == 123);
    int i = 123; assert_equal_int(4, sizeof(i == 0)); assert_equal_int(1, i == 123);
    long l = 123; assert_equal_int(4, sizeof(l == 0)); assert_equal_int(1, l == 123);

    char c_arr[] = {1, 2, 3}; assert_equal_int(4, sizeof(c_arr[0] == 0)); assert_equal_int(1, c_arr[0] == 1); assert_equal_int(1, c_arr[1] == 2); assert_equal_int(1, c_arr[2] == 3);
    short s_arr[] = {11, 22, 33}; assert_equal_int(4, sizeof(s_arr[0] == 0)); assert_equal_int(1, s_arr[0] == 11); assert_equal_int(1, s_arr[1] == 22); assert_equal_int(1, s_arr[2] == 33);
    int i_arr[] = {111, 222, 333}; assert_equal_int(4, sizeof(i_arr[0] == 0)); assert_equal_int(1, i_arr[0] == 111); assert_equal_int(1, i_arr[1] == 222); assert_equal_int(1, i_arr[2] == 333);
    long l_arr[] = {1111, 2222, 3333}; assert_equal_int(4, sizeof(l_arr[0] == 0)); assert_equal_int(1, l_arr[0] == 1111); assert_equal_int(1, l_arr[1] == 2222); assert_equal_int(1, l_arr[2] == 3333);

    return 0;
}
#endif /* ENABLE_TEST_CASE(TEST_OTHERS) */


/*
main function of test code
*/
int main()
{
#if ENABLE_TEST_CASE(TEST_CONSTANTS)
    test_constants();
#endif /* ENABLE_TEST_CASE(TEST_CONSTANTS) */
#if ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS)
    test_additive_operators();
#endif /* ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS) */
#if ENABLE_TEST_CASE(TEST_OTHERS)
    test_multiplicative();
    test_shift();
    test_bitwise_and();
    test_bitwise_xor();
    test_bitwise_or();
    test_logical_and();
    test_logical_or();
    test_conditional();
    test_generic_selection();
    test_compound_literal();
    test_unary();
    test_cast();
    test_comparision();
    test_local_variable();
    test_assignment();
    test_comma();
    test_goto_statement();
    test_if_statement();
    test_switch_statement();
    test_while_statement();
    test_do_statement();
    test_for_statement();
    test_function_call();
    test_function_definition();
    test_address_dereference();
    test_sizeof();
    test_alignof();
    test_pointer_operation();
    test_array();
    test_global_variable();
    test_void();
    test_bool();
    test_char();
    test_short();
    test_int();
    test_long();
    test_longlong();
    test_atomic();
    test_struct();
    test_union();
    test_enum();
    test_string_literal();
    test_character_constant();
    test_initializer();
    test_scope();
    test_storage_class();
    test_type_qualifier();
    test_alignas();
    test_static_assertion();
    test_conversion();
#endif /* ENABLE_TEST_CASE(TEST_OTHERS) */

    return 0;
}
