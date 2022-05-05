#!/bin/bash

COMPILER=$1
ASSEMBLY=$2
BINARY=$3
DEFINE=$4

C_PREPROCESSED=$TEST_DIRECTORY/test_code_pp.c
C_STUB="$TEST_DIRECTORY/function_call.c $TEST_DIRECTORY/lib_assert.c"
C_TARGET=$TEST_DIRECTORY/test_code.c
ASSEMBLY_TEMPORAL=$TEST_DIRECTORY/test_code_tmp.s

# build test code
gcc -DQCC_COMPILER -D${DEFINE} -E $C_TARGET | grep -v '^#' > $C_PREPROCESSED
$COMPILER $C_PREPROCESSED > $ASSEMBLY_TEMPORAL
cat $ASSEMBLY_TEMPORAL | sed -rz -e '
    s/\tpush ([a-z0-9]+)\n\tpop ([a-z][a-z0-9]+)\n/\tmov \2, \1\n/g;
    s/\tmov ([a-z]+), ([a-z]+)\n\tmov ([a-z]+), \1\n/\tmov \3, \2\n/g
    s/\tmov ([a-z]+), \1\n//g;
    s/\tadd ([a-z][a-z0-9]+), ([0-9]+)\n\tsub \1, \2\n//g;
' > $ASSEMBLY
gcc $ASSEMBLY $C_STUB $CFLAGS_BASE -static -o $BINARY
rm $C_PREPROCESSED $ASSEMBLY_TEMPORAL

if [ "$?" != 0 ]; then
    echo "failed to compile"
    exit 1
fi
