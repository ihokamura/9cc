#!/bin/bash

COMPILER_OLD=$1
COMPILER_NEW=$2

SOURCE=source
WORKSPACE=self
STD_HEADER=util/std_header.h

# preprocess header file
for file in $SOURCE/*.h
do
    sed -e 's/^#include <.*>$//g' $file > $WORKSPACE/$(basename $file)
done

compile()
{
    COMPILER=$1
    OUTPUT=$2

    for file in $SOURCE/*.c
    do
        expand $file $COMPILER
    done

    gcc -Wall -g -static -o $OUTPUT $WORKSPACE/*.s
}

expand()
{
    CFILE=$1
    COMPILER=$2
    OPTION=$3
    TMP1=$WORKSPACE/tmp1.c
    TMP2=$WORKSPACE/tmp2.c

    C_SRC=$WORKSPACE/$(basename $CFILE)
    ASSEMBLY=${C_SRC%.c}.s
    grep -v '^#include <.*>$' $CFILE > $TMP1

    gcc -E $TMP1 | \
    grep -v '#' | \
    sed -e '
        s/\bbool\b/_Bool/g;
        s/\btrue\b/1/g;
        s/\bfalse\b/0/g;
        s/\bNULL\b/(void *)0/g;
        s/\bUINT_MAX\b/4294967295U/g;
        s/\bINT_MAX\b/2147483647/g;
        s/\bULONG_MAX\b/18446744073709551615UL/g;
        s/\bLONG_MAX\b/9223372036854775807L/g;
        s/\bEXIT_FAILURE\b/1/g;
        s/\bSEEK_SET\b/0/g;
        s/\bSEEK_END\b/2/g;
        s/\bERANGE\b/34/g;
        s/\berrno\b/*__errno_location()/g;
        s/\bva_start\b/__builtin_va_start/g;
    ' > $TMP2

    cat $STD_HEADER $TMP2 > $C_SRC

    if [ $COMPILER = 'gcc' ]; then
        # for debug
        gcc -Wall -g -S -masm=intel -o $ASSEMBLY $C_SRC
    else
        $COMPILER $OPTION $C_SRC > $ASSEMBLY
    fi
}

compile $COMPILER_OLD $COMPILER_NEW
