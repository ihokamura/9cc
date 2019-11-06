#!/bin/bash

try()
{
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc tmp.s -o tmp

    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# only number
try 0 "0;"
try 42 "42;"

# operators in the same priority
try 21 "5+20-4;"

# space
try 41 "12 + 34 - 5;"
try 41 "  12  +  34  -  5;  "

# operators in different priorities
try 47 "5 + 6 * 7;"
try 15 "5 * (9 - 6);"
try 4 "(3 + 5) / 2;"

# unary operator
try 10 "-10 + 20;"
try 1 "+1 * -2 - (-3);"

# comparision operator
try 1 "42 == 6 * 7;"
try 0 "42 != 6 * 7;"
try 0 "42 < 6 * 7;"
try 1 "42 <= 6 * 7;"
try 0 "42 > 6 * 7;"
try 1 "42 >= 6 * 7;"

# local variable
try 3 "a = 1; b = 2; a + b;"
try 3 "x = 42; y = x / 2; q = (x - y) / 7;"
try 4 "alpha = 2; beta = 5 * alpha; gamma = beta - 3 * alpha;"

# return statement
try 42 "return 42;"
try 4 "alpha = 2; beta = 5 * alpha; return beta - 3 * alpha; return 0;"

echo OK