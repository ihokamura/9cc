#!/bin/bash

# variables
COMPILER=$1
ASSEMBLY=$2
BINARY=$3
WORKSPACE=$(pwd)/test
ORIGINAL=$WORKSPACE/test_code.c
PREPROCESSED=$WORKSPACE/tmp.c


# compile test code
gcc -E $ORIGINAL | grep -v '#' > $PREPROCESSED
$COMPILER $PREPROCESSED > $WORKSPACE/$ASSEMBLY
gcc -g -static -o $WORKSPACE/$BINARY $WORKSPACE/$ASSEMBLY $WORKSPACE/function_call.c

if [ "$?" == 0 ]; then
    # execute test
    $WORKSPACE/$BINARY
    if [ "$?" == 0 ]; then
        echo "passed tests"
        exit 0
    else
        echo "failed tests"
        exit 1
    fi
else
    # report error
    echo "failed to compile"
    exit 1
fi


# remove intermediate files
rm $PREPROCESSED
