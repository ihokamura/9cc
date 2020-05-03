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
#include <string.h>

#include "generator.h"
#include "parser.h"
#include "tokenizer.h"


/*
main function of 9cc
*/
int main(int argc, char **argv)
{
    // check arguments
    char *user_input = NULL;
    switch(argc)
    {
    case 2:
        // input from a file
        // format: 9cc <file>
        set_source(SRC_FILE);
        user_input = read_file(argv[1]);
        break;

    case 3:
        // input from command line
        // format: 9cc -c <code>
        if(strcmp(argv[1], "-c") != 0)
        {
            fprintf(stderr, "invalid option: %s\n", argv[1]);
            return 1;
        }
        else
        {
            set_source(SRC_CMD);
            user_input = argv[2];
        }
        break;

    default:
        // Other formats are not accepted.
        fprintf(stderr, "Usage: 9cc [ <file> | -c <code> ]\n");
        return 1;
    }

    // tokenize input
    tokenize(user_input);

    // construct syntax tree
    Program program;
    construct(&program);

    // output assembler according to the syntax tree
    generate(&program);

    return 0;
}