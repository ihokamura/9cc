CFLAGS=-std=c11 -g -Wall
SRCS=$(wildcard ./source/*.c)
HDRS=$(wildcard ./source/*.h)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): $(HDRS)

test: 9cc
	bash ./util/test.sh ./9cc test_code.s test_bin_9cc

self: 9cc
	bash ./util/self.sh

clean:
	rm -f 9cc ./source/*.o ./source/*~ ./test/tmp.c ./test/*.s ./test/test_bin_* ./test/9cc_* self/*

.PHONY: test self clean