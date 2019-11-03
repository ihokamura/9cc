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

    /* output assembler */
    printf(".intel_syntax noprefix\n");
    printf(".global _main\n");
    printf("_main:\n");
    printf("  mov rax, %d\n", atoi(argv[1]));
    printf("  ret\n");

    return 0;
}