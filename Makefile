# Makefile for qsh

ifeq ($(DEBUG),1)
	OPTS = $(DEBUG_OPTS)
else
	OPTS = $(RELEASE_OPTS)
endif

ifeq ($(OS),Windows_NT)
	LD := g++
	SL := dll
	LDFLAGS := -Wl,--gc-sections
else
	LD := ld
	SL := so
	LDFLAGS := --gc-sections
endif

CC   ?= cc
CXX  ?= c++
LD   ?= ld

PREFIX ?= install

DEBUG_OPTS := -O0 -g -fno-inline
RELEASE_OPTS := -O2

WARN += -Wall -Wextra -Werror
INCL += -Iinclude -Isrc/libqsh
CFLAGS += -DBUILD_QSH -std=c99 -fvisibility=hidden -fPIC -fdata-sections -ffunction-sections
CXXFLAGS += -DBUILD_QSH -std=c++11 -fvisibility=hidden -fPIC -fdata-sections -ffunction-sections
LDFLAGS += -shared

CAT ?= cat
MV  ?= mv
ECHO ?= echo
MKDIR ?= mkdir
PRINTF ?= printf
RM ?= rm -f
CP ?= cp

BISON ?= bison
FLEX  ?= flex
OBJCOPY ?= objcopy

VER=0.0.1

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

SRC_DIRS := $(shell find src/* -type d | xargs)
TEST_DIRS := tests

PARSER_DIR := src/libqsh/parser
LEX_GRAMMAR := tokenizer
PARSE_GRAMMAR := parser

GRAMMAR_FILE_NAMES := $(addsuffix .gen,$(LEX_GRAMMAR)) $(addsuffix .gen,$(PARSE_GRAMMAR))
GRAMMAR_FILES := $(addprefix $(PARSER_DIR)/,$(GRAMMAR_FILE_NAMES))

C_SRC_FILES := $(foreach dir,$(SRC_DIRS),$(addprefix $(OBJ_DIR)/,$(wildcard $(dir)/*.c)))
CXX_SRC_FILES := $(foreach dir,$(SRC_DIRS),$(addprefix $(OBJ_DIR)/,$(wildcard $(dir)/*.cpp)))

TEST_SRC_FILES := $(foreach dir,$(TEST_DIRS),$(wildcard $(dir)/*.cpp))
TESTS := $(TEST_SRC_FILES:.cpp=.test)
TEST_LOGS := $(TEST_SRC_FILES:.cpp=.log)

C_OBJ_FILES := $(C_SRC_FILES:.c=.o)
C_DEP_FILES := $(C_SRC_FILES:.c=.d)
CXX_OBJ_FILES := $(CXX_SRC_FILES:.cpp=.o)
CXX_DEP_FILES := $(CXX_SRC_FILES:.cpp=.d)

OBJS := $(C_OBJ_FILES) $(CXX_OBJ_FILES)
HDRS := $(shell find * | grep 'include.*\.hpp')
SRCS := $(C_SRC_FILES) $(CXX_SRC_FILES)

LIB_NAME:=libqsh.$(SL)
QSH_LIB := $(BUILD_DIR)/$(LIB_NAME)

ifneq ($(OS),Windows_NT)
	CP_DLL_TEST := $(CP) $(QSH_LIB).dbg $(PREFIX)/lib/
	DBG_LIB := $(QSH_LIB).dbg
else
	CP_DLL_TEST := $(CP) $(QSH_LIB)* .
	DBG_LIB := $(LIB_NAME).dbg
endif

.PHONY: check grammar check
.SECONDARY: $(TESTS)

all: $(QSH_LIB)

-include $(C_DEP_FILES)
-include $(CXX_DEP_FILES)

$(PARSER_DIR)/$(PARSE_GRAMMAR).gen: $(PARSER_DIR)/$(PARSE_GRAMMAR).l
	$(BISON) -d -o $(subst .gen,.c,$@) $<
	$(MV) $(subst .gen,.c,$@) $@

$(PARSER_DIR)/$(LEX_GRAMMAR).gen: $(PARSER_DIR)/$(LEX_GRAMMAR).l
	$(FLEX) --outfile=$@ $<

grammar: $(GRAMMAR_FILES) $(PARSER_DIR)/$(PARSE_GRAMMAR).l $(PARSER_DIR)/$(LEX_GRAMMAR).l

$(OBJ_DIR)/%.o: %.c
	@$(MKDIR) -p $(shell dirname $@)
	@$(CC) $(INCL) $(WARN) $(CFLAGS) $(OPTS) -c -MM -MF $(patsubst %.o,%.d,$@) $<
	@$(PRINTF) 'Compiling    \033[1m$<\033[0m...\n'
	$(CC) $(INCL) $(WARN) $(CFLAGS) $(OPTS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) -p $(shell dirname $@)
	@$(CXX) $(INCL) $(WARN) $(CXXFLAGS) $(OPTS) -c -MM -MF $(patsubst %.o,%.d,$@) $<
	@$(PRINTF) 'Compiling    \033[1m$<\033[0m...\n'
	$(CXX) $(INCL) $(WARN) $(CXXFLAGS) $(OPTS) -c $< -o $@

$(OBJ_DIR)/$(PARSER_DIR)/parser.o: $(GRAMMAR_FILES)
$(OBJ_DIR)/$(PARSER_DIR)/parser.o: WARN += -Wno-unused-function -Wno-unused-parameter

%.pc: %.c
	$(CC) $(INCL) $(WARN) $(CFLAGS) $(OPTS) -c $< -E | less

%.pcpp: %.cpp
	$(CXX) $(INCL) $(WARN) $(CXXFLAGS) $(OPTS) -c $< -E | less

$(QSH_LIB): $(OBJS)
	@$(PRINTF) 'Linking      \033[1m$@\033[0m...\n'
	$(LD) -o $(QSH_LIB) $(LDFLAGS) $(OBJS)
	@$(PRINTF) 'Stripping    \033[1m$@\033[0m...\n'
	$(OBJCOPY) $(QSH_LIB) $(DBG_LIB)
	$(OBJCOPY) --strip-debug $(QSH_LIB)
	$(OBJCOPY) --add-gnu-debuglink=$(DBG_LIB) $(QSH_LIB)

$(PREFIX)/lib/$(LIB_NAME): $(QSH_LIB) $(HDRS)
	@$(PRINTF) 'Installing @ \033[1m$@\033[0m...\n'
	$(MKDIR) -p $(PREFIX)/include $(PREFIX)/lib
	$(CP) -rf include/* $(PREFIX)/include
	$(CP) -f $(QSH_LIB) $(DBG_LIB) $(PREFIX)/lib

install: $(PREFIX)/lib/$(LIB_NAME)

$(DBG_LIB): install
	$(CP_DLL_TEST)

$(TESTS): WARN += -Wno-unused-parameter
tests/%.test: tests/%.cpp $(PREFIX)/lib/$(LIB_NAME) $(DBG_LIB)
	@$(PRINTF) 'Making test  \033[1m$@\033[0m...\n'
	$(CXX) -I$(PREFIX)/include -Itests $(WARN) $(filter-out -DBUILD_QSH,$(CXXFLAGS)) $(DEBUG_OPTS) $< -o $@ -Wl,-rpath $(PREFIX)/lib -L $(PREFIX)/lib -lqsh -lstdc++

tests/%.log: tests/%.test
	@$(PRINTF) 'Running test \033[1m$<\033[0m...\n'
	(($< 2>&1) > $@) || echo FAIL!

check: $(TEST_LOGS) $(TESTS)

run_tests:
	ls tests/*.test | sed 's/\.test/\.test \&\& printf 'OK!\n' \&\& /' | xargs -i{}  bash -c "{} true"

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) $(TESTS) $(TEST_LOGS)
	find * | grep '~$$' | xargs $(RM)

clean_grammar:
	$(RM) $(PARSER_DIR)/*.gen
