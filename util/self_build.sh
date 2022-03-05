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

    expand declaration.c $COMPILER
    expand expression.c $COMPILER
    expand generator.c $COMPILER
    expand main.c $COMPILER
    expand parser.c $COMPILER
    expand statement.c $COMPILER
    expand tokenizer.c $COMPILER
    expand type.c $COMPILER

    gcc -Wall -g -static -o $OUTPUT $WORKSPACE/*.s
}

expand()
{
    CFILE=$1
    COMPILER=$2
    OPTION=$3
    TMP1=tmp1.c
    TMP2=tmp2.c

    grep -v '^#include <.*>$' $SOURCE/$CFILE > $WORKSPACE/$TMP1

    gcc -E $WORKSPACE/$TMP1 | \
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
    ' > $WORKSPACE/$TMP2

    cat $STD_HEADER $WORKSPACE/$TMP2 > $WORKSPACE/$CFILE

    if [ $COMPILER = 'gcc' ]; then
        # for debug
        gcc -Wall -g -S -masm=intel -o $WORKSPACE/${1%.c}.s $WORKSPACE/$CFILE
    else
        $COMPILER $OPTION $WORKSPACE/$CFILE > $WORKSPACE/${1%.c}.s
    fi
}

compile $COMPILER_OLD $COMPILER_NEW
