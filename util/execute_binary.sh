#!/bin/bash

BINARY=$1

# execute test code
$BINARY

if [ "$?" == 0 ]; then
    echo "passed tests"
    exit 0
else
    echo "failed tests"
    exit 1
fi
