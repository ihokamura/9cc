/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv)
{
    /* check arguments */
    if(argc != 2)
    {
        fprintf(stderr, "wrong number of arguments.\n");
        return 1;
    }

    /* use Intel syntax */
    printf(".intel_syntax noprefix\n");

    /* start main function */
    printf(".global _main\n");
    printf("_main:\n");

    /* parse argument and output assembler code */
    char *p = argv[1];
    printf("  mov rax, %ld\n", strtol(p, &p, 10));
    while(*p)
    {
        if(*p == '+')
        {
            /* addition */
            p++;
            printf("  add rax, %ld\n", strtol(p, &p, 10));
        }
        else if(*p == '-')
        {
            /* subtraction */
            p++;
            printf("  sub rax, %ld\n", strtol(p, &p, 10));
        }
        else
        {
            /* Other characters are prohibited. */
            fprintf(stderr, "unexpected character: '%c'\n", *p);
            return 1;
        }
    }

    /* exit main function */
    printf("  ret\n");

    return 0;
}