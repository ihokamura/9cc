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

try_func()
{
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc test_functions.c -o test_functions.o -std=c11 -g -Wall -c
    gcc tmp.s test_functions.o -o tmp

    ./tmp
    actual="$?"

    rm test_functions.o

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
try 0 "alpha = 2; beta = 5 * alpha; return_beta - 3 * alpha; return 0;"
try 9 "a1 = 1; a2 = 2; b_11 = 3; b_22 = 4; a = a1 + a2; b = b_11 * b_22; return a + b / a2;"

# if statement and if-else statement
try 6 "tmp = 3; condition = 1; if(condition == 1) tmp = tmp * 2; return tmp;"
try 6 "tmp = 3; condition = 1; if(condition != 0) tmp = tmp * 2; else tmp = tmp * 3; return tmp;"
try 9 "tmp = 3; condition = 0; if(condition) tmp = tmp * 2; else tmp = tmp * 3; return tmp;"

# while statement
try 5 "count = 0; while(count < 5) count = count + 1; return count;"

# for statement
try 5 "for(count = 0; count < 5; count = count + 1) dummy = 1; return count;"
try 3 "count = 0; for(; count + 2 < 5; ) count = count + 1; return count;"

# compound statement
try 5 "tmp = 2; condition = 1; if(condition == 1) {tmp = 2; tmp = 2 * tmp + 1;} else {tmp = 0;} return tmp;"
try 2 "tmp = 2; condition = 1; if(condition != 1) {tmp = 2; tmp = 2 * tmp + 1;} else {} return tmp;"

# function call
try_func 0 "testfunc1(); return 0;"

echo OK