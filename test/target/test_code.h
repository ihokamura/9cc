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

#endif /* !TEST_CODE_H */
