CC:=gcc
CFLAGS:=-g -std=c11 -Wall
RM:=rm -fr

export CFLAGS_BASE:=$(CFLAGS)
export OBJECT_DIRECTORY:=build
export SELF_BUILD_DIRECTORY:=self
export SOURCE_DIRECTORY:=source
export TEST_DIRECTORY:=test
export UTILITY_DIRECTORY:=utility

BUILD_SCRIPT:=$(UTILITY_DIRECTORY)/build.sh
EXECUTE_SCRIPT:=$(UTILITY_DIRECTORY)/execute_binary.sh
SELF_BUILD_SCRIPT:=$(UTILITY_DIRECTORY)/self_build.sh

OBJ:=$(OBJECT_DIRECTORY)/9cc
SRCS:=$(wildcard $(SOURCE_DIRECTORY)/*.c)
TEST_ASSEMBLY:=$(TEST_DIRECTORY)/test_code.s
TEST_BINARY:=$(TEST_DIRECTORY)/test_bin_9cc
TEST_DEBUG_BINARY:=$(TEST_DIRECTORY)/test_bin_gcc

COMPILER_GEN1:=$(OBJ)
COMPILER_GEN2:=$(OBJECT_DIRECTORY)/9cc_gen2
COMPILER_GEN3:=$(OBJECT_DIRECTORY)/9cc_gen3

TEST_ASSEMBLY_GEN1:=$(TEST_ASSEMBLY)
TEST_ASSEMBLY_GEN2:=$(TEST_DIRECTORY)/test_code_gen2.s
TEST_ASSEMBLY_GEN3:=$(TEST_DIRECTORY)/test_code_gen3.s
TEST_BINARY_GEN1:=$(TEST_BINARY)
TEST_BINARY_GEN2:=$(TEST_DIRECTORY)/test_bin_9cc_gen2
TEST_BINARY_GEN3:=$(TEST_DIRECTORY)/test_bin_9cc_gen3

PROFILE_DIRECTORY:=profile
PORFILE_LOG:=$(PROFILE_DIRECTORY)/profile.log
PROFILE_OBJ:=$(PROFILE_DIRECTORY)/9cc
PROFILE_OUTPUT:=gmon.out

$(OBJ): $(OBJECT_DIRECTORY) $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $(OBJ)

$(COMPILER_GEN2): $(COMPILER_GEN1) $(SELF_BUILD_DIRECTORY)
	$(SELF_BUILD_SCRIPT) $(COMPILER_GEN1) $(COMPILER_GEN2)

$(COMPILER_GEN3): $(COMPILER_GEN2) $(SELF_BUILD_DIRECTORY)
	$(SELF_BUILD_SCRIPT) $(COMPILER_GEN2) $(COMPILER_GEN3)

$(PROFILE_OBJ): $(PROFILE_DIRECTORY) $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -p -o $(PROFILE_OBJ)

$(PROFILE_DIRECTORY):
	mkdir $(PROFILE_DIRECTORY)

$(OBJECT_DIRECTORY):
	mkdir $(OBJECT_DIRECTORY)

$(SELF_BUILD_DIRECTORY):
	mkdir $(SELF_BUILD_DIRECTORY)

clean:
	$(RM) \
		$(OBJECT_DIRECTORY) \
		$(PROFILE_OUTPUT) \
		$(PROFILE_DIRECTORY) \
		$(SOURCE_DIRECTORY)/*.o \
		$(TEST_DIRECTORY)/*.s $(TEST_DIRECTORY)/test_bin_* $(TEST_DIRECTORY)/9cc_* \
		$(SELF_BUILD_DIRECTORY)

debug_test:
	$(CC) $(TEST_DIRECTORY)/*/*.c $(CFLAGS) -DTEST_ALL -o $(TEST_DEBUG_BINARY)
	$(EXECUTE_SCRIPT) $(TEST_DEBUG_BINARY)

profile_test: $(PROFILE_OBJ) $(SRCS)
	$(BUILD_SCRIPT) $(PROFILE_OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_ALL
	gprof $(PROFILE_OBJ) $(PROFILE_OUTPUT) > $(PORFILE_LOG)

test: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_ALL
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_constants: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_CONSTANTS
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_cast_operators: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_CAST_OPERATORS
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_multiplicative_operators: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_MULTIPLICATIVE_OPERATORS
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_additive_operators: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_ADDITIVE_OPERATORS
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_bitwise_shift_operators: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_BITWISE_SHIFT_OPERATORS
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_relational_operators: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_RELATIONAL_OPERATORS
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_equality_operators: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_EQUALITY_OPERATORS
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_bitwise_and_operator: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_BITWISE_AND_OPERATOR
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_exclusive_or_operator: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_EXCLUSIVE_OR_OPERATOR
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_inclusive_or_operator: $(OBJ)
	$(BUILD_SCRIPT) $(OBJ) $(TEST_ASSEMBLY) $(TEST_BINARY) TEST_INCLUSIVE_OR_OPERATOR
	$(EXECUTE_SCRIPT) $(TEST_BINARY)

test_gen2: $(COMPILER_GEN2)
	$(BUILD_SCRIPT) $(COMPILER_GEN2) $(TEST_ASSEMBLY_GEN2) $(TEST_BINARY_GEN2) TEST_ALL
	$(EXECUTE_SCRIPT) $(TEST_BINARY_GEN2)

test_gen3: $(COMPILER_GEN3)
	$(BUILD_SCRIPT) $(COMPILER_GEN3) $(TEST_ASSEMBLY_GEN3) $(TEST_BINARY_GEN3) TEST_ALL
	$(EXECUTE_SCRIPT) $(TEST_BINARY_GEN3)

test_self_build: test_gen2 test_gen3
	diff -q $(TEST_ASSEMBLY_GEN2) $(TEST_ASSEMBLY_GEN3)

.PHONY: clean debug_test profile_test test test_constants test_cast_operators test_multiplicative_operators test_additive_operators test_gen2 test_gen3 test_self_build
