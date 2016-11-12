# Makefile for qsh

CC   ?= cc
CXX  ?= c++
LD   ?= ld

PREFIX ?= install

OPTS += -O3 -g
WARN += -Wall -Wextra -Werror
INCL += -Iinclude -Isrc/libqsh
CFLAGS += -DBUILD_QSH -std=c99 -fvisibility=hidden -fPIC $(OPTS)
CXXFLAGS += -DBUILD_QSH -std=c++11 -fvisibility=hidden -fPIC $(OPTS)
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

VER=0.0.1

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

SRC_DIRS := src/libqsh src/libqsh/parser
TEST_DIRS := tests

PARSER_DIR := src/libqsh/parser
LEX_GRAMMAR := tokenizer
PARSE_GRAMMAR := parser

GRAMMAR_FILE_NAMES := $(addsuffix .gen,$(LEX_GRAMMAR)) $(addsuffix .gen,$(PARSE_GRAMMAR))
GRAMMAR_FILES := $(addprefix $(PARSER_DIR)/,$(GRAMMAR_FILE_NAMES))
GRAMMAR_OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(GRAMMAR_FILES:.c=.o))

C_SRC_FILES := $(foreach dir,$(SRC_DIRS),$(addprefix $(OBJ_DIR)/,$(wildcard $(dir)/*.c)))
CXX_SRC_FILES := $(foreach dir,$(SRC_DIRS),$(addprefix $(OBJ_DIR)/,$(wildcard $(dir)/*.cpp)))

TEST_SRC_FILES := $(foreach dir,$(TEST_DIRS),$(wildcard $(dir)/*.cpp))
TESTS := $(TEST_SRC_FILES:.cpp=)
TEST_LOGS := $(TEST_SRC_FILES:.cpp=.log)

C_OBJ_FILES := $(C_SRC_FILES:.c=.o)
C_DEP_FILES := $(C_SRC_FILES:.c=.d)
CXX_OBJ_FILES := $(CXX_SRC_FILES:.cpp=.o)
CXX_DEP_FILES := $(CXX_SRC_FILES:.cpp=.d)

# OBJS := $(C_OBJ_FILES) $(CXX_OBJ_FILES) $(GRAMMAR_OBJ_FILES)
OBJS := $(C_OBJ_FILES) $(CXX_OBJ_FILES)

LIB_NAME:=libqsh.so
QSH_LIB := $(BUILD_DIR)/$(LIB_NAME)

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

grammar: ${GRAMMAR_FILES}

$(OBJ_DIR)/%.o: %.c
	@$(MKDIR) -p $(shell dirname $@)
	@$(CC) $(CFLAGS) $(INCL) $(WARN) -c -MM -MF $(patsubst %.o,%.d,$@) $<
	@$(PRINTF) 'Compiling    \033[1m$<\033[0m...\n'
	$(CC) $(CFLAGS) $(INCL) $(WARN) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) -p $(shell dirname $@)
	@$(CXX) $(CXXFLAGS) $(INCL) $(WARN) -c -MM -MF $(patsubst %.o,%.d,$@) $<
	@$(PRINTF) 'Compiling    \033[1m$<\033[0m...\n'
	$(CXX) $(CXXFLAGS) $(INCL) $(WARN) -c $< -o $@

# $(GRAMMAR_OBJ_FILES): WARN += -Wno-unused-function -Wno-unused-parameter
# $(GRAMMAR_OBJ_FILES): CFLAGS := $(subst c99,gnu99,$(CFLAGS))
$(OBJ_DIR)/$(PARSER_DIR)/parser.o: WARN += -Wno-unused-function -Wno-unused-parameter

%.pc: %.c
	$(CC) $(CFLAGS) $(INCL) $(WARN) -c $< -E | less

%.pcpp: %.cpp
	$(CXX) $(CXXFLAGS) $(INCL) $(WARN) -c $< -E | less

$(QSH_LIB): $(OBJS)
	@$(PRINTF) 'Linking      \033[1m$@\033[0m...\n'
	$(LD) -o $(QSH_LIB) $(LDFLAGS) $(OBJS)

$(PREFIX)/lib/$(LIB_NAME): $(QSH_LIB)
	@$(PRINTF) 'Installing @ \033[1m$@\033[0m...\n'
	$(MKDIR) -p $(PREFIX)/include $(PREFIX)/lib
	$(CP) -rf include/* $(PREFIX)/include
	$(CP) -f $(QSH_LIB) $(PREFIX)/lib

install: $(PREFIX)/lib/$(LIB_NAME)

$(TESTS): |install
	@$(PRINTF) 'Making test  \033[1m$@\033[0m...\n'
	$(CXX) $(CXXFLAGS) -I$(PREFIX)/include -Itests $(WARN) -Wno-unused-parameter $(addsuffix .cpp,$@) -o $@ -Wl,-rpath $(PREFIX)/lib -L $(PREFIX)/lib -lqsh -lstdc++

%.log: ${TESTS}
	@$(PRINTF) 'Running test \033[1m$<\033[0m...\n'
	$<
#env LD_LIBRARY_PATH=$(PREFIX)/lib $<

check: ${TEST_LOGS}

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) -f $(TESTS) $(TEST_LOGS)
	find * | grep '~$$' | xargs $(RM)
