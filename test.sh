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

try_all()
{
# only number
try 0 "main(){0;}"
try 42 "main(){42;}"

# operators in the same priority
try 21 "main(){5+20-4;}"

# space
try 41 "main(){12 + 34 - 5;}"
try 41 "  main  (  )  {  12  +  34  -  5;  }  "

# operators in different priorities
try 47 "main(){5 + 6 * 7;}"
try 15 "main(){5 * (9 - 6);}"
try 4 "main(){(3 + 5) / 2;}"

# unary operator
try 10 "main(){-10 + 20;}"
try 1 "main(){+1 * -2 - (-3);}"

# comparision operator
try 1 "main(){42 == 6 * 7;}"
try 0 "main(){42 != 6 * 7;}"
try 0 "main(){42 < 6 * 7;}"
try 1 "main(){42 <= 6 * 7;}"
try 0 "main(){42 > 6 * 7;}"
try 1 "main(){42 >= 6 * 7;}"

# local variable
try 3 "main(){a = 1; b = 2; a + b;}"
try 3 "main(){x = 42; y = x / 2; q = (x - y) / 7;}"
try 4 "main(){alpha = 2; beta = 5 * alpha; gamma = beta - 3 * alpha;}"

# return statement
try 42 "main(){return 42;}"
try 4 "main(){alpha = 2; beta = 5 * alpha; return beta - 3 * alpha; return 0;}"
try 0 "main(){alpha = 2; beta = 5 * alpha; return_beta - 3 * alpha; return 0;}"
try 9 "main(){a1 = 1; a2 = 2; b_11 = 3; b_22 = 4; a = a1 + a2; b = b_11 * b_22; return a + b / a2;}"

# if statement
try 6 "main(){tmp = 3; condition = 1; if(condition == 1) tmp = tmp * 2; return tmp;}"
try 6 "main(){tmp = 3; condition = 1; if(condition != 0) tmp = tmp * 2; else tmp = tmp * 3; return tmp;}"
try 9 "main(){tmp = 3; condition = 0; if(condition) tmp = tmp * 2; else tmp = tmp * 3; return tmp;}"

# while statement
try 45 "main(){sum = i = 0; while(i < 10){sum = sum + i; i = i + 1;} return sum;}"
try 45 "main(){sum = i = 0; while(1){if(i >= 10) return sum; sum = sum + i; i = i + 1;} return sum;}"

# do statement
try 45 "main(){sum = i = 0; do{sum = sum + i; i = i + 1;} while(i < 10); return sum;}"
try 45 "main(){sum = 0; i = 0; do{if(i >= 10) {return sum;} else {sum = sum + i; i = i + 1; }} while(1); return sum;}"

# for statement
try 45 "main(){sum = 0; for(i = 0; i < 10; i = i + 1){sum = sum + i;} return sum;}"
try 45 "main(){sum = 0; i = 0; for(; i < 10; i = i + 1){sum = sum + i;} return sum;}"
try 45 "main(){sum = 0; for(i = 0; ; i = i + 1){if(i >= 10) {return sum;} sum = sum + i;}}"
try 45 "main(){sum = 0; for(i = 0; i < 10; ){sum = sum + i; i = i + 1;} return sum;}"

# function call
try_func 0 "main(){testfunc1(); return 0;}"
try_func 3 "main(){alpha = 1; beta = 2; testfunc1(); return alpha + beta;}"
try_func 6 "main(){alpha = 1; beta = 2; gamma = 3; testfunc1(); return alpha + beta + gamma;}"
try_func 0 "main(){testfunc2(1); return 0;}"
try_func 3 "main(){alpha = 1; beta = 2; testfunc3(alpha, beta); return alpha + beta;}"
try_func 3 "main(){alpha = 2; beta = 1; testfunc4(alpha, beta, alpha + beta, alpha - beta, alpha * beta, alpha / beta); return alpha + beta;}"
try_func 123 "main(){alpha = 2; beta = 1; return testfunc4(alpha, beta, alpha + beta, alpha - beta, alpha * beta, alpha / beta);}"

# function definition
try 0 "func(){return 0;} main(){return func();}"
try 3 "func(x, y){return x + y;} main(){tmp = func(1, 2); return tmp;}"
try 3 "func1(){return 1;} func2(){return 2;} main(){return func1() + func2();}"
try_func 0 "func(a, b, c, d, e, f){print_int(a); print_int(b); print_int(c); print_int(d); print_int(e); print_int(f); return 0;} main(){tmp = func(1, 2, 3, 4, 5, 6); return tmp;}"
try 6 "fact(n){if(n == 0){return 1;} else{return n * fact(n - 1);}} main(){return fact(3);}"
try 21 "fib(n){if(n > 1){return fib(n - 1) + fib(n - 2);} else if(n == 1){return 1;} else{return 0;}} main(){return fib(8);}"

# address operator and dereference operator
try 3 "main(){x = 1; y = &x; return *y + 2;}"
}

try_all

echo OK