#!/bin/bash

COMPILER=$1
BINARY=$2
DEFINE=$3

C_STUB="$TEST_DIRECTORY/common/*.c $TEST_DIRECTORY/stub/*.c"
C_TARGET="$TEST_DIRECTORY/target/*.c"
S_TARGET="$TEST_DIRECTORY/target/*_$(basename $COMPILER).s"

# build test code
for c_src in $(ls $C_TARGET)
do
    c_preprocessed=${c_src/\.c/_pp.c}
    assembly_temporal=${c_src/\.c/_$(basename $COMPILER)_tmp.s}

    gcc -DQCC_COMPILER -D${DEFINE} -E $c_src | grep -v '^#' > $c_preprocessed
    $COMPILER $c_preprocessed > $assembly_temporal
    cat $assembly_temporal | sed -rz -e '
    s/\tpush ([a-z0-9]+)\n\tpop ([a-z][a-z0-9]+)\n/\tmov \2, \1\n/g;
    s/\tmov ([a-z]+), ([a-z]+)\n\tmov ([a-z]+), \1\n/\tmov \3, \2\n/g
    s/\tmov ([a-z]+), \1\n//g;
    s/\tadd ([a-z][a-z0-9]+), ([0-9]+)\n\tsub \1, \2\n//g;
' > ${c_src/\.c/_$(basename $COMPILER).s}
    rm $c_preprocessed $assembly_temporal
done
gcc $S_TARGET $C_STUB $CFLAGS_BASE -static -o $BINARY

if [ "$?" != 0 ]; then
    echo "failed to compile"
    exit 1
fi
