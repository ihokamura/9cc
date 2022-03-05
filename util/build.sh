#!/bin/bash

COMPILER=$1
ASSEMBLY=$2
BINARY=$3

C_PREPROCESSED=test/test_code_pp.c
C_STUB=test/function_call.c
C_TARGET=test/test_code.c

CFLAGS="-g -static -Wall"

# build test code
gcc -E $C_TARGET | grep -v '#' > $C_PREPROCESSED
$COMPILER $C_PREPROCESSED > $ASSEMBLY
gcc $ASSEMBLY $C_STUB $CFLAGS -o $BINARY
rm $C_PREPROCESSED

if [ "$?" != 0 ]; then
    echo "failed to compile"
    exit 1
fi
