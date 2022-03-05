#!/bin/bash

COMPILER=$1
ASSEMBLY=$2
BINARY=$3

C_PREPROCESSED=$TEST_DIRECTORY/test_code_pp.c
C_STUB=$TEST_DIRECTORY/function_call.c
C_TARGET=$TEST_DIRECTORY/test_code.c

# build test code
gcc -E $C_TARGET | grep -v '#' > $C_PREPROCESSED
$COMPILER $C_PREPROCESSED > $ASSEMBLY
gcc $ASSEMBLY $C_STUB $CFLAGS_BASE -static -o $BINARY
rm $C_PREPROCESSED

if [ "$?" != 0 ]; then
    echo "failed to compile"
    exit 1
fi
