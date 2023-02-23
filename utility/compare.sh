#!/bin/bash

COMPILER_SELF=$1
COMPILER_OTHER=$2

for c_src in $(ls $TEST_DIRECTORY/target/*.c)
do
    diff -q ${c_src/\.c/_$(basename $COMPILER_SELF)\.s} ${c_src/\.c/_$(basename $COMPILER_OTHER)\.s}
done
