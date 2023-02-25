#ifndef TEST_CODE_H
#define TEST_CODE_H

#ifdef QCC_COMPILER
#include "../../utility/std_header.h"
#else
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif /* QCC_COMPILER */
#include "../common/check.h"
#include "../common/configuration.h"
#include "../stub/function_call.h"

#define put_title(title) printf("# %s\n", title)

#if ENABLE_TEST_CASE(TEST_CONSTANTS)
void test_constants();
#endif /* ENABLE_TEST_CASE(TEST_CONSTANTS) */
#if ENABLE_TEST_CASE(TEST_CAST_OPERATORS)
void test_cast_operators();
#endif /* ENABLE_TEST_CASE(TEST_CAST_OPERATORS) */
#if ENABLE_TEST_CASE(TEST_MULTIPLICATIVE_OPERATORS)
void test_multiplicative_operators();
#endif /* ENABLE_TEST_CASE(TEST_MULTIPLICATIVE_OPERATORS) */
#if ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS)
void test_additive_operators();
#endif /* ENABLE_TEST_CASE(TEST_ADDITIVE_OPERATORS) */
#if ENABLE_TEST_CASE(TEST_BITWISE_SHIFT_OPERATORS)
void test_bitwise_shift_operators();
#endif /* ENABLE_TEST_CASE(TEST_BITWISE_SHIFT_OPERATORS) */
#if ENABLE_TEST_CASE(TEST_RELATIONAL_OPERATORS)
void test_relational_operators();
#endif /* ENABLE_TEST_CASE(TEST_RELATIONAL_OPERATORS) */
#if ENABLE_TEST_CASE(TEST_EQUALITY_OPERATORS)
void test_equality_operators();
#endif /* ENABLE_TEST_CASE(TEST_EQUALITY_OPERATORS) */
#if ENABLE_TEST_CASE(TEST_BITWISE_AND_OPERATOR)
void test_bitwise_and_operator();
#endif /* ENABLE_TEST_CASE(TEST_BITWISE_AND_OPERATOR) */
#if ENABLE_TEST_CASE(TEST_EXCLUSIVE_OR_OPERATOR)
void test_exclusive_or_operator();
#endif /* ENABLE_TEST_CASE(TEST_EXCLUSIVE_OR_OPERATOR) */
#if ENABLE_TEST_CASE(TEST_INCLUSIVE_OR_OPERATOR)
void test_inclusive_or_operator();
#endif /* ENABLE_TEST_CASE(TEST_INCLUSIVE_OR_OPERATOR) */

#endif /* !TEST_CODE_H */
