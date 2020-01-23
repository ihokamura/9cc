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
try 0 "int main(){0;}"
try 42 "int main(){42;}"

# operators in the same priority
try 21 "int main(){5+20-4;}"

# space
try 41 "int main(){12 + 34 - 5;}"
try 41 "  int  main  (  )  {  12  +  34  -  5;  }  "

# operators in different priorities
try 47 "int main(){5 + 6 * 7;}"
try 15 "int main(){5 * (9 - 6);}"
try 4 "int main(){(3 + 5) / 2;}"

# unary operator
try 10 "int main(){-10 + 20;}"
try 1 "int main(){+1 * -2 - (-3);}"

# comparision operator
try 1 "int main(){42 == 6 * 7;}"
try 0 "int main(){42 != 6 * 7;}"
try 0 "int main(){42 < 6 * 7;}"
try 1 "int main(){42 <= 6 * 7;}"
try 0 "int main(){42 > 6 * 7;}"
try 1 "int main(){42 >= 6 * 7;}"

# local variable
try 3 "int main(){int a; int b; a = 1; b = 2; a + b;}"
try 3 "int main(){int x; int y; int q; x = 42; y = x / 2; q = (x - y) / 7;}"
try 4 "int main(){int alpha; int beta; int gamma; alpha = 2; beta = 5 * alpha; gamma = beta - 3 * alpha;}"

# return statement
try 42 "int main(){return 42;}"
try 4 "int main(){int alpha; int beta; alpha = 2; beta = 5 * alpha; return beta - 3 * alpha; return 0;}"
try 0 "int main(){int alpha; int beta; int return_beta; alpha = 2; beta = 5 * alpha; return_beta - 3 * alpha; return 0;}"
try 9 "int main(){int a; int a1; int a2; int b_11; int b; int b_22; a1 = 1; a2 = 2; b_11 = 3; b_22 = 4; a = a1 + a2; b = b_11 * b_22; return a + b / a2;}"

# if statement
try 6 "int main(){int tmp; int condition; tmp = 3; condition = 1; if(condition == 1) tmp = tmp * 2; return tmp;}"
try 6 "int main(){int tmp; int condition; tmp = 3; condition = 1; if(condition != 0) tmp = tmp * 2; else tmp = tmp * 3; return tmp;}"
try 9 "int main(){int tmp; int condition; tmp = 3; condition = 0; if(condition) tmp = tmp * 2; else tmp = tmp * 3; return tmp;}"

# while statement
try 45 "int main(){int sum; int i; sum = i = 0; while(i < 10){sum = sum + i; i = i + 1;} return sum;}"
try 45 "int main(){int sum; int i; sum = i = 0; while(1){if(i >= 10) return sum; sum = sum + i; i = i + 1;} return sum;}"

# do statement
try 45 "int main(){int sum; int i; sum = i = 0; do{sum = sum + i; i = i + 1;} while(i < 10); return sum;}"
try 45 "int main(){int sum; int i; sum = 0; i = 0; do{if(i >= 10) {return sum;} else {sum = sum + i; i = i + 1; }} while(1); return sum;}"

# for statement
try 45 "int main(){int sum; int i; sum = 0; for(i = 0; i < 10; i = i + 1){sum = sum + i;} return sum;}"
try 45 "int main(){int sum; int i; sum = 0; i = 0; for(; i < 10; i = i + 1){sum = sum + i;} return sum;}"
try 45 "int main(){int sum; int i; sum = 0; for(i = 0; ; i = i + 1){if(i >= 10) {return sum;} sum = sum + i;}}"
try 45 "int main(){int sum; int i; sum = 0; for(i = 0; i < 10; ){sum = sum + i; i = i + 1;} return sum;}"

# function call
try_func 0 "int main(){testfunc1(); return 0;}"
try_func 3 "int main(){int alpha; int beta; alpha = 1; beta = 2; testfunc1(); return alpha + beta;}"
try_func 6 "int main(){int alpha; int beta; int gamma; alpha = 1; beta = 2; gamma = 3; testfunc1(); return alpha + beta + gamma;}"
try_func 0 "int main(){testfunc2(1); return 0;}"
try_func 3 "int main(){int alpha; int beta; alpha = 1; beta = 2; testfunc3(alpha, beta); return alpha + beta;}"
try_func 3 "int main(){int alpha; int beta; alpha = 2; beta = 1; testfunc4(alpha, beta, alpha + beta, alpha - beta, alpha * beta, alpha / beta); return alpha + beta;}"
try_func 123 "int main(){int alpha; int beta; alpha = 2; beta = 1; return testfunc4(alpha, beta, alpha + beta, alpha - beta, alpha * beta, alpha / beta);}"

# function definition
try 0 "int func(){return 0;} int main(){return func();}"
try 3 "int func(int x, int y){return x + y;} int main(){int tmp; tmp = func(1, 2); return tmp;}"
try 3 "int func1(){return 1;} int func2(){return 2;} int main(){return func1() + func2();}"
try_func 0 "int func(int a, int b, int c, int d, int e, int f){print_int(a); print_int(b); print_int(c); print_int(d); print_int(e); print_int(f); return 0;} int main(){int tmp; tmp = func(1, 2, 3, 4, 5, 6); return tmp;}"
try 6 "int fact(int n){if(n == 0){return 1;} else{return n * fact(n - 1);}} int main(){return fact(3);}"
try 21 "int fib(int n){if(n > 1){return fib(n - 1) + fib(n - 2);} else if(n == 1){return 1;} else{return 0;}} int main(){return fib(8);}"

# address operator and dereference operator
try 3 "int main(){int x; int *y; x = 1; y = &x; return *y + 2;}"
try 3 "int main(){int x; int *y; x = 1; y = &x; *y = 3; return x;}"
try 3 "int main(){int x; int *y; int **z; x = 1; y = &x; z = &y; **z = 3; return x;}"
try 3 "int func(int *x){return *x;} int main(){int a0; int b; a0 = 3; b = func(&a0); return b;}"
try 3 "int func(int **x){return **x;} int main(){int a0; int *a1; int b; a0 = 3; a1 = &a0; b = func(&a1); return b;}"
try 3 "int func(int ***x){return ***x;} int main(){int a0; int *a1; int **a2; int b; a0 = 3; a1 = &a0; a2 = &a1; b = func(&a2); return b;}"
try 3 "int *func(int *x){return x;} int main(){int a0; int *b; a0 = 3; b = func(&a0); return *b;}"
try 3 "int *func(int **x){return *x;} int main(){int a0; int *a1; int *b; a0 = 3; a1 = &a0; b = func(&a1); return *b;}"
try 3 "int **func(int **x){return x;} int main(){int a0; int *a1; int **b; a0 = 3; a1 = &a0; b = func(&a1); return **b;}"

# pointer addition and subtraction
try_func 4 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *q;}"
try_func 4 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = 2 + p; return *q;}"
try_func 2 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *(q - 1);}"

# sizeof operator
try 4 "int main(){return sizeof(1);}"
try 4 "int main(){return sizeof sizeof(1);}"
try 4 "int main(){int x; return sizeof(x);}"
try 8 "int main(){int *y; return sizeof(y);}"
try 8 "int main(){int x; return sizeof(&x);}"
try 4 "int main(){int *y; return sizeof(*y);}"
try 4 "int main(){int x; return sizeof(x + 3);}"
try 8 "int main(){int *y; return sizeof(y + 3);}"
try 4 "int main(){int *y; return sizeof(*(y + 3));}"

# array
try 2 "int main(){int a[1]; *a = 2; return *a;}"
try 2 "int main(){int a[2]; *(a + 1) = 2; return 2 * *(a + 1) - 2;}"
try 3 "int main(){int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);}"
try 10 "int main(){int a[4]; int i; for(i = 0; i < 4; i = i + 1){*(a + i) = i + 1;} return *a + *(a + 1) + *(a + 2) + *(a + 3);}"
try 10 "int main(){int a[4]; int i; for(i = 0; i < 4; i = i + 1){a[i] = i + 1;} return a[0] + 1[a] + a[2] + 3[a];}"
try 12 "int main(){int a[3]; return sizeof(a);}"
try 1 "int main(){int a[3]; return (a == &a);}"
try 0 "int main(){int a[2][3]; a[0][0] = 0; return a[0][0];}"
try 1 "int main(){int a[2][3]; a[0][1] = 1; return a[0][1];}"
try 2 "int main(){int a[2][3]; a[0][2] = 2; return a[0][2];}"
try 3 "int main(){int a[2][3]; a[1][0] = 3; return a[1][0];}"
try 4 "int main(){int a[2][3]; a[1][1] = 4; return a[1][1];}"
try 5 "int main(){int a[2][3]; a[1][2] = 5; return a[1][2];}"
try 5 "int main(){int a[3][2]; a[2][1] = 5; return a[2][1];}"
try 24 "int main(){int a[3][2]; return sizeof(a);}"
try 8 "int main(){int a[3][2]; return sizeof(a[0]);}"
try 4 "int main(){int a[3][2]; return sizeof(a[0][0]);}"
try 15 "int main(){int a[3][2]; int i; int j; for(i = 0; i < 3; i = i + 1){for(j = 0; j < 2; j = j + 1){a[i][j] = 2 * i + j;}} return a[0][0] + a[0][1] + a[1][0] + a[1][1] + a[2][0] + a[2][1];}"
}

try_all

echo OK