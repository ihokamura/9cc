CFLAGS=-std=c11 -g -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test/test.sh

clean:
	rm -f 9cc *.o *~ ./test/test_code.s ./test/test_bin

.PHONY: test clean