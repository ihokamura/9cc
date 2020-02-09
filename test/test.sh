#!/bin/bash

# move to test directory
pushd ./test

# compile test code
../9cc test_code > test_code.s
gcc test_code.s function_call.c -o test_bin -g -static

if [ "$?" == 0 ]; then
    # execute test
    ./test_bin
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

# move to the original directory
popd