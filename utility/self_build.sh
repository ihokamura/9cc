#!/bin/bash

COMPILER_OLD=$1
COMPILER_NEW=$2

STD_HEADER=$UTILITY_DIRECTORY/std_header.h

remove_include()
{
    input=$1
    output=$2

    grep -v '#include <.*>' $input > $output
}

assemble()
{
    compiler=$1
    c_file=$2
    compiler_option=$3

    compiler_input=$SELF_BUILD_DIRECTORY/$(basename $c_file)
    s_file=${compiler_input%.c}.s
    tmp1=$SELF_BUILD_DIRECTORY/tmp1.c
    tmp2=$SELF_BUILD_DIRECTORY/tmp2.c

    remove_include $c_file $tmp1

    gcc -E $tmp1 | \
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
    ' > $tmp2

    cat $STD_HEADER $tmp2 > $compiler_input

    if [ $compiler = 'gcc' ]; then
        # for debug
        gcc $compiler_input $CFLAGS_BASE -S -masm=intel -o $s_file
    else
        $compiler $compiler_input $compiler_option > $s_file
    fi
}


for file in $SOURCE_DIRECTORY/*.h
do
    remove_include $file $SELF_BUILD_DIRECTORY/$(basename $file)
done

for file in $SOURCE_DIRECTORY/*.c
do
    assemble $COMPILER_OLD $file
done

gcc $SELF_BUILD_DIRECTORY/*.s $CFLAGS_BASE -static -o $COMPILER_NEW
