/*
test code for 9cc compiler
*/


// function declaration
extern int printf(char *format, ...);
extern void exit(int code);

extern int func_call_return0();
extern int func_call_return1();
extern int func_call_return2();
extern int func_call_add(int x, int y);
extern int func_call_arg6(int a0, int a1, int a2, int a3, int a4, int a5);
extern int func_call_arg8(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7);
extern int func_call_arg7(int a0, int a1, int a2, int a3, int a4, int a5, int a6);
extern void alloc4(int **p, int a0, int a1, int a2, int a3);


/*
print title of a test case
*/
int put_title(char *title)
{
    return printf("--- %s ---\n", title);
}


/*
assertion for char type
*/
int assert_char(char expected, char actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%d expected, but got %d\n", expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for short type
*/
int assert_short(short expected, short actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%d expected, but got %d\n", expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for int type
*/
int assert_int(int expected, int actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%d expected, but got %d\n", expected, actual);
        exit(1);
        return 1;
    }
}


/*
assertion for long type
*/
int assert_long(long expected, long actual)
{
    if(expected == actual)
    {
        return 0;
    }
    else
    {
        printf("%ld expected, but got %ld\n", expected, actual);
        exit(1);
        return 1;
    }
}


/*
number
*/
int test_number()
{
    put_title("number");

    assert_int(0, 0);
    assert_int(42, 42);

    return 0;
}


/*
additive operator
*/
int test_additive()
{
    put_title("additive operator");

    assert_int(21, 5+20-4);

    return 0;
}


/*
multiplicative operator
*/
int test_multiplicative()
{
    put_title("multiplicative operator");

    assert_int(47, 5 + 6 * 7);
    assert_int(15, 5 * (9 - 6));
    assert_int(4, (3 + 5) / 2);
    assert_int(0, 123 % 3);
    assert_int(1, ((123 + 1) % 5) % 3);

    return 0;
}


/*
shift operator
*/
int test_shift()
{
    put_title("shift operator");

    assert_int(4, 1 << 2);
    assert_int(40, 5 << 3);
    assert_int(2, 16 >> 3);
    assert_int(1, 15 >> 3);

    return 0;
}


/*
bitwise AND operator
*/
int test_bitwise_and()
{
    put_title("bitwise AND operator");

    assert_int(0, 1 & 16);
    assert_int(1, 1 & 15);
    assert_int(0, 3 & 12);
    assert_int(0, 0 & 12);

    return 0;
}


/*
bitwise exclusive OR operator
*/
int test_bitwise_xor()
{
    put_title("bitwise XOR operator");

    assert_int(17, 1 ^ 16);
    assert_int(14, 1 ^ 15);
    assert_int(15, 3 ^ 12);
    assert_int(12, 0 ^ 12);

    return 0;
}


/*
bitwise inclusive OR operator
*/
int test_bitwise_or()
{
    put_title("bitwise OR operator");

    assert_int(17, 1 | 16);
    assert_int(15, 1 | 15);
    assert_int(15, 3 | 12);
    assert_int(12, 0 | 12);

    return 0;
}


/*
logical AND operator
*/
int test_logical_and()
{
    put_title("logical AND operator");

    assert_int(0, 0 && 0);
    assert_int(0, 0 && 1);
    assert_int(0, 1 && 0);
    assert_int(1, 1 && 1);

    int a = 0; int b = 0;
    assert_int(0, a++ && ++b); assert_int(1, a); assert_int(0, b);
    assert_int(1, ++a && ++b); assert_int(2, a); assert_int(1, b);

    return 0;
}


/*
logical OR operator
*/
int test_logical_or()
{
    put_title("logical OR operator");

    assert_int(0, 0 || 0);
    assert_int(1, 0 || 1);
    assert_int(1, 1 || 0);
    assert_int(1, 1 || 1);

    int a = 0; int b = 0;
    assert_int(1, a++ || ++b); assert_int(1, a); assert_int(1, b);
    assert_int(1, ++a || ++b); assert_int(2, a); assert_int(1, b);

    return 0;
}


/*
conditional operator
*/
int test_conditional()
{
    put_title("conditional operator");

    int a = 1; int b = 0; int r;
    r = a ? 123 : 456; assert_int(123, r);
    r = b ? 123 : 456; assert_int(456, r);
    r = a-- ? b++ : b--; assert_int(0, r); assert_int(0, a); assert_int(1, b);
    r = a++ ? ++b : --b; assert_int(0, r); assert_int(1, a); assert_int(0, b);
}


/*
unary operator
*/
int test_unary()
{
    put_title("unary operator");

    assert_int(10, -10 + 20);
    assert_int(1, +1 * -2 - (-3));

    int a; a = 123; assert_int(124, ++a); assert_int(124, a);
    int b; b = 123; assert_int(122, --b); assert_int(122, b);
    int c; c = 123; assert_int(123, c++); assert_int(124, c);
    int d; d = 123; assert_int(123, d--); assert_int(122, d);
    int e; e = 15; assert_int(-16, ~e);
    int f; f = 15; assert_int(0, !f); assert_int(1, !(!f));

    int *p; alloc4(&p, 1, 2, 4, 8);
    ++p; assert_int(4, *(++p)); assert_int(4, *p);
    --p; assert_int(1, *(--p)); assert_int(1, *p);
    p++; assert_int(2, *(p++)); assert_int(4, *p);
    p--; assert_int(2, *(p--)); assert_int(1, *p);

    return 0;
}


/*
cast operator
*/
int test_cast()
{
    put_title("cast operator");

    long l;

    char c;
    l = 255; c = (char)l; assert_char(255, c);
    l = 256; c = (char)l; assert_char(0, c);

    short s;
    l = 65535; s = (short)l; assert_short(65535, s);
    l = 65536; s = (short)l; assert_short(0, s);

    int i;
    l = 4294967295; i = (int)l; assert_int(4294967295, i);
    l = 4294967296; i = (int)l; assert_int(0, i);

    return 0;
}


/*
comparision operator
*/
int test_comparision()
{
    put_title("comparision operator");

    assert_int(1, 42 == 6 * 7);
    assert_int(0, 42 != 6 * 7);
    assert_int(0, 42 < 6 * 7);
    assert_int(1, 42 <= 6 * 7);
    assert_int(0, 42 > 6 * 7);
    assert_int(1, 42 >= 6 * 7);

    return 0;
}


/*
local variable
*/
int test_local_variable()
{
    put_title("local variable");

    int a; int b; a = 1; b = 2; assert_int(3, a + b);
    int x; int y; int q; x = 42; y = x / 2; q = (x - y) / 7; assert_int(3, q);
    int alpha; int beta; int gamma; alpha = 2; beta = 5 * alpha; gamma = beta - 3 * alpha; assert_int(4, gamma);

    return 0;
}


/*
assignment
*/
int test_assignment()
{
    put_title("assignment");

    int a; assert_int(123, a = 123);
    int b; b = 123; assert_int(369, b += 246);
    int c; c = 123; assert_int(246, c -= -123);
    int d; d = 123; assert_int(369, d *= 3);
    int e; e = 123; assert_int(41, e /= 3);
    int f; f = 123; assert_int(0, f %= 3);
    int g; g = 4; assert_int(16, g <<= 2);
    int h; h = 4; assert_int(1, h >>= 2);
    int i; i = 8; assert_int(8, i &= 12);
    int j; j = 8; assert_int(4, j ^= 12);
    int k; k = 8; assert_int(12, k |= 12);

    int *p; alloc4(&p, 1, 2, 4, 8);
    p += 2; assert_int(*p, 4);
    p -= 1; assert_int(*p, 2);

    return 0;
}


/*
comma operator
*/
int test_comma()
{
    put_title("comma");

    int a = 0; int b = 123; int r;

    r = a, b; assert_int(0, r);
    r = (a++, b++); assert_int(123, r); assert_int(1, a); assert_int(124, b);
    r = ++a, ++b; assert_int(2, r); assert_int(2, a); assert_int(125, b);

    return 0;
}


/*
goto statement
*/
int func_def_goto(int condition)
{
    if(condition == 1)
    {
        goto label1;
    }

    if(condition == 2)
    {
        goto label2;
    }

    return 0;

label1:
    return 1;

label2:
    return 2;
}
int test_goto_statement()
{
    put_title("goto statement");

    int condition; int r;
    condition = 1; r = func_def_goto(condition); assert_int(1, r);
    condition = 2; r = func_def_goto(condition); assert_int(2, r);
    condition = 0; r = func_def_goto(condition); assert_int(0, r);
}


/*
if statement
*/
int test_if_statement()
{
    put_title("if statement");

    int tmp; int condition;
    tmp = 3; condition = 1; if(condition == 1) tmp = tmp * 2; assert_int(6, tmp);
    tmp = 3; condition = 1; if(condition != 0) tmp = tmp * 2; else tmp = tmp * 3; assert_int(6, tmp);
    tmp = 3; condition = 0; if(condition) tmp = tmp * 2; else tmp = tmp * 3; assert_int(9, tmp);

    return 0;
}


/*
switch statement
*/
int func_def_switch1(int condition)
{
    switch(condition)
    {
    case 1:
        return 1;

    case 3:
        return 3;

    default:
        return 0;
    }
}
int func_def_switch2(int condition)
{
    int r = 0;

    switch(condition)
    {
    case 1:
        r = 1;
        break;

    case 2:
        r = 2;
        ; // intentionally omit break statement

    case 3:
        r = 3;
        break;

    default:
        r = 0;
        break;
    }

    return r;
}
int func_def_switch3(int condition1, int condition2)
{
    int r = 0;

    switch(condition1)
    {
    case 1:
        r = 1;
        break;

    case 2:
    {
        switch(condition2)
        {
        case 5:
            r = 5;
            break;

        case 6:
            r = 6;
            break;

        default:
            r = -1;
            break;
        }
        break;
    }

    case 3:
        r = 3;
        break;

    default:
        r = 0;
        break;
    }

    return r;
}
int test_switch_statement()
{
    put_title("switch statement");

    int condition; int condition1; int condition2; int r;
    condition = 1; r = func_def_switch1(condition); assert_int(1, r);
    condition = 3; r = func_def_switch1(condition); assert_int(3, r);
    condition = 5; r = func_def_switch1(condition); assert_int(0, r);
    condition = 1; r = func_def_switch2(condition); assert_int(1, r);
    condition = 2; r = func_def_switch2(condition); assert_int(3, r);
    condition = 3; r = func_def_switch2(condition); assert_int(3, r);
    condition = 5; r = func_def_switch2(condition); assert_int(0, r);
    condition1 = 1; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_int(1, r);
    condition1 = 2; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_int(5, r);
    condition1 = 2; condition2 = 6; r = func_def_switch3(condition1, condition2); assert_int(6, r);
    condition1 = 2; condition2 = 7; r = func_def_switch3(condition1, condition2); assert_int(-1, r);
    condition1 = 3; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_int(3, r);
    condition1 = 5; condition2 = 5; r = func_def_switch3(condition1, condition2); assert_int(0, r);
}


/*
while statement
*/
int test_while_statement()
{
    put_title("while statement");

    int sum; int i;
    sum = i = 0; while(i < 10){sum = sum + i; i = i + 1;} assert_int(45, sum);
    sum = i = 0; while(1){if(i >= 10){assert_int(45, sum); return sum;} sum = sum + i; i = i + 1;}
    sum = i = 0; while(1){if(i >= 10){break;} sum = sum + i; ++i;} assert_int(45, sum);
    sum = i = 0; while(1){if(i >= 100){sum += i; break;} if(i >= 10){continue;} sum = sum + i; ++i;} assert_int(145, sum);

    return 0;
}


/*
do statement
*/
int test_do_statement()
{
    put_title("do statement");

    int sum; int i;
    sum = i = 0; do{sum = sum + i; i = i + 1;} while(i < 10); assert_int(45, sum);
    sum = i = 0; do{if(i >= 10){assert_int(45, sum); return sum;} else {sum = sum + i; i = i + 1;}} while(1);
    sum = i = 0; do{if(i >= 10){break;} else {sum = sum + i; ++i;}} while(1); assert_int(45, sum);
    sum = i = 0; do{if(i >= 100){sum += i; break;} else if(i >= 10){continue;} else {sum = sum + i; ++i;}} while(1); assert_int(145, sum);

    return 0;
}


/*
for statement
*/
int test_for_statement()
{
    put_title("for statement");

    int sum; int i; int j;
    sum = 0; for(i = 0; i < 10; i = i + 1){sum = sum + i;} assert_int(45, sum);
    sum = 0; i = 0; for(; i < 10; i = i + 1){sum = sum + i;} assert_int(45, sum);
    sum = 0; for(i = 0; i < 10; ){sum = sum + i; i = i + 1;} assert_int(45, sum);
    sum = 0; i = 0; for( ; ; ){sum = sum + i; if(++i >= 10){break;}} assert_int(45, sum);
    sum = 0; i = 0; for( ; ; i++){if(i >= 100){sum += i; break;} else if(i >= 10){continue;} else{sum = sum + i;}} assert_int(145, sum);
    sum = 0; for(i = 0; ; i++){if(i >= 5){break;} for(j = 0; ; j++){if(j >= 10){break;} sum += j;}} assert_int(225, sum);
    sum = 0; for(i = 0; i < 10; i++){if(i >= 5){continue;} for(j = 0; j < 100; ++j){if(j >= 10){continue;} sum += j;}} assert_int(225, sum);

    return 0;
}


/*
function call
*/
int test_function_call()
{
    put_title("function call");

    assert_int(0, func_call_return0());
    assert_int(3, func_call_return1() + func_call_return2());
    assert_int(3, func_call_add(1, 2));
    assert_int(543210, func_call_arg6(0, 1, 2, 3, 4, 5));
    assert_int(6543210, func_call_arg7(0, 1, 2, 3, 4, 5, 6));
    assert_int(76543210, func_call_arg8(0, 1, 2, 3, 4, 5, 6, 7));
}


/*
function definition
*/
int func_def_return0()
{
    return 0;
}
int func_def_return1()
{
    return 1;
}
int func_def_return2()
{
    return 2;
}
int func_def_add(int x, int y)
{
    return x + y;
}
int func_def_arg6(int a0, int a1, int a2, int a3, int a4, int a5)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000;
}
int func_def_arg7(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000;
}
int func_def_arg8(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
    return a0 + a1*10 + a2*100 + a3*1000 + a4*10000 + a5*100000 + a6*1000000 + a7*10000000;
}
int func_def_factorial(int n)
{
    if(n == 0)
    {
        return 1;
    }
    else
    {
        return n * func_def_factorial(n - 1);
    }
}
int func_def_fibonacci(int n)
{
    if(n == 0)
    {
        return 0;
    }
    else if (n == 1)
    {
        return 1;
    }
    else
    {
        return func_def_fibonacci(n - 2) + func_def_fibonacci(n - 1);
    }
}
int test_function_definition()
{
    put_title("function definition");

    assert_int(0, func_def_return0());
    assert_int(3, func_def_return1() + func_def_return2());
    assert_int(3, func_def_add(1, 2));
    assert_int(6, func_def_factorial(3));
    assert_int(543210, func_def_arg6(0, 1, 2, 3, 4, 5));
    assert_int(6543210, func_def_arg7(0, 1, 2, 3, 4, 5, 6));
    assert_int(76543210, func_def_arg8(0, 1, 2, 3, 4, 5, 6, 7));
    assert_int(6, func_def_factorial(3));    
    assert_int(21, func_def_fibonacci(8));

    return 0;
}


/*
address operator and dereference operator
*/
int func_def_argp1(int *x)
{
    return *x;
}
int func_def_argp2(int **x)
{
    return **x;
}
int func_def_argp3(int ***x)
{
    return ***x;
}
int *func_def_returnp1(int *x)
{
    return x;
}
int **func_def_returnp2(int **x)
{
    return x;
}
int test_address_dereference()
{
    put_title("address operator and dereference operator");

    int x; int *y; int **z;
    x = 1; y = &x; assert_int(3, *y + 2);
    x = 1; y = &x; *y = 3; assert_int(3 , x);
    x = 1; y = &x; z = &y; **z = 3; assert_int(3, x);

    int a0; int *a1; int **a2; int b0; int *b1; int **b2;
    a0 = 3; b0 = func_def_argp1(&a0); assert_int(3, b0);
    a0 = 3; a1 = &a0; b0 = func_def_argp2(&a1); assert_int(3, b0);
    a0 = 3; a1 = &a0; a2 = &a1; b0 = func_def_argp3(&a2); assert_int(3, b0);
    a0 = 3; b1 = func_def_returnp1(&a0); assert_int(3, *b1);
    a0 = 3; a1 = &a0; b2 = func_def_returnp2(&a1); assert_int(3, **b2);

    return 0;
}


/*
sizeof operator
*/
int test_sizeof()
{
    put_title("sizeof operator");

    assert_int(4, sizeof(1));
    assert_int(8, sizeof sizeof(1));

    char c; char *pc;
    assert_int(1, sizeof(char));
    assert_int(8, sizeof(char *));
    assert_int(8, sizeof(char **));
    assert_int(1, sizeof(c));
    assert_int(8, sizeof(pc));
    assert_int(8, sizeof(&c));
    assert_int(1, sizeof(*pc));
    assert_int(4, sizeof(c + 1));
    assert_int(8, sizeof(pc + 1));

    short s; short *ps;
    assert_int(2, sizeof(short));
    assert_int(8, sizeof(short *));
    assert_int(8, sizeof(short **));
    assert_int(2, sizeof(s));
    assert_int(8, sizeof(ps));
    assert_int(8, sizeof(&s));
    assert_int(2, sizeof(*ps));
    assert_int(4, sizeof(s + 1));
    assert_int(8, sizeof(ps + 1));

    int i; int *pi;
    assert_int(4, sizeof(int));
    assert_int(8, sizeof(int *));
    assert_int(8, sizeof(int **));
    assert_int(4, sizeof(i));
    assert_int(8, sizeof(pi));
    assert_int(8, sizeof(&i));
    assert_int(4, sizeof(*pi));
    assert_int(4, sizeof(i + 1));
    assert_int(8, sizeof(pi + 1));

    long l; long *pl;
    assert_int(8, sizeof(long));
    assert_int(8, sizeof(long *));
    assert_int(8, sizeof(long **));
    assert_int(8, sizeof(l));
    assert_int(8, sizeof(pl));
    assert_int(8, sizeof(&l));
    assert_int(8, sizeof(*pl));
    assert_int(8, sizeof(l + 1));
    assert_int(8, sizeof(pl + 1));

    struct {int m1; int m2;} st1; assert_int(8, sizeof(st1));
    struct {char m1; int m2; short m3;} st2; assert_int(12, sizeof(st2));
    struct {char m1; struct {char mm1; int mm2; short mm3;} m2; short m3;} st3; assert_int(20, sizeof(st3));

    union {int m1; int m2;} un1; assert_int(4, sizeof(un1));
    union {char m1; int m2; short m3;} un2; assert_int(4, sizeof(un2));
    union {char m1; union {char mm1; int mm2; short mm3;} m2; short m3;} un3; assert_int(4, sizeof(un3));

    assert_int(12, sizeof(const int [3]));
    assert_int(24, sizeof(int const [3][2]));
    assert_int(48, sizeof(int const *[3][2][1]));
    assert_int(48, sizeof(int const * const[3][2][1]));
    assert_int(8, sizeof(int (*)[3][2][1]));
    assert_int(32, sizeof(int (*[4])[3][2][1]));
    assert_int(32, sizeof(int (**[4])[3][2][1]));
    assert_int(8, sizeof(int (*(*)[4])[3][2][1]));
    assert_int(8, sizeof(int (*)(void)));
    assert_int(24, sizeof(int (*[3])(int, int param, char *, volatile long ** const *)));
    typedef void (*sig_handler)(int); assert_int(8, sizeof(sig_handler));

    return 0;
}


/*
pointer addition and subtraction
*/
int test_pointer_addition_subtraction()
{
    put_title("pointer addition and subtraction");

    int *p; int *q; alloc4(&p, 1, 2, 4, 8);
    q = p + 2; assert_int(4, *q);
    q = 2 + p; assert_int(4, *q);
    q = p + 2; assert_int(2, *(q - 1));

    return 0;
}


/*
array
*/
int test_array()
{
    put_title("array");

    int a1[1]; int a2[2]; int a3[1 + 2]; int a4[2 << 1]; int a32[3][2]; int *p; int i; int j;
    *a1 = 2; assert_int(2, *a1);
    *(a2 + 1) = 2; assert_int(2, 2 * *(a2 + 1) - 2);
    *a2 = 1; *(a2 + 1) = 2; p = a2; assert_int(3, *p + *(p + 1));
    for(i = 0; i < 4; i = i + 1){*(a4 + i) = i + 1;} assert_int(10, *a4 + *(a4 + 1) + *(a4 + 2) + *(a4 + 3));
    for(i = 0; i < 4; i = i + 1){a4[i] = i + 1;} assert_int(10, a4[0] + 1[a4] + a4[2] + 3[a4]);
    assert_int(12, sizeof(a3));
    assert_int(1, (a3 == &a3));
    a32[0][0] = 0; assert_int(0, a32[0][0]);
    a32[0][1] = 1; assert_int(1, a32[0][1]);
    a32[0][2] = 2; assert_int(2, a32[0][2]);
    a32[1][0] = 3; assert_int(3, a32[1][0]);
    a32[1][1] = 4; assert_int(4, a32[1][1]);
    a32[1][2] = 5; assert_int(5, a32[1][2]);
    a32[2][1] = 5; assert_int(5, a32[2][1]);
    assert_int(24, sizeof(a32));
    assert_int(8, sizeof(a32[0]));
    assert_int(4, sizeof(a32[0][0]));
    for(i = 0; i < 3; i = i + 1){for(j = 0; j < 2; j = j + 1){a32[i][j] = 2 * i + j;}} assert_int(15, a32[0][0] + a32[0][1] + a32[1][0] + a32[1][1] + a32[2][0] + a32[2][1]);

    return 0;
}


/*
global variable
*/
int gvar_int1; int gvar_int2; int *gvar_p1; int gvar_a4[4]; int gvar_a45[4][5];
int func_def_set_gvar_int1()
{
    gvar_int1 = 1;

    return gvar_int1;
}
int test_global_variable()
{
    put_title("global variable");

    int i;
    gvar_int1 = 1; assert_int(1, gvar_int1);
    gvar_int2 = 2; assert_int(3, func_def_set_gvar_int1() + gvar_int2);
    gvar_int1 = 3; gvar_p1 = &gvar_int1; assert_int(3, *gvar_p1);
    for(i = 0; i < 4; i = i + 1){gvar_a4[i] = i;} assert_int(6, gvar_a4[0] + gvar_a4[1] + gvar_a4[2] + gvar_a4[3]);
    assert_int(80, sizeof gvar_a45);

    return 0;
}


/*
void type
*/
int gvar_int;
void func_def_arg_v(void)
{
    gvar_int *= 2;

    return;
}
int test_void()
{
    put_title("void type");

    gvar_int = 123;
    func_def_arg_v();
    assert_int(246, gvar_int);

    return 0;
}


/*
char type
*/
int test_char()
{
    put_title("char type");

    char c = -1; assert_char(-1, c);
    signed char sc = -2; assert_char(-2, sc);

    return 0;
}


/*
short type
*/
int test_short()
{
    put_title("short type");

    short s = -1; assert_short(-1, s);
    signed short ss = -2; assert_short(-2, ss);
    short int si = -3; assert_short(-3, si);
    signed short int ssi = -4; assert_short(-4, ssi);

    return 0;
}


/*
int type
*/
int test_int()
{
    put_title("int type");

    int i = -1; assert_int(-1, i);
    signed s = -2; assert_int(-2, s);
    signed int si = -3; assert_int(-3, si);

    return 0;
}


/*
long type
*/
int test_long()
{
    put_title("long type");

    long l = -1; assert_long(-1, l);
    signed long sl = -2; assert_long(-2, sl);
    long int li = -3; assert_long(-3, li);
    signed long int sli = -4; assert_long(-4, sli);

    return 0;
}


/*
struct type
*/
int test_struct()
{
    put_title("struct type");

    struct {int m1; int m2;} st1, *pst1;
    st1.m1 = 1; assert_int(1, st1.m1);
    st1.m2 = 2; assert_int(2, st1.m2);
    pst1 = &st1;
    pst1->m1 = 3; assert_int(3, st1.m1);
    pst1->m2 = 4; assert_int(4, st1.m2);

    struct {char m1; int m2; short m3;} st2;
    st2.m1 = 1; assert_char(1, st2.m1);
    st2.m2 = 2; assert_int(2, st2.m2);
    st2.m3 = 3; assert_int(3, st2.m3);

    struct {char m1; struct {char mm1; int mm2; short mm3;} m2; short m3;} st3, st3_copy;
    st3.m1 = 1; assert_char(1, st3.m1);
    st3.m2.mm1 = 11; assert_char(11, st3.m2.mm1);
    st3.m2.mm2 = 22; assert_int(22, st3.m2.mm2);
    st3.m2.mm3 = 33; assert_short(33, st3.m2.mm3);
    st3.m3 = 3; assert_char(3, st3.m3);
    st3_copy = st3; assert_char(1, st3_copy.m1); assert_char(11, st3_copy.m2.mm1); assert_int(22, st3_copy.m2.mm2); assert_short(33, st3_copy.m2.mm3); assert_char(3, st3_copy.m3);

    struct {char m1, *m2; int *m3, m4, **m5;} st4;
    st4.m1 = 1; st4.m2 = &st4.m1; assert_char(1, *st4.m2);
    st4.m4 = 2; st4.m3 = &st4.m4; st4.m5 = &st4.m3; assert_int(2, *st4.m3); assert_int(2, **st4.m5);

    struct {int a1[5];} st5, st5_copy;
    int i; for(i = 0; i < 5; i++) st5.a1[i] = i; st5_copy = st5; for(i = 0; i < 5; i++) assert_int(i, st5_copy.a1[i]);

    return 0;
}


/*
union type
*/
int test_union()
{
    put_title("union type");

    union {int m1; int m2;} un1, *pun1;
    un1.m1 = 1; assert_int(1, un1.m1);
    un1.m2 = 2; assert_int(2, un1.m2);
    pun1 = &un1;
    pun1->m1 = 3; assert_int(3, un1.m1);
    pun1->m2 = 4; assert_int(4, un1.m2);

    union {char m1; int m2; short m3;} un2;
    un2.m1 = 1; assert_char(1, un2.m1);
    un2.m2 = 2; assert_int(2, un2.m2);
    un2.m3 = 3; assert_int(3, un2.m3);

    union {char m1; union {char mm1; int mm2; short mm3;} m2; short m3;} un3;
    un3.m1 = 1; assert_char(1, un3.m1);
    un3.m2.mm1 = 11; assert_char(11, un3.m2.mm1);
    un3.m2.mm2 = 22; assert_int(22, un3.m2.mm2);
    un3.m2.mm3 = 33; assert_short(33, un3.m2.mm3);
    un3.m3 = 3; assert_char(3, un3.m3);

    union {struct {long mm1; long mm2;} m1; int m2[3];} un4, un4_copy;
    un4.m1.mm1 = 1; un4.m1.mm2 = 2; un4_copy = un4; assert_long(1, un4_copy.m1.mm1); assert_long(2, un4_copy.m1.mm2);
    int i; for(i = 0; i < 3; i++) un4.m2[i] = 10 * i; un4_copy = un4; for(i = 0; i < 3; i++) assert_int(10 * i, un4_copy.m2[i]);

    return 0;
}


/*
enumeration type
*/
int test_enum()
{
    put_title("enumeration type");

    enum {EN1, EN2} en1;
    en1 = EN2 * 2;
    assert_int(0, EN1); assert_int(1, EN2); assert_int(2, en1);

    enum {EN3 = 11, EN4, EN5 = 11 * 2, EN6, } en2 = EN4;
    assert_int(11, EN3); assert_int(12, EN4); assert_int(22, EN5); assert_int(23, EN6); assert_int(12, en2);

    return 0;
}


/*
string-literal
*/
int test_string_literal()
{
    put_title("string-literal");

    char *s;
    s = "foo";
    assert_char(111, s[1]);

    return 0;
}


/*
integer-constant
*/
int test_integer_constant()
{
    put_title("integer-constant");

    int sint_max = 2147483647; assert_int(2147483647, sint_max);
    long sint_max_p1 = 2147483648; assert_long(2147483648, sint_max_p1);
    int sint_min = -2147483648; assert_int(-2147483648, sint_min);
    long sint_min_m1 = -2147483649; assert_long(-2147483649, sint_min_m1);
}


/*
initializer
*/
char gvar_char_init = 1;
short gvar_short_init = 2;
int gvar_int_init = 3;
long gvar_long_init = 4;
int gvar_int_uninit1, gvar_int_init1 = 11, gvar_int_init2 = 12, gvar_int_uninit2;
int gvar_a1[3] = {11, 22, 11 + 22};
int gvar_a2[2][2] = {{11, 22}, {33, 44}};
int gvar_a3[][3] = {{11, 22, }, {44, }};
struct {int m1; int m2;} gvar_st1 = {111, 222};
struct {char m1; struct {int mm1; short mm2;} m2;} gvar_st2 = {111, {222, 333}};
char gvar_c1[] = "foo";
char *gvar_str = "foo";

int test_initializer()
{
    put_title("initializer");

    char c = 1; assert_char(1, c);
    short s = 2; assert_short(2, s);
    int i = 3; assert_int(3, i);
    long l = 4; assert_long(4, l);

    int *p; alloc4(&p, 1, 2, 4, 8); int *q = &p[1]; assert_int(4, q[1]);
    char *ps = "foo"; assert_char(111, ps[1]);

    int i1 = 1, i2 = 2, *p1 = &p[1]; assert_int(1, i1); assert_int(i2, 2); assert_int(4, p1[1]);

    assert_int(1, gvar_char_init);
    assert_int(2, gvar_short_init);
    assert_int(3, gvar_int_init);
    assert_int(4, gvar_long_init);

    gvar_int_uninit1 = 1, gvar_int_uninit2 = 2;
    assert_int(1, gvar_int_uninit1);
    assert_int(2, gvar_int_uninit2);
    assert_int(11, gvar_int_init1);
    assert_int(12, gvar_int_init2);

    assert_int(11, gvar_a1[0]); assert_int(22, gvar_a1[1]); assert_int(33, gvar_a1[2]);
    assert_int(11, gvar_a2[0][0]); assert_int(22, gvar_a2[0][1]); assert_int(33, gvar_a2[1][0]); assert_int(44, gvar_a2[1][1]);
    assert_int(11, gvar_a3[0][0]); assert_int(22, gvar_a3[0][1]); assert_int(0, gvar_a3[0][2]); assert_int(44, gvar_a3[1][0]); assert_int(0, gvar_a3[1][1]); assert_int(0, gvar_a3[1][2]);
    assert_int(111, gvar_st1.m1); assert_int(222, gvar_st1.m2);
    assert_int(111, gvar_st2.m1); assert_int(222, gvar_st2.m2.mm1); assert_int(333, gvar_st2.m2.mm2);
    assert_char(111, gvar_c1[1]);
    assert_char(111, gvar_str[1]);

    int a0 = {1}; assert_int(1, a0);
    int a1[3] = {1, 2, 3}; assert_int(1, a1[0]); assert_int(2, a1[1]); assert_int(3, a1[2]);
    int a2[2][3] = {{1, 2, 3}, {4, 5, 6}}; assert_int(1, a2[0][0]); assert_int(2, a2[0][1]); assert_int(3, a2[0][2]); assert_int(4, a2[1][0]); assert_int(5, a2[1][1]); assert_int(6, a2[1][2]);
    int a3[2][2][2] = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}}; int idx1, idx2, idx3; for(idx1 = 0; idx1 < 2; idx1++) for(idx2 = 0; idx2 < 2; idx2++) for(idx3 = 0; idx3 < 2; idx3++)  assert_int(4 * idx1 + 2 * idx2 + 1 * idx3 + 1, a3[idx1][idx2][idx3]);
    int a4[3] = {1, 2, }; assert_int(1, a4[0]); assert_int(2, a4[1]); assert_int(0, a4[2]);
    int a5[2][3] = {{1}, {4, 5}}; assert_int(1, a5[0][0]); assert_int(0, a5[0][1]); assert_int(0, a5[0][2]); assert_int(4, a5[1][0]); assert_int(5, a5[1][1]); assert_int(0, a5[1][2]);
    int a6[] = {1, 2, 3}; assert_int(1, a6[0]); assert_int(2, a6[1]); assert_int(3, a6[2]); assert_int(3, sizeof(a6) / sizeof(a6[0]));
    int a7[][3] = {{1, 2, }, {4, 5, }}; assert_int(1, a7[0][0]); assert_int(2, a7[0][1]); assert_int(0, a7[0][2]); assert_int(4, a7[1][0]); assert_int(5, a7[1][1]); assert_int(0, a7[1][2]); assert_int(2, sizeof(a7) / sizeof(a7[0]));
    char c1[5] = "foo"; assert_char(102, c1[0]); assert_char(111, c1[1]); assert_char(111, c1[2]); assert_char(0, c1[3]); assert_char(0, c1[4]); assert_int(5, sizeof(c1));
    char c2[] = "foo"; assert_char(102, c2[0]); assert_char(111, c2[1]); assert_char(111, c2[2]); assert_char(0, c2[3]); assert_int(4, sizeof(c2));
    const char *c3[] = {"foo", "f"}; assert_char(102, c3[0][0]); assert_char(111, c3[0][1]); assert_char(111, c3[0][2]); assert_char(0, c3[0][3]); assert_char(102, c3[1][0]); assert_char(0, c3[1][1]); 

    typedef struct {int m1; int m2;} st_type;
    st_type st1 = {1, 2}; assert_int(1, st1.m1); assert_int(2, st1.m2);
    st_type st2[3] = {{1, 2}, {3, 4}, {5, 6}}; assert_int(1, st2[0].m1); assert_int(2, st2[0].m2); assert_int(3, st2[1].m1); assert_int(4, st2[1].m2); assert_int(5, st2[2].m1); assert_int(6, st2[2].m2);
    st_type st3[3] = {{1}, st2[1], }; assert_int(1, st3[0].m1); assert_int(0, st3[0].m2); assert_int(3, st3[1].m1); assert_int(4, st3[1].m2); assert_int(0, st3[2].m1); assert_int(0, st3[2].m2);
    typedef union {int m1; int m2;} un_type;
    un_type un1 = {1}; assert_int(1, un1.m1);

    return 0;
}


/*
scope of variables and tags
*/
int test_scope_var = 1;
struct test_scope_tag {int s1;};
int test_scope(void)
{
    put_title("scope");

    assert_int(1, test_scope_var);

    int test_scope_var = 2;
    assert_int(2, test_scope_var);

    {
        int test_scope_var = 3;
        assert_int(3, test_scope_var);
    }

    assert_int(2, test_scope_var);
    struct test_scope_tag tag_var;
    tag_var.s1 = 11; assert_int(11, tag_var.s1);

    {
        struct test_scope_tag {int s1;};
        tag_var.s1 = 22; assert_int(22, tag_var.s1);

        struct test_scope_tag tag_var;
        tag_var.s1 = 33; assert_int(33, tag_var.s1);
    }

    assert_int(22, tag_var.s1);

    {
        struct test_scope_tag;
        struct test_scope_tag {int s1;} tag_var;
        tag_var.s1 = 44; assert_int(44, tag_var.s1);
    }

    {
        union test_scope_tag {int u1;} tag_var;
        tag_var.u1 = -11; assert_int(-11, tag_var.u1);
    }

    {
        enum test_scope_tag {EN1 = 100, EN2};
        enum test_scope_tag tag_var = EN1; assert_int(100, tag_var);
        {
            enum test_scope_tag tag_var = EN2; assert_int(101, tag_var);

            enum test_scope_tag {EN1 = 1000, EN2};
            tag_var = EN2; assert_int(1001, tag_var);
        }
        assert_int(100, tag_var);
    }

    assert_int(22, tag_var.s1);

    return 0;
}


/*
storage class specifier
*/
typedef int int_type;
int test_storage_class(void)
{
    put_title("storage_class");

    int_type t = 0; assert_int(0, t);
    static int s = 2; assert_int(2, s);
    auto int a = 3; assert_int(3, a);
    register int r = 4; assert_int(4, r);

    typedef char type, *ptr_type;
    type c1 = 1; assert_char(1, c1);
    ptr_type pc1 = &c1; assert_char(1, *pc1);

    {
        type c2 = 2; assert_char(2, c2);

        typedef short type;
        type s = -1; assert_short(-1, s);
        assert_int(2, sizeof(type));
    }

    assert_int(1, sizeof(type));

    return 0;
}


/*
type qualifier
*/
int test_type_qualifier(void)
{
    put_title("type qualifier");

    typedef const int const_int;

    const int a = 1; assert_int(1, a);
    volatile int b = 2; assert_int(2, b);
    const volatile int c = 3; assert_int(3, c);
    const int const unsigned volatile d = 4; assert_int(4, d);
    const_int *e = &a; assert_int(1, *e);
    const int * const f = &a; assert_int(1, *f);
    const int * const * restrict volatile g = &f; assert_int(1, **g);

    return 0;
}


/*
main function of test code
*/
int main()
{
    test_number();
    test_additive();
    test_multiplicative();
    test_shift();
    test_bitwise_and();
    test_bitwise_xor();
    test_bitwise_or();
    test_logical_and();
    test_logical_or();
    test_conditional();
    test_unary();
    test_cast();
    test_comparision();
    test_local_variable();
    test_assignment();
    test_comma();
    test_goto_statement();
    test_if_statement();
    test_switch_statement();
    test_while_statement();
    test_do_statement();
    test_for_statement();
    test_function_call();
    test_function_definition();
    test_address_dereference();
    test_sizeof();
    test_pointer_addition_subtraction();
    test_array();
    test_global_variable();
    test_void();
    test_char();
    test_short();
    test_int();
    test_long();
    test_struct();
    test_union();
    test_enum();
    test_string_literal();
    test_integer_constant();
    test_initializer();
    test_scope();
    test_storage_class();
    test_type_qualifier();

    return 0;
}
