CC=gcc
CFLAGS=-g -std=c11 -Wall

OBJ_DIRECTORY=build
SELF_DIRECTORY=self
SRC_DIRECTORY=source

OBJ=$(OBJ_DIRECTORY)/9cc
SRCS=$(wildcard $(SRC_DIRECTORY)/*.c)

BUILD_SCRIPT=util/build.sh
EXECUTE_SCRIPT=util/execute_binary.sh
SELF_BUILD_SCRIPT=util/self.sh

TEST_ASSEMBLY=test/test_code.s
TEST_BINARY=test/test_bin_9cc


$(OBJ): $(OBJ_DIRECTORY) $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $(OBJ)

$(OBJ_DIRECTORY):
	mkdir $(OBJ_DIRECTORY)

$(SELF_DIRECTORY):
	mkdir $(SELF_DIRECTORY)

clean:
	rm -fr \
		$(OBJ_DIRECTORY) \
		$(SRC_DIRECTORY)/*.o \
		test/tmp.c test/*.s test/test_bin_* test/9cc_* \
		$(SELF_DIRECTORY)

test: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY)
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_self_build: $(OBJ) $(SELF_DIRECTORY)
	$(SELF_BUILD_SCRIPT)

.PHONY: clean test test_self_build
