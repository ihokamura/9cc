#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// indicator of feature enabled or disabled
#define ENABLED     (1U)
#define DISABLED    (0U)

// features
#define INCLUDE_FLOATING_POINT_TYPE    (ENABLED) // include tests on floating-point types
#ifdef TEST_ALL
#undef TEST_ALL
#define TEST_ALL    (ENABLED)
#endif /* TEST_ALL */
#define ENABLE_TEST_CASE(TEST_CASE_NAME)    (TEST_CASE_NAME || (TEST_ALL == ENABLED))

#endif /* !CONFIGURATION_H */
