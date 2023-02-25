#include "test_code.h"

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

    assert_size_of_expression(4, 2147483646); assert_constant_int_max_minus1(2147483646);
    assert_size_of_expression(4, 2147483647); assert_constant_int_max(2147483647);
    assert_size_of_expression(8, 2147483648); assert_constant_int_max_plus1(2147483648);
    assert_size_of_expression(8, 9223372036854775806); assert_constant_long_max_minus1(9223372036854775806);
    assert_size_of_expression(8, 9223372036854775807); assert_constant_long_max(9223372036854775807);

    assert_size_of_expression(4, 4294967294U); assert_constant_unsigned_int_max_minus1(4294967294U);
    assert_size_of_expression(4, 4294967295U); assert_constant_unsigned_int_max(4294967295U);
    assert_size_of_expression(8, 4294967296U); assert_constant_unsigned_int_max_plus1(4294967296U);
    assert_size_of_expression(8, 18446744073709551614U); assert_constant_unsigned_long_max_minus1(18446744073709551614U);
    assert_size_of_expression(8, 18446744073709551615U); assert_constant_unsigned_long_max(18446744073709551615U);

    assert_size_of_expression(8, 9223372036854775806L); assert_constant_long_max_minus1(9223372036854775806L);
    assert_size_of_expression(8, 9223372036854775807L); assert_constant_long_max(9223372036854775807L);

    assert_size_of_expression(8, 18446744073709551614UL); assert_constant_unsigned_long_max_minus1(18446744073709551614UL);
    assert_size_of_expression(8, 18446744073709551615UL); assert_constant_unsigned_long_max(18446744073709551615UL);

    assert_size_of_expression(8, 9223372036854775806LL); assert_constant_long_max_minus1(9223372036854775806LL);
    assert_size_of_expression(8, 9223372036854775807LL); assert_constant_long_max(9223372036854775807LL);

    assert_size_of_expression(8, 18446744073709551614ULL); assert_constant_unsigned_long_max_minus1(18446744073709551614ULL);
    assert_size_of_expression(8, 18446744073709551615ULL); assert_constant_unsigned_long_max(18446744073709551615ULL);

    assert_size_of_expression(4, 017777777776); assert_constant_int_max_minus1(017777777776);
    assert_size_of_expression(4, 017777777777); assert_constant_int_max(017777777777);
    assert_size_of_expression(4, 020000000000); assert_constant_int_max_plus1(020000000000);
    assert_size_of_expression(4, 037777777777); assert_constant_unsigned_int_max_minus1(037777777776);
    assert_size_of_expression(4, 037777777777); assert_constant_unsigned_int_max(037777777777);
    assert_size_of_expression(8, 040000000000); assert_constant_unsigned_int_max_plus1(040000000000);
    assert_size_of_expression(8, 0777777777777777777776); assert_constant_long_max_minus1(0777777777777777777776);
    assert_size_of_expression(8, 0777777777777777777777); assert_constant_long_max(0777777777777777777777);
    assert_size_of_expression(8, 01000000000000000000000); assert_constant_long_max_plus1(01000000000000000000000);
    assert_size_of_expression(8, 01777777777777777777776); assert_constant_unsigned_long_max_minus1(01777777777777777777776);
    assert_size_of_expression(8, 01777777777777777777777); assert_constant_unsigned_long_max(01777777777777777777777);

    assert_size_of_expression(4, 037777777777U); assert_constant_unsigned_int_max_minus1(037777777776U);
    assert_size_of_expression(4, 037777777777U); assert_constant_unsigned_int_max(037777777777U);
    assert_size_of_expression(8, 040000000000U); assert_constant_unsigned_int_max_plus1(040000000000U);
    assert_size_of_expression(8, 01777777777777777777776U); assert_constant_unsigned_long_max_minus1(01777777777777777777776U);
    assert_size_of_expression(8, 01777777777777777777777U); assert_constant_unsigned_long_max(01777777777777777777777U);

    assert_size_of_expression(8, 0777777777777777777776L); assert_constant_long_max_minus1(0777777777777777777776L);
    assert_size_of_expression(8, 0777777777777777777777L); assert_constant_long_max(0777777777777777777777L);
    assert_size_of_expression(8, 01000000000000000000000L); assert_constant_long_max_plus1(01000000000000000000000L);
    assert_size_of_expression(8, 01777777777777777777776L); assert_constant_unsigned_long_max_minus1(01777777777777777777776L);
    assert_size_of_expression(8, 01777777777777777777777L); assert_constant_unsigned_long_max(01777777777777777777777L);

    assert_size_of_expression(8, 01777777777777777777776UL); assert_constant_unsigned_long_max_minus1(01777777777777777777776UL);
    assert_size_of_expression(8, 01777777777777777777777UL); assert_constant_unsigned_long_max(01777777777777777777777UL);

    assert_size_of_expression(8, 01777777777777777777776ULL); assert_constant_unsigned_long_max_minus1(01777777777777777777776ULL);
    assert_size_of_expression(8, 01777777777777777777777ULL); assert_constant_unsigned_long_max(01777777777777777777777ULL);

    assert_size_of_expression(4, 0x7FFFFFFE); assert_constant_int_max_minus1(0x7FFFFFFE);
    assert_size_of_expression(4, 0x7FFFFFFF); assert_constant_int_max(0x7FFFFFFF);
    assert_size_of_expression(4, 0x80000000); assert_constant_int_max_plus1(0x80000000);
    assert_size_of_expression(4, 0xFFFFFFFF); assert_constant_unsigned_int_max_minus1(037777777776);
    assert_size_of_expression(4, 0xFFFFFFFF); assert_constant_unsigned_int_max(0xFFFFFFFF);
    assert_size_of_expression(8, 0x100000000); assert_constant_unsigned_int_max_plus1(0x100000000);
    assert_size_of_expression(8, 0x7FFFFFFFFFFFFFFE); assert_constant_long_max_minus1(0x7FFFFFFFFFFFFFFE);
    assert_size_of_expression(8, 0x7FFFFFFFFFFFFFFF); assert_constant_long_max(0x7FFFFFFFFFFFFFFF);
    assert_size_of_expression(8, 0x8000000000000000); assert_constant_long_max_plus1(0x8000000000000000);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFE); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFE);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFF); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFF);

    assert_size_of_expression(4, 0xFFFFFFFFU); assert_constant_unsigned_int_max_minus1(037777777776U);
    assert_size_of_expression(4, 0xFFFFFFFFU); assert_constant_unsigned_int_max(0xFFFFFFFFU);
    assert_size_of_expression(8, 0x100000000U); assert_constant_unsigned_int_max_plus1(0x100000000U);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFEU); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEU);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFFU); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFU);

    assert_size_of_expression(8, 0x7FFFFFFFFFFFFFFEL); assert_constant_long_max_minus1(0x7FFFFFFFFFFFFFFEL);
    assert_size_of_expression(8, 0x7FFFFFFFFFFFFFFFL); assert_constant_long_max(0x7FFFFFFFFFFFFFFFL);
    assert_size_of_expression(8, 0x8000000000000000L); assert_constant_long_max_plus1(0x8000000000000000L);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFEL); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEL);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFFL); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFL);

    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFEUL); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEUL);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFFUL); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFUL);

    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFEULL); assert_constant_unsigned_long_max_minus1(0xFFFFFFFFFFFFFFFEULL);
    assert_size_of_expression(8, 0xFFFFFFFFFFFFFFFFULL); assert_constant_unsigned_long_max(0xFFFFFFFFFFFFFFFFULL);

    assert_size_of_expression(4, 1);
    assert_size_of_expression(4, 1U); assert_size_of_expression(4, 1u);
    assert_size_of_expression(8, 1L); assert_size_of_expression(8, 1l);
    assert_size_of_expression(8, 1UL); assert_size_of_expression(8, 1Ul); assert_size_of_expression(8, 1uL); assert_size_of_expression(8, 1ul);
    assert_size_of_expression(8, 1LL); assert_size_of_expression(8, 1ll);
    assert_size_of_expression(8, 1ULL); assert_size_of_expression(8, 1Ull); assert_size_of_expression(8, 1uLL); assert_size_of_expression(8, 1ull);

    assert_size_of_expression(4, 01);
    assert_size_of_expression(4, 01U); assert_size_of_expression(4, 01u);
    assert_size_of_expression(8, 01L); assert_size_of_expression(8, 01l);
    assert_size_of_expression(8, 01UL); assert_size_of_expression(8, 01Ul); assert_size_of_expression(8, 01uL); assert_size_of_expression(8, 01ul);
    assert_size_of_expression(8, 01LL); assert_size_of_expression(8, 01ll);
    assert_size_of_expression(8, 01ULL); assert_size_of_expression(8, 01Ull); assert_size_of_expression(8, 01uLL); assert_size_of_expression(8, 01ull);

    assert_size_of_expression(4, 0x1);
    assert_size_of_expression(4, 0x1U); assert_size_of_expression(4, 0x1u);
    assert_size_of_expression(8, 0x1L); assert_size_of_expression(8, 0x1l);
    assert_size_of_expression(8, 0x1UL); assert_size_of_expression(8, 0x1Ul); assert_size_of_expression(8, 0x1uL); assert_size_of_expression(8, 0x1ul);
    assert_size_of_expression(8, 0x1LL); assert_size_of_expression(8, 0x1ll);
    assert_size_of_expression(8, 0x1ULL); assert_size_of_expression(8, 0x1Ull); assert_size_of_expression(8, 0x1uLL); assert_size_of_expression(8, 0x1ull);

#if (INCLUDE_FLOATING_POINT_TYPE == ENABLED)
    // 6.4.4.2 Floating constants
    float f = 1.0f; assert_equal_float(1.0f, f);
    double d = 2.0; assert_equal_double(2.0, d);
#endif /* INCLUDE_FLOATING_POINT_TYPE */
}
#endif /* ENABLE_TEST_CASE(TEST_CONSTANTS) */
