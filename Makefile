CC=gcc
CFLAGS=-g -std=c11 -Wall

OBJ_DIRECTORY=build
SELF_DIRECTORY=self
SRC_DIRECTORY=source

OBJ=$(OBJ_DIRECTORY)/9cc
OBJ_GEN2=$(OBJ_DIRECTORY)/9cc_gen2
OBJ_GEN3=$(OBJ_DIRECTORY)/9cc_gen3
SRCS=$(wildcard $(SRC_DIRECTORY)/*.c)

BUILD_SCRIPT=util/build.sh
EXECUTE_SCRIPT=util/execute_binary.sh
SELF_BUILD_SCRIPT=util/self_build.sh

TEST_ASSEMBLY=test/test_code.s
TEST_ASSEMBLY_GEN2=test/test_code_gen2.s
TEST_ASSEMBLY_GEN3=test/test_code_gen3.s
TEST_BINARY=test/test_bin_9cc
TEST_BINARY_GEN2=test/test_bin_9cc_gen2
TEST_BINARY_GEN3=test/test_bin_9cc_gen3


$(OBJ): $(OBJ_DIRECTORY) $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $(OBJ)

$(OBJ_GEN2): $(OBJ) $(SELF_DIRECTORY)
	$(SELF_BUILD_SCRIPT) $(OBJ) $(OBJ_GEN2)

$(OBJ_GEN3): $(OBJ_GEN2) $(SELF_DIRECTORY)
	$(SELF_BUILD_SCRIPT) $(OBJ_GEN2) $(OBJ_GEN3)

$(OBJ_DIRECTORY):
	mkdir $(OBJ_DIRECTORY)

$(SELF_DIRECTORY):
	mkdir $(SELF_DIRECTORY)

clean:
	rm -fr \
		$(OBJ_DIRECTORY) \
		$(SRC_DIRECTORY)/*.o \
		test/*.s test/test_bin_* test/9cc_* \
		$(SELF_DIRECTORY)

test: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY)
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_gen2: $(OBJ_GEN2)
	$(BUILD_SCRIPT) $(OBJ_GEN2) $(TEST_ASSEMBLY_GEN2) $(TEST_BINARY_GEN2)
	$(EXECUTE_SCRIPT) $(TEST_BINARY_GEN2)

test_gen3: $(OBJ_GEN3)
	$(BUILD_SCRIPT) $(OBJ_GEN3) $(TEST_ASSEMBLY_GEN3) $(TEST_BINARY_GEN3)
	$(EXECUTE_SCRIPT) $(TEST_BINARY_GEN3)

test_self_build: test_gen2 test_gen3
	diff -q $(TEST_ASSEMBLY_GEN2) $(TEST_ASSEMBLY_GEN3)

.PHONY: clean test test_gen2 test_gen3 test_self_build
