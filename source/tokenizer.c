/*
# 9cc compiler

## Reference
[1] https://www.sigbus.info/compilerbook
*/

/*
# Contents of this file
* tokenizer (user input parser)
*/

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "type.h"

// definition of list
#include "list.h"
define_list(Token)
define_list_operations(Token)

// type definition
typedef enum IntegerSuffix IntegerSuffix;
enum IntegerSuffix
{
    IS_NONE,              // none
    IS_UNSIGNED,          // unsigned-suffix
    IS_LONG,              // long-suffix
    IS_UNSIGNED_LONG,     // unsigned-suffix and long-suffix
    IS_LONGLONG,          // long-long-suffix
    IS_UNSIGNED_LONGLONG, // unsigned-suffix and long-long-suffix
};


// function prototype
static Token *new_token(TokenKind kind, char *str, int len);
static int is_space(const char *str);
static int is_comment(const char *str);
static int is_reserved(const char *str);
static int is_identifier(const char *str);
static int is_string(const char *str);
static int is_constant(const char *str, TypeKind *kind, long *value);
static int is_octal_digit(int character);
static int is_hexadeciaml_digit(int character);
static int parse_escape_sequence(const char *str);
static long convert_integer_constant(const char *start, int base, IntegerSuffix suffix, TypeKind *kind);
static int convert_character_constant(const char *str);
static void report_position(const char *loc);


// global variable
// list of punctuators
// It is necessary to put longer strings above than shorter strings.
static const char *punctuator_list[] = {
    "...",
    "<<=",
    ">>=",
    "->",
    "==",
    "!=",
    "<=",
    ">=",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "&=",
    "^=",
    "|=",
    "++",
    "--",
    "<<",
    ">>",
    "&&",
    "||",
    ".",
    "[",
    "]",
    "(",
    ")",
    "{",
    "}",
    "&",
    "*",
    "+",
    "-",
    "~",
    "!",
    "/",
    "%",
    "<",
    ">",
    "^",
    "|",
    "?",
    ":",
    ";",
    "=",
    ",",
};
static const size_t PUNCTUATOR_LIST_SIZE = sizeof(punctuator_list) / sizeof(punctuator_list[0]); // number of punctuators
// list of keywords
static const char *keyword_list[] = {
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "else",
    "enum",
    "extern",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "register",
    "restrict",
    "return",
    "signed",
    "sizeof",
    "short",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    "_Alignof",
    "_Atomic",
    "_Bool",
    "_Generic",
    "_Thread_local",
};
static const size_t KEYWORD_LIST_SIZE = sizeof(keyword_list) / sizeof(keyword_list[0]); // number of keywords
// map of simple escape sequences (excluding "\")
static const struct {int character; int value;} simple_escape_sequence_map[] = {
    {'\'', '\''},
    {'\"', '\"'},
    {'\?', '\?'},
    {'\\', '\\'},
    {'a', '\a'},
    {'b', '\b'},
    {'f', '\f'},
    {'n', '\n'},
    {'r', '\r'},
    {'t', '\t'},
    {'v', '\v'},
};
static const size_t SIMPLE_ESCAPE_SEQUENCE_SIZE = sizeof(simple_escape_sequence_map) / sizeof(simple_escape_sequence_map[0]); // number of simple escape sequences
// list of octal digits
static const char octal_digit_list[] = "01234567";
static const size_t OCTAL_DIGIT_LIST_SIZE = sizeof(octal_digit_list) / sizeof(octal_digit_list[0]); // number of octal digits
// list of hexadecimal digits
static const char hexadecimal_digit_list[] = "0123456789abcdefABCDEF";
static const size_t HEXADECIMAL_DIGIT_LIST_SIZE = sizeof(hexadecimal_digit_list) / sizeof(hexadecimal_digit_list[0]); // number of hexadecimal digits
// list of integer suffixes
// It is necessary to put longer strings above than shorter strings.
static const struct {const char *string; IntegerSuffix suffix;} integer_suffix_list[] = {
    {"ull", IS_UNSIGNED_LONGLONG},
    {"uLL", IS_UNSIGNED_LONGLONG},
    {"Ull", IS_UNSIGNED_LONGLONG},
    {"ULL", IS_UNSIGNED_LONGLONG},
    {"llu", IS_UNSIGNED_LONGLONG},
    {"llU", IS_UNSIGNED_LONGLONG},
    {"LLu", IS_UNSIGNED_LONGLONG},
    {"LLU", IS_UNSIGNED_LONGLONG},
    {"ul",  IS_UNSIGNED_LONG},
    {"uL",  IS_UNSIGNED_LONG},
    {"Ul",  IS_UNSIGNED_LONG},
    {"UL",  IS_UNSIGNED_LONG},
    {"lu",  IS_UNSIGNED_LONG},
    {"lU",  IS_UNSIGNED_LONG},
    {"Lu",  IS_UNSIGNED_LONG},
    {"LU",  IS_UNSIGNED_LONG},
    {"ll",  IS_LONGLONG},
    {"LL",  IS_LONGLONG},
    {"u" ,  IS_UNSIGNED},
    {"U" ,  IS_UNSIGNED},
    {"l",   IS_LONG},
    {"L",   IS_LONG},
};
static const size_t INTEGER_SUFFIX_LIST_SIZE = sizeof(integer_suffix_list) / sizeof(integer_suffix_list[0]); // number of integer suffixes
static char *user_input; // input of compiler
static List(Token) *token_list; // list of tokens
static ListEntry(Token) *current_token; // currently parsing token
static int source_type; // type of source
static const char *file_name; // name of source file


/*
make a new token and concatenate it to the current token
*/
static Token *new_token(TokenKind kind, char *str, int len)
{
    Token *token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->str = str;
    token->len = len;
    token->type = TY_INT;
    token->value = 0;

    return token;
}


/*
peek a reserved string
* If the next token is a given string, this function returns true.
* Otherwise, it returns false.
*/
bool peek_reserved(const char *str)
{
    Token *current = get_element(Token)(current_token);

    return (
           (current->kind == TK_RESERVED)
        && (current->len == strlen(str))
        && (strncmp(current->str, str, current->len) == 0));
}


/*
peek a token
* If the next token is a given kind of token, this function returns the token by argument and true by value.
* Otherwise, it returns false.
*/
bool peek_token(TokenKind kind, Token **token)
{
    Token *current = get_element(Token)(current_token);

    if(current->kind == kind)
    {
        *token = current;
        return true;
    }
    else
    {
        return false;
    }
}


/*
consume a reserved string
* If the next token is a given string, this function parses the token and returns true.
* Otherwise, it returns false.
*/
bool consume_reserved(const char *str)
{
    if(!peek_reserved(str))
    {
        return false;
    }

    current_token = next_entry(Token, current_token);

    return true;
}


/*
consume a token
* If the next token is a given kind of token, this function parses and returns the token by argument and true by value.
* Otherwise, it returns false.
*/
bool consume_token(TokenKind kind, Token **token)
{
    if(!peek_token(kind, token))
    {
        return false;
    }

    current_token = next_entry(Token, current_token);

    return true;
}


/*
get the currently parsing token
*/
Token *get_token(void)
{
    return get_element(Token)(current_token);
}


/*
set the currently parsing token
*/
void set_token(Token *token)
{
    for_each_entry(Token, cursor, token_list)
    {
        if(get_element(Token)(cursor) == token)
        {
            current_token = cursor;
            return;
        }
    }
}


/*
parse a reserved string
* If the next token is a given string, this function parses the token.
* Otherwise, it reports an error.
*/
void expect_reserved(const char *str)
{
    if(!consume_reserved(str))
    {
        report_error(get_element(Token)(current_token)->str, "expected '%s'.", str);
    }
}


/*
parse an identifier
* If the next token is an identifier, this function parses and returns the token.
* Otherwise, it reports an error.
*/
Token *expect_identifier(void)
{
    Token *current = get_element(Token)(current_token);
    if(current->kind != TK_IDENT)
    {
        report_error(current->str, "expected an identifier.");
    }

    current_token = next_entry(Token, current_token);

    return current;
}


/*
parse a constant
* If the next token is a constant, this function parses and returns the token.
* Otherwise, it reports an error.
*/
Token *expect_constant(void)
{
    Token *current = get_element(Token)(current_token);
    if(current->kind != TK_CONST)
    {
        report_error(current->str, "expected a constant.");
    }

    current_token = next_entry(Token, current_token);

    return current;
}


/*
tokenize a given string
*/
void tokenize(char *str)
{
    // save input
    user_input = str;

    // initialize token stream
    token_list = new_list(Token)();

    while(*str)
    {
        int len;

        // ignore space
        len = is_space(str);
        if(len > 0)
        {
            str += len;
            continue;
        }

        // ignore comment
        len = is_comment(str);
        if(len > 0)
        {
            str += len;
            continue;
        }

        // parse a reserved string
        len = is_reserved(str);
        if(len > 0)
        {
            Token *token = new_token(TK_RESERVED, str, len);
            current_token = add_list_entry_tail(Token)(token_list, token);
            str += len;
            continue;
        }

        // parse an identifer
        len = is_identifier(str);
        if(len > 0)
        {
            Token *token = new_token(TK_IDENT, str, len);
            current_token = add_list_entry_tail(Token)(token_list, token);
            str += len;
            continue;
        }

        // parse a string-literal
        len = is_string(str);
        if(len > 0)
        {
            Token *token = new_token(TK_STR, str + 1, len - 2);
            current_token = add_list_entry_tail(Token)(token_list, token);
            str += len;
            continue;
        }

        // parse a constant
        TypeKind type;
        long value;
        len = is_constant(str, &type, &value);
        if(len > 0)
        {
            Token *token = new_token(TK_CONST, str, len);
            current_token = add_list_entry_tail(Token)(token_list, token);
            token->type = type;
            token->value = value;
            str += len;
            continue;
        }

        // Other characters are not accepted as a token.
        report_error(str, "cannot tokenize.");
    }

    // reset the currently parsing token
    current_token = get_first_entry(Token)(token_list);
}


/*
check if the current token is the end of input
*/
bool at_eof(void)
{
    return end_iteration(Token)(token_list, current_token);
}


/*
make string of an identifier
*/
char *make_identifier(const Token *token)
{
    char *ident = calloc(token->len + 1, sizeof(char));

    return strncpy(ident, token->str, token->len);
}


/*
set type of source code
*/
void set_source(SourceType type)
{
    source_type = type;
}


/*
read source code from a file
*/
char *read_file(const char *path)
{
    // open source file
    FILE *stream = fopen(path, "r");
    if(stream == NULL)
    {
        fprintf(stderr, "cannot open %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // get length of source code
    if(fseek(stream, 0, SEEK_END) == -1)
    {
        fprintf(stderr, "%s: fseek: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    size_t size = ftell(stream);
    if(fseek(stream, 0, SEEK_SET) == -1)
    {
        fprintf(stderr, "%s: fseek: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // read source code and make it end with "\n\0"
    char *buffer = calloc(size + 2, sizeof(char));
    fread(buffer, size, 1, stream);
    if((size == 0) || (buffer[size - 1] != '\n'))
    {
        buffer[size] = '\n';
        buffer[size + 1] = '\0';
    }
    else
    {
        buffer[size] = '\0';
    }

    // save name of source file
    file_name = path;

    return buffer;
}


/*
report a warning
*/
void report_warning(const char *loc, const char *fmt, ...)
{
    // report the position where an error is detected
    const char *pos = (loc == NULL ? get_token()->str : loc);
    report_position(pos);

    // print the message
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
}


/*
report an error
* This function never returns.
*/
void report_error(const char *loc, const char *fmt, ...)
{
    // report the position where an error is detected
    const char *pos = (loc == NULL ? get_token()->str : loc);
    report_position(pos);

    // print the message
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);

    // stop compiling
    exit(1);
}


/*
check if the following string is space
*/
static int is_space(const char *str)
{
    int len = 0;

    while(isspace(str[len]))
    {
        len++;
    }

    return len;
}


/*
check if the following string is comment
*/
static int is_comment(const char *str)
{
    int len = 0;

    if(strncmp(str, "//", 2) == 0)
    {
        // line comment
        len = 2;
        while(str[len] != '\n')
        {
            len++;
        }
    }
    else if(strncmp(str, "/*", 2) == 0)
    {
        // block comment
        const char *start = &str[2];
        char *end = strstr(start, "*/");
        if(end == NULL)
        {
            report_error(str, "block comment is not closed with \"*/\".\n");
        }

        len = (end - start) + 4; // add 4 to count "/*" and "*/"
    }

    return len;
}

/*
check if the following string is reserved
*/
static int is_reserved(const char *str)
{
    // check punctuators
    for(size_t i = 0; i < PUNCTUATOR_LIST_SIZE; i++)
    {
        const char *punc = punctuator_list[i];
        size_t len = strlen(punc);
        if(strncmp(str, punc, len) == 0)
        {
            return len;
        }
    }

    // check keywords
    for(size_t i = 0; i < KEYWORD_LIST_SIZE; i++)
    {
        const char *kw = keyword_list[i];
        size_t len = strlen(kw);
        if((strncmp(str, kw, len) == 0) && (!isalnum(str[len]) && (str[len] != '_')))
        {
            return len;
        }
    }

    return 0;
}


/*
check if the following string is an identifier
*/
static int is_identifier(const char *str)
{
    int len = 0;

    if(isalpha(*str) || (*str == '_'))
    {
        len++;
        str++;

        // there may be a digit after second character
        while(isalnum(*str) || (*str == '_'))
        {
            len++;
            str++;
        }
    }

    return len;
}


/*
check if the following string is a string-literal
*/
static int is_string(const char *str)
{
    int len = 0;

    if(*str == '"')
    {
        len++;
        while((str[len] != '\0') && (str[len] != '"'))
        {
            if(str[len] == '\\')
            {
                len++;
                len += parse_escape_sequence(&str[len]);
            }
            else
            {
                len++;
            }
        }

        if(str[len] == '"')
        {
            len++;
        }
        else
        {
            report_error(str, "expected '\"'");
        }
    }

    return len;
}


/*
check if the following string is a constant
*/
static int is_constant(const char *str, TypeKind *kind, long *value)
{
    int len = 0;

    if(*str == '\'')
    {
        // character constant
        len++;

        if(str[len] == '\'')
        {
            report_error(str, "empty character constant");
        }

        if(str[len] == '\\')
        {
            len++;
            len += parse_escape_sequence(&str[len]);
        }
        else
        {
            len++;
        }

        if(str[len] == '\'')
        {
            len++;
        }
        else
        {
            report_error(str, "expected \"'\"");
        }

        *kind = TY_INT;
        *value = convert_character_constant(str);
    }
    else
    {
        // integer constant
        int base;
        const char *start;
        int (*check_digit)(int);

        // check prefix
        if((strncmp(&str[len], "0x", 2) == 0) || (strncmp(&str[len], "0X", 2) == 0))
        {
            len += 2;
            base = 16;
            check_digit = is_hexadeciaml_digit;
        }
        else if(str[len] == '0')
        {
            base = 8;
            check_digit = is_octal_digit;
        }
        else
        {
            base = 10;
            check_digit = isdigit;
        }

        // consume digits
        start = &str[len];
        while(check_digit(str[len]))
        {
            len++;
        }

        // check suffix
        IntegerSuffix suffix = IS_NONE;
        for(size_t i = 0; i < INTEGER_SUFFIX_LIST_SIZE; i++)
        {
            const char *string = integer_suffix_list[i].string;
            size_t suffix_length = strlen(string);
            if(strncmp(&str[len], string, suffix_length) == 0)
            {
                len += suffix_length;
                suffix = integer_suffix_list[i].suffix;
                break;
            }
        }

        *value = convert_integer_constant(start, base, suffix, kind);
    }

    return len;
}


/*
check if the character is an octal digit
*/
static int is_octal_digit(int character)
{
    for(size_t i = 0; i < OCTAL_DIGIT_LIST_SIZE; i++)
    {
        if(character == octal_digit_list[i])
        {
            return true;
        }
    }

    return false;
}


/*
check if the character is a hexadecimal digit
*/
static int is_hexadeciaml_digit(int character)
{
    for(size_t i = 0; i < HEXADECIMAL_DIGIT_LIST_SIZE; i++)
    {
        if(character == hexadecimal_digit_list[i])
        {
            return true;
        }
    }

    return false;
}


/*
parse an escape sequence
*/
static int parse_escape_sequence(const char *str)
{
    int len = 0;

    // check simple escape sequence
    for(size_t i = 0; i < SIMPLE_ESCAPE_SEQUENCE_SIZE; i++)
    {
        if(str[len] == simple_escape_sequence_map[i].character)
        {
            len++;
            break;
        }
    }
    if(len > 0)
    {
        return len;
    }

    // check octal escape sequence
    // An octal escape sequence consists of up to 3 octal digits.
    for(size_t count = 0; count < 3; count++)
    {
        if(is_octal_digit(str[len]))
        {
            len++;
        }
    }
    if(len > 0)
    {
        return len;
    }

    report_error(str, "unknown escape sequence");
    return len;
}


/*
convert an integer-constant
*/
static long convert_integer_constant(const char *start, int base, IntegerSuffix suffix, TypeKind *kind)
{
    // note that the converted string always represents positive value
    unsigned long value = strtoul(start, NULL, base);

    bool valid = (errno != ERANGE);
    if(valid)
    {
        // determine type by value, base and suffix
        switch(suffix)
        {
        case IS_NONE:
            if(value <= INT_MAX)
            {
                *kind = TY_INT;
            }
            else if((base != 10) && (value <= UINT_MAX))
            {
                *kind = TY_UINT;
            }
            else if(value <= LONG_MAX)
            {
                *kind = TY_LONG;
            }
            else if(base != 10)
            {
                *kind = TY_ULONG;
            }
            else
            {
                valid = false;
            }
            break;

        case IS_UNSIGNED:
            if(value <= UINT_MAX)
            {
                *kind = TY_UINT;
            }
            else
            {
                *kind = TY_ULONG;
            }
            break;

        case IS_LONG:
        case IS_LONGLONG:
            if(value <= LONG_MAX)
            {
                *kind = TY_LONG;
            }
            else if(base != 10)
            {
                *kind = TY_ULONG;
            }
            else
            {
                valid = false;
            }
            break;

        case IS_UNSIGNED_LONG:
        case IS_UNSIGNED_LONGLONG:
            *kind = TY_ULONG;
            break;

        default:
            *kind = TY_INT;
            break;
        }
    }

    // handle invalid case
    if(!valid)
    {
        report_warning(start, "integer constant is too large");
        *kind = TY_LONG;
    }

    // represent integer by long type
    return (long)value;
}


/*
convert a character constant
*/
static int convert_character_constant(const char *str)
{
    if(str[1] == '\\')
    {
        // simple escape sequence
        for(size_t i = 0; i < SIMPLE_ESCAPE_SEQUENCE_SIZE; i++)
        {
            if(str[2] == simple_escape_sequence_map[i].character)
            {
                return simple_escape_sequence_map[i].value;
            }
        }

        // octal escape sequence
        return strtol(&str[2], NULL, 8);
    }
    else
    {
        return str[1];
    }
}


/*
report the position at which a warning or an error is detected
*/
static void report_position(const char *loc)
{
    int pos;

    if(source_type == SRC_FILE)
    {
        // search the first and last character of the line including the given location
        int start_pos = 0;
        while((user_input < &loc[start_pos]) && (loc[start_pos-1] != '\n'))
        {
            start_pos--;
        }
        int end_pos = 0;
        while(loc[end_pos] != '\n')
        {
            end_pos++;
        }

        // search the line number including the given location
        int line_number = 1;
        for(char *p = user_input; p < &loc[start_pos]; p++)
        {
            if(*p == '\n')
            {
                line_number++;
            }
        }

        // output file name, line number and the line including the given location
        int indent = fprintf(stderr, "%s:%d: ", file_name, line_number);
        fprintf(stderr, "%.*s\n", end_pos - start_pos, &loc[start_pos]);
        pos = indent + (-start_pos);
    }
    else
    {
        // output all the input
        fprintf(stderr, "%s\n", user_input);
        pos = loc - user_input;
    }

    // emphasize the position
    fprintf(stderr, "%*s^\n", pos, "");
}
