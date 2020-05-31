CFLAGS=-std=c11 -g -Wall
SRCS=$(wildcard ./source/*.c)
HDRS=$(wildcard ./source/*.h)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): $(HDRS)

test: 9cc
	./test/test.sh

clean:
	rm -f 9cc ./source/*.o ./source/*~ ./test/test_code.s ./test/test_bin

.PHONY: test clean