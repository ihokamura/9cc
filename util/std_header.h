// type definition
typedef unsigned long size_t;

typedef struct FILE FILE;

typedef struct __va_elem __va_elem;
struct __va_elem
{
  int gp_offset;
  int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
};
typedef __va_elem va_list[1];

// function prototype

// assert.h
static void assert() {}

// ctype.h
int isalnum(int c);
int isalpha(int c);
int isdigit(int c);
int isspace(int c);

// stdio.h
extern FILE *stderr;
extern FILE *stdout;
FILE *fopen(const char *path, const char *mode);
int fprintf(FILE *stream, const char *format, ...);
int fputc(int c, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
char *strerror(int errnum);
int vfprintf(FILE *stream, const char *format, va_list ap);

// stdlib.h
void *calloc(size_t count, size_t size);
void exit(int status);
void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
long strtol(const char *str, char **endptr, int base);
unsigned long strtoul(const char *str, char **endptr, int base);

// string.h
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
char *strncpy(char *dst, char *src, size_t len);
char *strstr(const char *haystack, const char *needle);                                              

// sys/errno.h
int *__errno_location();
