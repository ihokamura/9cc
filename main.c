/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* main function
*/

#include <stdio.h>

#include "9cc.h"


/*
main function of 9cc
*/
int main(int argc, char **argv)
{
    // check arguments
    if(argc != 2)
    {
        fprintf(stderr, "wrong number of arguments.\n");
        return 1;
    }

    // parse input
    char *user_input = argv[1];
    tokenize(user_input);

    // construct syntax tree
    construct();

    // output assembler according to the syntax tree
    generate();

    return 0;
}