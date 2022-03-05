#!/bin/bash

# variables
SOURCE=$(pwd)/source
WORKSPACE=$(pwd)/self
TEST=$(pwd)/test
STD_HEADER=std_header.h
GEN1_BIN=$(pwd)/build/9cc
GEN2_BIN=$WORKSPACE/9cc_gen2
GEN3_BIN=$WORKSPACE/9cc_gen3
GEN1_ASSEMBLY=$TEST/test_code.s
GEN2_ASSEMBLY=$TEST/test_code_gen2.s
GEN3_ASSEMBLY=$TEST/test_code_gen3.s


# functions
prepare()
{
    for file in $SOURCE/*.h
    do
        sed -e 's/^#include <.*>$//g' $file > $WORKSPACE/$(basename $file)
    done

    cp util/$STD_HEADER $WORKSPACE
}

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

test()
{
    COMPILER=$1
    ASSEMBLY=$2
    BINARY=$3
    TEST_SCRIPT=$(pwd)/util/test.sh

    bash $TEST_SCRIPT $COMPILER $ASSEMBLY $BINARY
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

    cat $WORKSPACE/$STD_HEADER $WORKSPACE/$TMP2 > $WORKSPACE/$CFILE

    if [ $(basename $COMPILER) = 'gcc' ]; then
        # for debug
        gcc -Wall -g -S -masm=intel -o $WORKSPACE/${1%.c}.s $WORKSPACE/$CFILE
    else
        $COMPILER $OPTION $WORKSPACE/$CFILE > $WORKSPACE/${1%.c}.s
    fi
}

check_diff()
{
    FILE1=$1
    FILE2=$2

    # compare files and report whether they are equal or not.
    diff $FILE1 $FILE2
    if [ $? != 0 ]; then
        echo $FILE1 is different from $FILE2.
    fi
}


# self-compile and run test
prepare
compile $GEN1_BIN $GEN2_BIN
test $GEN2_BIN $(basename $GEN2_ASSEMBLY) $(basename $GEN2_BIN)
compile $GEN2_BIN $GEN3_BIN
test $GEN3_BIN $(basename $GEN3_ASSEMBLY) $(basename $GEN3_BIN)

# check difference of assembly codes
check_diff $GEN2_ASSEMBLY $GEN3_ASSEMBLY
